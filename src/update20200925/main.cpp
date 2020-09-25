/**********************************************************************
 * TEENSY 3.6 BAT DETECTOR 
 * Copyright (c) 2018/2019/2020, Cor Berrevoets, registax@gmail.com
 * 
 *   Hardware and PCB by Edwin Houwertjes see:  https://drive.google.com/drive/folders/1NRtWXN9gGVnbPbqapPHgUCFOQDGjEV1q
 *   
 *   Based on original code by DD4WH
 *   https://github.com/DD4WH/Teensy-Bat-Detector
 * 
 *   TeensyForum-thread 
 *   https://forum.pjrc.com/threads/38988-Bat-detector
 *
 *   made possible by the samplerate code by Frank Boesing
 *   Audio sample rate code - function setI2SFreq
 *   Copyright (c) 2016, Frank Bösing, f.boesing@gmx.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ***********************************************************************/

/* TEENSY 3.6 PINSETUP (20200721) pins marked with X are in use  

                 GND X                X Vin  - PREAMP V+  
                   0 .                X  Analog GN D    
                   1 .                X  3.3V - MEMS MIC         
                   2 .                X  23 AUDIO -LRCLK         
       TFT-PWM     3 .                X  22 AUDIO -TX             
                   4 .                X  21 TFT CS                
                   5 .                X  20 TFT DC             
       AUDIO MEMCS 6 X                X  19 AUDIO - SCL         
       AUDIO MOSI  7 X                X  18 AUDIO - SDA         
                   8 .                X  17 A3 - ADC          <- VIN_ADC monitoring
       AUDIO BCLK  9 X                X  16 A2 - ADC          <- ADC_IN function                               
       AUDIO SDCS 10 X                X  15 AUDIO -VOL                      
       AUDIO MCLK 11 X                X  14 AUDIO -SCLK                     
       AUDIO MISO 12 X                X  13 AUDIO -RX                       
                3.3V X                X  GND                 
                  24 .                .  A22 DAC1
                  25 .                .  A21 DAC0 
                  26 .                X  39  TFT MISO
        TFT SCLK  27 X                X  38  MICROPUSH_L
        TFT MOSI  28 X                X  37  MICROPUSH_R
     ENC_L-BUTTON 29 X                X  36  ENC_R-BUTTON
     ENC_L A      30 X                X  35  ENC_R A
     ENC_L B      31 X                X  34  ENC_R B
                  32 .                .  33

*/

/* SGTL5000 references */
/* VDDD pin 30 1.8V (via AP7313)
   VDDA pin 5 3.3V (via ferrite 600)
   VDDI pin 30 3.3V (before ferrite 600) */

#define batversion "DEV 20200828"
#define versionno 1001 // used in EEProm storage <1000 is pre-release
//PIO (platformio) or ARDUINO environment 
#define COMPILER PIO
#define DEVELOP
#define DEBUG //allows debug info over the serial terminal 

#define USE_SD // can be compiled wihout the need for SD (no recording/play will be available)
#define USE_EEPROM //allows to remove EEprom section 
#define USE_HIPASS_FILTER //when used sounds can be filtered before processing

#define ADAPTED_SGTL_LIB // allows setting of vars in SGTL directly using a different library

#define USE_HEX_FILENAMES
  
//#define ADVANCEDMENU // will allow setting of VAG/FFT_N/INPUTSOURCE
//#define USE_ADC_IN // use A2 (pin16) for direct microphone input

//#define USE_LOGGER // will activacte logfiles when writing to SD (NOT AVAILABLE)

#ifndef USE_ADC_IN
  //#define USE_VIN_ADC // battery monitor (NOT AVAILABLE/TESTED ) cannot be used with AUDIO ADC_IN
#endif

/***********************************************************************
 *  TEENSY 3.6 BAT DETECTOR 
 *  Copyright (c) 2018/2019/2020, Cor Berrevoets, registax@gmail.com
 *
 *  HARDWARE:
 *     TEENSY 3.6
 *     TEENSY audio board
 *     Ultrasonic microphone with seperate preamplifier connected to mic/gnd on audioboard
 *       eg. Knowles MEMS SPU0410LR5H-QB
 *           ICS - 40730 
 *     //added july2020: use ADC for microphones
 *     TFT based on ILI9341
 *     2 rotary encoders with pushbutton
 *     2 pushbuttons
 *     SDCard
 * 
 *  when using a GX16-4 connector for the microphone: pinout 1=signal, 2=GND, 3=+V, 4=GND 
 *
*   IMPORTANT: uses the SD card slot of the Teensy, NOT the SD card slot of the audio board
 *
 *  4 operational modes: 
 *                       Heterodyne.
 *                       Automatic heterodyne 
 *                       Automatic TimeExpansion (live)
 *                        Frequency divider (1/10 implemented)
 *                       Passive (no processing)
 *
 *  Sample rates up to 384k
 *   
 *  Record raw data/WAV files
 *  Play raw data and WAV-files(user selectable) from the SDcard using time_expansion 
 *
 * 
 * **********************************************************************/
 
#include <Arduino.h>
#ifndef __MKL26Z64__
	// only SIM_UIDML will probably change !!
  unsigned long chipNum[4] = { SIM_UIDH, SIM_UIDMH, SIM_UIDML, SIM_UIDL }; //read unique chipno from Teensy
#else
	unsigned long chipNum[4] = { 0L, SIM_UIDMH, SIM_UIDML, SIM_UIDL };
#endif

// *************************** CODE BLOCKS and EXTERNAL LIBRARIES **************************
#include "bat_debug.h" //DEBUG MACROS will be empty functions when DEBUG is not used
#include "bat_vars.h" //global variables
#include "bat_audio.h" //specific calls to SGTL5000 and audio-library 
#include "bat_encoder.h" //setup and routines for encoder and micropushbuttons
#include "bat_timers.h" //time and timers 
#include "bat_tft.h" //screen routines
#include "bat_fft.h" // FFT routines and variables
#include "bat_menu.h" //menu structures


#ifdef USE_EEPROM
   #include "bat_EEPROM.h" //routines to read/write EEprom
#endif 

//allows usage of ADC as the primary source 
#ifdef USE_ADC_IN  //audio in over A2 (pin 16) 0-1.2v max (see https://forum.pjrc.com/threads/31700-Audio-Library-recommended-circuit-for-adc 
  #include "audio_mods.h" // setup of high-speed ADC sampling 
  #define AUDIO_INPUT_ADC     2
  #define F_SAMP 384000 
  #define ADC_PIN A2
#endif

#include <SPI.h>

#ifdef USE_SD
  #include "bat_sd.h" //routines to read/write SD
#endif

// ******************** BUFFER FOR TIMEEXPANSION ROUTINE ********************
#define GRANULAR_MEMORY_SIZE 16*1024  // enough for 50 ms at 281kHz sampling
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

//optional monitoring of VIN_ADC at startup, can only be used if ADC_IN is not used for AUDIO

#ifdef USE_VIN_ADC
  int VIN=0; //VIN from ADC
  #include <ADC.h>
  ADC *adc = new ADC();
#endif

/**************************************      FUNCTIONS      ******************************************/

/**************************************  MAIN SCREEN UPDATE ******************************************/

void update_screen(uint from, uint update_type) {
  //update_type 
  /*0=all
    1=change from encoder RIGHT rotation
    2=change from encoder LEFT rotation
  */
  D_PRINT("UPDATESCREEN")
  D_PRINTXY(from, update_type);

  bool update_header=true; //top header with default information on volume gain etc.
  bool update_frequencygraph=true; //the small graph below the header
  bool update_encoder_line=true; //the encoders
  bool update_pushbutton_line=true; //the pushbuttons
  
  if ((display_mode==settings_page) or (recorderActive) or (AUTO_REC) ) //we are in settings, no need to update headers etc
  { update_encoder_line=false; // no default update of the encoder/pushbuttons
    update_pushbutton_line=false;
    update_header=false;
    update_frequencygraph=false;
   }

  if ((update_type==1) or (update_type==2)) //change coming from RIGHT or LEFT encoder_rotation
   {
    update_pushbutton_line=false; //dont update pushbuttons
    if ((EncRight_function==enc_menu) and (update_type==1)) //update from RIGHT encoder but its in menu_mode so no header 
      {
        update_header=false;
      }
    if ((EncLeft_function==enc_menu) and (update_type==2)) //update from LEFT encoder but its in menu_mode so no header 
      {
        update_header=false;
      }
   }


  //start screenupdates
  //select default color and font
  tft.setTextColor(ENC_MENU_COLOR);
  tft.setFont(MENU_FONT);
  
  if (update_header) 
   { //***************** HEADER  ********************/
     showHeader();
   }

  if (not recorderActive)
       {updateTime(); //will only update every 10 seconds 
       }

  if (update_frequencygraph)
    { // ******************* GRAPHS 
      //clear the grapharea above the live graphs
      // show a scale with ticks for every 10kHz except for no_graph or settings_page
      if (((display_mode==spectrumgraph) or (display_mode==waterfallgraph)) and (AUTO_REC==false))
        { drawScale();
          drawScaleMarker(); //position the dot on the scale
        }
      showNOSD();
            
      
    }

   // *********************** SETTINGS PAGE ****************************
   if (display_mode==settings_page) // display user settings
         { showSettings();
           showSettingsButtons();
          }
    
     //BOTTOM PART OF SCREEN      
    /****************** SHOW ENCODER/BUTTON SETTING ***********************/
   

   if (update_encoder_line)  
     {  showEncoders();
       }
   if (update_pushbutton_line)
       {  showPushButtons();
        }
    
}

// *************************************************** FUNCTIONS **************************************
void set_mic_gain(int8_t gain) {
    set_mic(gain);
    powerspectrum_Max=0; // reset the powerspectrum_Max for the FFTpowerspectrum
} // end function set_mic_gain


//  ********************************************* MAIN MODE CHANGE ROUTINE *************************
void changeDetector_mode(int new_mode)
{
  detector_mode=new_mode;
  //always switch encoders to default positions (VOL/GAIN)
  granular1.stop(); //stop detecting routines
  //restore default positions if we are not in REC or PLAY
  if ((LeftButton_Mode!=MODE_REC) and (LeftButton_Mode!=MODE_PLAY))
      {defaultMenuPosition();
      }

  
  D_PRINTXY("CHANGE DETECTOR:",DT[detector_mode])
  
  if (detector_mode==detector_heterodyne) //switchting to heterodyne
         { osc_frequency=last_osc_frequency; //set the frequency to the last stored frequency
           set_freq_Oscillator (osc_frequency); //set SINE
           set_OutputMixer(heterodynemixer); //connect output to heterodyne
          
         }

  if (detector_mode==detector_Auto_heterodyne)
         { set_OutputMixer(heterodynemixer);//connect output to heterodyne
           sine1.amplitude(1);//sine ON
         }

  if (detector_mode==detector_Auto_TE)
         { granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE); //setup the memory for timeexpansion
           set_OutputMixer(granularmixer); //connect output to granular
           granular1.setSpeed(1.0/TE_speed); //default TE is 1/0.06 ~ 1/16 :TODO, switch from 1/x floats to divider value x
           sine1.amplitude(0); //shutdown SINE 
           sine1.frequency(0);
         }

  if (detector_mode==detector_divider)
         { granular1.beginDivider(GRANULAR_MEMORY_SIZE);
           set_OutputMixer(granularmixer);
           granular1.setdivider(FD_divider); //V112 changes to effect_granular have been made !!!
           sine1.amplitude(0); //shutdown SINE
           sine1.frequency(0);
           
         }

  if (detector_mode==detector_passive)
         { set_OutputMixer(passivemixer);
         }
}

/******************************************  SD FUNCTIONS ******************************/

#ifdef USE_SD

// ****************************************************  RECORDING
void startRecording() {

  D_PRINT_FORMATLN(D_BOLDGREEN,"START RECORDER")
  startREC(); //sets file ready creates new filename
  set_SR(rec_SR);   //switch to recording samplerate
  //setup the recording screen
  
  granular1.stop(); //stop granular
  outputMixer.gain(1,0);  //shutdown granular output
  if (!AUTO_REC) //during AUTODETECT several startRecording calls 
   {last_detector_mode=detector_mode; // save last used detectormode
    last_osc_frequency=osc_frequency;
    last_display_mode=display_mode;
   }

  changeDetector_mode(record_detector);// can only listen to A-HT or heterodyne when recording 

  nj=0; //counter for recording blocks
  startRecordScreen(); //show 
  recorder.begin(); //start the recording to SD

}

// ******************************************************** STOP RECORDING
void stopRecording() {
  D_PRINT_FORMATLN(D_BOLDGREEN,"STOPRECORDING FUNCTION")
  stopREC(&recorder);
  
  set_SR(oper_SR); //switch back to operational samplerate
  tft.fillScreen(COLOR_BLACK); //clear the screen
  if (not AUTO_REC) 
   { D_PRINT_FORMATLN(D_BOLDGREEN,"RESTORE MODES AFTER REC");
    osc_frequency=last_osc_frequency;
    changeDetector_mode(last_detector_mode);
    display_mode=last_display_mode;
    update_screen(2,0);
   }
            

}
// ******************************************  END RECORDING *************************



// **************** ******************************PLAYING ************************************************
void startPlaying(int SRate) {
//      String NAME = "Bat_"+String(file_number)+".raw";
//      char fi[15];
//      NAME.toCharArray(fi, sizeof(NAME));
  D_PRINTLN("START PLAYING");
  playActive=true;
  //direct play is used to test functionalty based on previous recorded data
  //this will play a previous recorded raw file through the system as if it were live data coming from the microphone
     
//allow settling
  delay(100);
  //last_SR=SR; //store current samplerate 
  if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY)
   {
    D_PRINT("SR ");
    D_PRINTLN(SRate); 
    if (SRate!=SR_192K)
         {SRate=cyclic_constrain(play_SR,0,SR_8K,MAX_play_SR);
         
          }
    
    D_PRINT(SR[SRate].txt);   
    set_SR(SRate);
   }
  continousPlay=false; 
  if (SRate<MAX_play_SR)                      
              {set_OutputMixer(passivemixer); //no processing                       
              } 
          else
          { 
             changeDetector_mode(detector_mode);
             continousPlay=true;
          }
              

  fileselect=cyclic_constrain(fileselect,0,0,filecounter-1);
  strncpy(filename, filelist[fileselect],  13);

 D_PRINTXY(" ",filename)
 player.play(filename);

}

// ********************************************************** STOP PLAYING ***************************************
void stopPlaying() {
  
  player.stop();
  D_PRINTLN("STOP PLAYING");
  playActive=false;
  update_screen(3,0); 
  // //restore last SR setting
  if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY)
  {
  set_SR(oper_SR);
  osc_frequency=last_osc_frequency;
  //restore heterodyne frequency
  set_freq_Oscillator (osc_frequency);
  }
}

// **************** CONTINUE PLAYING
void continuePlaying() {
  //the end of file was reached
  if (playActive)
   {if (!player.isPlaying()) //not playing
     {
      stopPlaying(); //finished so stop
       if (continousPlay) //keep playing until stopped by the user
          { startPlaying(play_SR); //use the samplerate set by the user
            }
     }
   }
}
#endif //USE SD

// **************** General graph and detector selective functions *******************************************************
void update_Graphs(void) 
{
// code for 256 point FFT
//TODO MOVE TO BAT_FFT for the calcs
 if (myFFT.available()) {
  const uint16_t Y_OFFSET = TOP_OFFSET;
  static int count = TOP_OFFSET;
         updateFFTanalysis(); //get FFT data and check for ultrasound 
         updateTime();
         
/************************** TE MAIN DETECTION **************************/

/*************************  SIGNAL DETECTION ***********************/
    //signal detected in the Ultrasound range
    if (sample_UltraSound) //ultrasound in the sample !
      {
        // if the previous sample was not Ultrasound so we detected a new signal
        if (!lastSample_UltraSound)
          { 
            time_since_StartDetection=0; //start timing of the call length
            //time_since_EndDetection=0; //end of detection should be zeroed ? V114 ?? 
            //restart the TimeExpansion only if replay of a previous call has ended
            if ((detector_mode==detector_Auto_TE) and (TE_ReplayEnded) )
             { granular1.stop();
               granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE);
               granular1.setSpeed(1.0/TE_speed);
               TE_ReplayEnded=false; 
             }
          } /* previously NO ultrasound detected */
       // lastSample_UltraSound=sample_UltraSound; 
       if ((detector_mode==detector_Auto_heterodyne))  //keep Auto_HT on track 
              { 
                 osc_frequency=int((FFT_peakF_bin*(SR_FFTratio)/500))*500-1000; //round to nearest 500hz and shift 1000hz up to make the signal audible
                 osc_frequency=constrain(osc_frequency,7000,int(SR_real/2000)*1000-1000);
                 set_freq_Oscillator(osc_frequency);
                 last_osc_frequency=osc_frequency;
                
                 if (not (recorderActive) & (display_mode!=no_graph))
                   {if(FFT_count%100==0)
                   { drawScale();
                     drawScaleMarker();
                   }
                 }
                
                }
     }
   else // NO ultrasound detected in current sample !
        { //previous sample was Ultrasound
          if (lastSample_UltraSound) 
           { callLength=time_since_StartDetection; // store the time since the start of the call  NOT USED !!
             time_since_EndDetection=0; //start timing endof call
             }
         // lastSample_UltraSound=sample_UltraSound; // set the end of the call
        }

   lastSample_UltraSound=sample_UltraSound;

    // restart TimeExpansion recording after at least TE_GAP milliseconds of time has passed since initial detection. This is to shortcut long calls
    // for instance a batcall click is often a few ms long. The replay takes call*TE_spd times longer so the longest call we want to hear fully
    // should be considered the best setting ? Eg. pipistrelle with 5-6ms of call using a 20x slowdown would require a TE_GAP of 100-120 to hear the full 
    // call.

    // TODO: link TE_GAP to TE_spd !! If speed goes down TE_GAP can also go down, allowing a better response
    // or cut the call to a maximum length multiplied by the te_spd ? Also if batclicks arrive in a sequence the TE_GAP should be smaller than the pause 
    // between the clicks. 
    // TODO: consider using time_since_EndDetection as a cutoff. time_since_EndDetection signals the time since the detector has not collected any Ultrasound.
    // this would allow a cutoff of the output X ms after the end of the signal instead of since the start of detection

    //hard cut the incoming signal after TE_GAP milliseconds of time since the end of the last detection 
    // we are playing a signal

  //if TE_REPLAY is still active decide if the replay should stop 

  //if (LR_DELAY==0) //old stop mode, stop recording when the time between now and the start of the signal is longer dan TE_GAP
   {if ((!TE_ReplayEnded) and (time_since_StartDetection>TE_GAP))
      { TE_ReplayEnded=true;
        lastSample_UltraSound=false; // even if we have captured UltraSound in the current(and thus last)sample mark it als false to allow trigger of TE
        granular1.stopTimeExpansion();
        time_since_EndDetection=0; //we ended the detection
      }
   }

  // if (LR_DELAY==1) //new stop mode,  stop replay if the time between now and the end of the signal is longer than TE_GAP
  //  {if ((!TE_ReplayEnded) and (time_since_EndDetection>TE_GAP))
  //     { TE_ReplayEnded=true;
  //       lastSample_UltraSound=false; // even if we have captured UltraSound in the current(and thus last)sample mark it als false to allow trigger of TE
  //       granular1.stopTimeExpansion();
  //       time_since_EndDetection=0; //we ended detection
  //     }
  //  }
  switch (display_mode)
             {
             case spectrumgraph:
              { if (fftSpectrumAvailable)
                    { spectrum();
                      fftSpectrumAvailable=false;                                      
                    }      
                if ((powerspectrumCounter>100)  )
                      { D_PRINTLN("PS")
                        showPowerSpectrum();
                        drawScale();
                        drawScaleMarker(); //position the dot on the scale
                        }
                  
                break;   
              }   

              // update power-spectrumdisplay after every 50th FFT sample with bat-activity
              case waterfallgraph:
                if ((powerspectrumCounter>100)  )
                      { showPowerSpectrum();
                        drawScale();
                        drawScaleMarker(); //position the dot on the scale
                        }
                break;        
             }
   if (display_mode==waterfallgraph)
   { //TODO: MAKE THIS MORE DEPENDENT. with higher FFTrate 25ms will flood the screen ! 
     if (fftSpectrumAvailable) 
      { //if (TE_ReplayEnded) //not playing TE
        { //if (time_since_EndDetection<20) //keep scrolling the waterfall until after the last UltraSound
          {
          //  tft.writeRect( 0,count, ILI9341_TFTWIDTH,1, (uint16_t*) &FFT_pixels_old); //show a line with spectrumdata
          //  tft.setScroll(count);
          //  count++;
           tft.writeRect( 0,count, ILI9341_TFTWIDTH,1, (uint16_t*) &FFT_pixels); //show a line with spectrumdata
           tft.setScroll(count);
           count++;
           fftSpectrumAvailable=false;
           }
        
       }
      }
    //  else
    //   { if (time_since_EndDetection<1) 
    //      { memset(FFT_pixels,0,sizeof(FFT_pixels));
    //        tft.writeRect( 0,count, ILI9341_TFTWIDTH,1, (uint16_t*) &FFT_pixels); //show a line with spectrumdata
    //        tft.setScroll(count);
    //        count++;
    //        fftSpectrumAvailable=false;
    //      }

    //   }  
      
      
     if (count >= ILI9341_TFTHEIGHT-BOTTOM_OFFSET) 
       {count = Y_OFFSET;
       }
   }

   //automatic recording of signals if we are NOT recording allready
   if (recorderActive==false)
        //if autorecording is on and a signal was found and the last recording ended more than AREC_P seconds ago
        if ((AUTO_REC) and (sample_UltraSound) and (recording_stopped>(AREC_P*AREC_BLOCK)) )
          {
          #ifdef USE_SD  
              recorderActive=true; 
              autocounter++;
              startRecording();
              recording_running=0; 
              time_since_EndDetection=0;
              display_mode=no_graph;
          #endif 
          }

  }

}


//*****************************************************update encoders
void updateEncoder(uint8_t Encoderside )
 {
   D_PRINT("Update Enc ") D_PRINTLN(Encoderside)

  /************************setup vars*************************/
   int encodermode=-1; // menu=0 value =1;
   int change=0;
   int menu_idx=0;
   int choices=0;

    //get encoderdata depending on which encoder sent data
   if (Encoderside==enc_leftside)
    { encodermode=EncLeft_function;
      change=EncLeftchange;
      menu_idx=EncLeft_menu_idx;
      menu_id=LeftBaseMenu[menu_idx].menu_id;
      choices=LeftMenuOptions; //available menu options
    }

   if (Encoderside==enc_rightside)
    { encodermode=EncRight_function;
      change=EncRightchange;
      menu_idx=EncRight_menu_idx;
      if (LeftBaseMenu[EncLeft_menu_idx].menu_id!=MENU_SETTINGS)
            { menu_id=RightBaseMenu[menu_idx].menu_id;
              }
      choices=RightMenuOptions; //available menu options
    }


  /************************react to changes from the encoder*************************/

  //encoder is in menumode
  if (encodermode==enc_menu)
    { menu_idx=menu_idx+change;

     //allow revolving choices based on the encoder (left or right !!)
      menu_idx=cyclic_constrain(menu_idx,0,0,choices-1);
      
      if (Encoderside==enc_leftside)
          { EncLeft_menu_idx=menu_idx; //limit the menu
            menu_id=LeftBaseMenu[menu_idx].menu_id;
               }

     //limit the changes of the rightside encoder for specific functions
      if (Encoderside==enc_rightside)
          { EncRight_menu_idx=menu_idx; //limit the menu
            menu_id=RightBaseMenu[menu_idx].menu_id;
               }
    }

  //encoder is in valuemode and has changed position so change an active setting
  if ((encodermode==enc_value) and (change!=0))
    { //changes have to be based on the choosen menu position
      /******************************VOLUME  ***************/
      if (menu_id==MENU_VOL)
        { volume+=change;
          volume=constrain(volume,0,90);
          set_vol(volume);
        }

      /******************************MAIN SR   ***************/
      if ((menu_id==MENU_SR) and (LeftButton_Mode!=MODE_PLAY))  //selects a possible SR but only if we are not in the playing mode
        { oper_SR+=change;
          oper_SR=constrain(oper_SR,SR_MIN,SR_MAX); //not cyclic
          set_SR(oper_SR);
        }
     /******************************MIC_GAIN  ***************/
      if (menu_id==MENU_GAIN_MIC)
        {
         mic_gain+=change;
         mic_gain=constrain(mic_gain,0,63);
         set_mic_gain(mic_gain);
     
        }
      /******************************FREQUENCY  ***************/
      if (menu_id==MENU_FRQ)
         { int delta=500;
          if (detector_mode==detector_heterodyne) //only allow manual change in heterodyne mode
           {uint32_t currentmillis=millis();
              //when turning the encoder faster make the changes larger
              if ((currentmillis-lastmillis)<500)
                  { delta=1000;}
              if ((currentmillis-lastmillis)<250)
                  { delta=2000;}
              if ((currentmillis-lastmillis)<100)
                  { delta=5000;}

              osc_frequency=osc_frequency+delta*change;
              // limit the frequency to 500hz steps
              osc_frequency=constrain(osc_frequency,7000,int(SR_real/2000)*1000-1000);
              last_osc_frequency=osc_frequency; //always backup the F setting
              set_freq_Oscillator (osc_frequency);
              lastmillis=millis();
           }
         }

      #ifdef USE_HIPASS_FILTER
          if (menu_id==MENU_HIP)
          {
                HI_pass+=change*2;
                HI_pass=constrain(HI_pass,HI_PASS_OFF,50); 
                //set_SR(oper_SR);
                setHiPass();
          

          }
      #endif  
            
      /******************************SETTINGS MENU  LEFTSIDE ENCODER INDEX ***************************/

        // for the settings menu the left encoder selects the option and the right encoder allows changes the values
        if ((menu_id==MENU_SETTINGS) and (Encoderside==enc_leftside))
          { uint idx=settings_page_nr;
            set_menu_pos[idx]+=change;
            set_menu_pos[idx]=cyclic_constrain(set_menu_pos[idx],0,0,settings_MenuOptions[idx]-1);

            if (idx==0)
              {set_menu_id[idx]=Settings0Menu[set_menu_pos[idx]].menu_id;
              }
            if (idx==1)
              {set_menu_id[idx]=Settings1Menu[set_menu_pos[idx]].menu_id;
              }
            if (idx==2)
              {set_menu_id[idx]=Settings2Menu[set_menu_pos[idx]].menu_id;
              }    
          }
     
      /******************************SETTINGS MENU RIGHTSIDE ENCODER VALUES ***************************/
      
        // change the SETTINGSPAGE (needs to be different for each settingsMenu)
        if ((menu_id==MENU_SETTINGS) and (Encoderside==enc_rightside))
        { 
           //code specific to prevent a cascade of codechanges!
          if ((set_menu_id[0]==SET_MENU_PAGE) and (settings_page_nr==0))
             { show_next_settings_page(change);
              } 
          else   
          if ((set_menu_id[1]==SET_MENU_PAGE) and (settings_page_nr==1)) 
            { show_next_settings_page(change);
              }    
          else
          if ((set_menu_id[2]==SET_MENU_PAGE) and (settings_page_nr==2)) 
            { show_next_settings_page(change);
              }  

        //the rightside encoder is used to change the current value of a given settings menu-entry
        //V115 previously settings menu defines were grouped per page now they are unique so no need to test from which page the request comes
          //frequency_divider
          if (set_menu_id[settings_page_nr]==SET_MENU_FD_DIV)
            { //TE_low+=change;
              FD_divider=cyclic_constrain(FD_divider,change,4,16); //4,8,16,32,64
              granular1.setdivider(FD_divider);
             }
          //time expansion lowest frequency
          if (set_menu_id[settings_page_nr]==SET_MENU_TE_GAP)
            { //TE_low+=change;
              TE_GAP=cyclic_constrain(TE_GAP,change*5,15,200);
             }
              
          //time expansion lowest frequency
          if (set_menu_id[settings_page_nr]==SET_MENU_TE_LOW)
            { //TE_low+=change;
              TE_low=cyclic_constrain(TE_low,change,15,35);
              signal_LoF_bin= int((TE_low*1000.0)/(SR_FFTratio));
              }
          //time expansion replay speed   
          if (set_menu_id[settings_page_nr]==SET_MENU_TE_SPD)
            {  //TE_speed+=change;
               TE_speed=cyclic_constrain(TE_speed,change, 5,30);
             }
          //operational sample rate   
          if (set_menu_id[settings_page_nr]==SET_MENU_SR)
            { oper_SR+=change;
              oper_SR=constrain(oper_SR,SR_MIN,SR_MAX);
              set_SR(oper_SR);
             }   
           if (set_menu_id[settings_page_nr]==SET_MENU_GAIN)
          {     def_gain+=change;
                def_gain=constrain(def_gain,0,63); 
                //set_SR(oper_SR);
                mic_gain=def_gain;
                set_mic_gain(mic_gain);
            }   
            if (set_menu_id[settings_page_nr]==SET_MENU_VOL)
          {     def_vol+=change;
                def_vol=constrain(def_vol,0,90); 
                //set_SR(oper_SR);
                volume=def_vol;
                set_vol(volume);
            }    

       #ifdef USE_HIPASS_FILTER
          if (set_menu_id[settings_page_nr]==SET_MENU_HIPASS)
          {     HI_pass+=change*2;
                HI_pass=constrain(HI_pass,HI_PASS_OFF,50); 
                //set_SR(oper_SR);
                setHiPass();
            }
       #endif

          //default display mode   
          if (set_menu_id[settings_page_nr]==SET_MENU_DISPLAY)
            { //startup_display+=change;
              startup_display=cyclic_constrain(startup_display,change, 0,3); //allowed settings
             }   
          //default detector mode
          if (set_menu_id[settings_page_nr]==SET_MENU_DETECTOR)
            { //startup_detector+=change;
              startup_detector=cyclic_constrain(startup_detector,change,0,4); //allowed settings
             }      
          if (set_menu_id[settings_page_nr]==SET_MENU_REC_DETECT)
            { //startup_detector+=change;
              record_detector=cyclic_constrain(record_detector,change,0,detector_Auto_heterodyne); //allowed settings
             }    
          //startup settings 
          if (set_menu_id[settings_page_nr]==SET_MENU_STARTUPMODE) //0= default, 1=user settings from EEprom
            { //use_presets+=change;
              use_presets=cyclic_constrain(use_presets,change, 0,1); //allowed settings
             }      
          //default sampleRate for replay
          if (set_menu_id[settings_page_nr]==SET_MENU_SR_PLY)
            { //play_SR+=change;
              play_SR=cyclic_constrain(play_SR,change, SR_8K,MAX_play_SR);
              //if (play_SR==MAX_play_SR)
              //   {play_SR=SR_192K;}
            }
          //default sampleRate for recording  
          if (set_menu_id[settings_page_nr]==SET_MENU_SR_REC)
            { //rec_SR+=change;
              rec_SR=cyclic_constrain(rec_SR,change, SR_44K,SR_MAX);
            }

          //default FORMAT for recording  
          if (set_menu_id[settings_page_nr]==SET_MENU_FFORMAT)
            { //rec_SR+=change;
              FFORMAT=cyclic_constrain(FFORMAT,change, FFORMAT_RAW,FFORMAT_WAV);
            }
  
          //set time
          if (set_menu_id[settings_page_nr]==SET_MENU_TIME) //we are on the time menusection
            {
             updateTimeMenu(change);
                           
            }
             //set date
            if (set_menu_id[settings_page_nr]==SET_MENU_DATE)
            { updateDateMenu(change);
              
            }

            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_DURATION)
              {  
                 AREC_D=cyclic_constrain(AREC_D,change,1,60); //5-300sec
              }

            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_PAUSE)
              {  
                AREC_P=cyclic_constrain(AREC_P,change,1,60); //5-300sec
              }
            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_BREAK)
              {  
                AREC_B=cyclic_constrain(AREC_B,change,1,12); //5-60sec
              }
              
            //default sampleRate for recording  
            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_SR_REC)
            {
              rec_SR=cyclic_constrain(rec_SR,change,SR_44K,SR_MAX);
            }
            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_S)
              {  
                 AREC_S=cyclic_constrain(AREC_S,change, 1,20);
              }
            if (set_menu_id[settings_page_nr]==SET_MENU_AREC_F)
              {  
                 AREC_F=cyclic_constrain(AREC_F,change*5,10,80);
              } 
      

           /* COLOR MENU */
           if ((set_menu_id[settings_page_nr]>=SET_MENU_COLORS) and (set_menu_id[settings_page_nr]<=SET_MENU_BCKCOLORS))
           {
                cwheelpos[set_menu_id[settings_page_nr]-SET_MENU_COLORS]+=change*4;
                uint cwheel=cyclic_constrain(cwheelpos[set_menu_id[settings_page_nr]-SET_MENU_COLORS],0,0,cwheelmax);
                cwheelpos[set_menu_id[settings_page_nr]-SET_MENU_COLORS]=cwheel;
                if (set_menu_id[settings_page_nr]==SET_MENU_COLORS)
                  { ENC_MENU_COLOR=rainbow(cwheel,2);
                    if (cwheel==0)
                      {ENC_MENU_COLOR=COLOR_YELLOW;//defaults
                       }
                  }     
                if (set_menu_id[settings_page_nr]==SET_MENU_HICOLORS)
                  {HILIGHT_MENU_COLOR=rainbow(cwheel,2);
                   if (cwheel==0)
                      {HILIGHT_MENU_COLOR=COLOR_WHITE; //defaults
                       }
                   }
                if (set_menu_id[settings_page_nr]==SET_MENU_BCKCOLORS)
                  {BCK_MENU_COLOR=rainbow(cwheel,1);
                   if (cwheel==0)
                      {BCK_MENU_COLOR=COLOR_DARKRED; //defaults
                       }
                   }
             }

          //encoder direction
          if (set_menu_id[settings_page_nr]==SET_MENU_ENC)
            { ENCODER_TURN=ENCODER_TURN*-1; //reverse
              enc_dn=ENCODER_TURN*-1;
              enc_up=ENCODER_TURN;
             } 

          #ifdef USE_ADC_IN   
          if (set_menu_id[settings_page_nr]==SET_MENU_ADC)
            { ADC_ON=!ADC_ON; //toggle 
              if (ADC_ON)
                { set_InputMixer(in_adc);
                  set_mic_gain(mic_gain);
                }
                else
                {
                  set_InputMixer(in_mic);
                  set_mic_gain(mic_gain);
                }
             }  
         #endif 

#ifdef ADVANCEDMENU
            if (set_menu_id[settings_page_nr]==SET_MENU_INPUT)
              {    if (myInput==AUDIO_INPUT_LINEIN) 
                     { myInput=AUDIO_INPUT_MIC;
                       
                      }
                   else
                   {
                     myInput=AUDIO_INPUT_LINEIN;
                                      
                   }
                  
                 AudioNoInterrupts();
                   sgtl5000.inputSelect(myInput);
                   set_mic_gain(mic_gain);
                 AudioInterrupts();

              }
              //LR_DELAY
            if (set_menu_id[settings_page_nr]==SET_MENU_LR_DELAY)
            { //TE_low+=change;
              LR_DELAY=cyclic_constrain(LR_DELAY,change*2,0,20);
              delay1.delay(0,LR_DELAY);
              //granular1.setLR_DELAY(LR_DELAY);
             }   
            if (set_menu_id[settings_page_nr]==SET_MENU_FFT_N)
              {  FFT_N+=change;
                 FFT_N=constrain(FFT_N,2,16);
                 myFFT.averageTogether(FFT_N);
              }
             

            #ifdef ADAPTED_SGTL_LIB 
            if (set_menu_id[settings_page_nr]==SET_MENU_VAG)
               { VAG+=change;
                 VAG=constrain(VAG,uint(0),uint(0x1F));
                 sgtl5000_ref_ctrl=VAG*0x10 | BIAS;  //BIAS is default 0x0002 VAG is default 0x1F, combined 0x01F2
                 AudioNoInterrupts();
                 sgtl5000.SET_CHIP_MIC_REF(sgtl5000_ref_ctrl);
                 AudioInterrupts();

                }
           #endif     
#endif           
           

    }
    //******************************END SETTINGS MENU  RIGHTSIDE ENCODER  ***************************/

      /******************************SELECT A FILE  ***************/
      if (((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY)) and (EncLeft_function==enc_value))//menu selected file to be played
         {  
           #ifdef USE_SD
             fileselect=cyclic_constrain(fileselect,EncLeftchange,0,filecounter-1);
           
           #endif
         }

      /******************************CHANGE PLAY SR   ***************/
      if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY) and (RightBaseMenu[EncRight_menu_idx].menu_id==MENU_SR) and (EncRight_function==enc_value))//menu play selected on the left and right
          {if ((LeftButton_Mode==MODE_PLAY))
              { if (EncRightchange!=0)
                   {play_SR=cyclic_constrain(play_SR,EncRightchange,SR_8K,MAX_play_SR);
                    if (play_SR==MAX_play_SR)
                      {play_SR=SR_192K;}
                    if (playActive)
                          {stopPlaying();
                          }
                     set_SR(play_SR);
                   } 
              }
        }

      

    }
 }
// **************************  ENCODERS
void update_Encoders()
{
  getEncoderchanges();
  if ((EncLeftchange!=0) )
    {updateEncoder(enc_leftside);
      }
  if ((EncRightchange!=0) )
    {updateEncoder(enc_rightside);
        }
 
 //update display only if a change has happened to at least one encoder
 if ((EncRightchange!=0) or (EncLeftchange!=0))
      {
         if (EncRightchange!=0)
            {update_screen(4,1);
            }
            else
            {update_screen(4,2);}
      }

}
// ******************************************************************************  BUTTONS
void update_Buttons()
{// Respond to button presses
 // try to make the interrupts as short as possible when recording
 
 if (AUTO_REC) //for autorec functions
   {  micropushButton_L.update(); //ONLY check the left pushbutton
      if (micropushButton_L.risingEdge()) //************* left pushbutton was pressed !!!! 
       { D_PRINT_FORMATLN(D_BOLDGREEN,"MANUAL STOP AUTOREC FUNCTION")
         #ifdef USE_SD
          AUTO_REC=false; //stop auto_recording 
          if (recorderActive) //manual stop during a recording 
              { D_PRINTLN(" STOP ACTIVE AUTO_REC")
                stopRecording();
               }
                //stop recording and bring back the previous detector_mode and display_mode
          else  //detector is in autorec mode but was not recording so in between two recordings stopped
              { D_PRINTLN(" STOP PAUZE AUTO_REC")
                stopRecording();
              }

          delay(100); //wait a bit
          D_PRINTLN(" LBUTTON-> DISPLAY1")
          LeftButton_Mode = MODE_DISPLAY; //switch back to display
          display_mode=last_display_mode;
          recorderActive=false;
          update_screen(5,0);
          
         #endif
       }
   }
 //recorder is active but not in AUTOREC mode
 if (!AUTO_REC)
 {
 if (recorderActive) // RECORDING MODE so do minimal checks !! if AUTOREC was on the check has allready been done
   {  micropushButton_L.update(); //ONLY check the left encoderbutton 
      if ((micropushButton_L.risingEdge())  )
       { D_PRINT_FORMATLN(D_BOLDGREEN,"MANUAL STOP RECORDING")
         #ifdef USE_SD 
           stopRecording();
         #endif
         delay(100); //wait a bit
         recorderActive=false;
         update_screen(5,0);
       }
   }
 else // ************** BUTTON PRESSED handling during NON-recording outside of AUTOREC******/ 
  {
  
  updateButtonStatus();
  
  //rightbutton is completely dedicated to detectormode
   if (micropushButton_R.risingEdge()) //and (LeftButton_Mode!=MODE_PLAY)) 
     {
        D_PRINTLN("Update PButton R") 
        if (display_mode!=settings_page) 
            { //not in the settings menu so default setting of detectormode
              //allow when not in mode_play or in mode_play_direct
              if ( (LeftButton_Mode!=MODE_PLAY) or ( (LeftButton_Mode==MODE_PLAY) and (play_SR=SR_192K) ))
               {
                if (detector_mode==detector_heterodyne)
                  {
                    last_osc_frequency=osc_frequency; //store osc_frequency when leaving HT mode
                  }
                changeDetector_mode(cyclic_constrain(detector_mode,1,0,detector_passive));
                update_screen(6,0);
               }
            }
        else //in the settings menu for time/date the right_micropush can be used to step through hrs/min & yr/month/day 
              {
                if (set_menu_id[settings_page_nr]==SET_MENU_TIME)
                  { //jump to hours or minutes
                  timemenu_pos=cyclic_constrain(timemenu_pos,1,0,1);
                  update_screen(6,0); 
                  }
                if (set_menu_id[settings_page_nr]==SET_MENU_DATE)
                  { //jump to hours or minutes
                  timemenu_pos=cyclic_constrain(timemenu_pos,1,0,2);
                  update_screen(6,0); 
                  }   

              }    
    }
   //leftbutton function is based on leftbutton_mode)
    if (micropushButton_L.risingEdge()) 
    {   D_PRINTLN("Update PButton L")
        if (display_mode==settings_page) //get out of the settings directly
         { display_mode=last_display_mode-1;
           D_PRINT_FORMATLN(D_BOLDGREEN,"LBUTTON -> DISPLAY")
           LeftButton_Mode=MODE_DISPLAY;
           EncLeft_function=enc_menu;
           EncRight_function=enc_menu; 
           EncRight_menu_idx=MENU_GAIN_MIC;
         }

        if (LeftButton_Mode==MODE_DISPLAY) 
          {
           //display_mode+=1;
           display_mode=cyclic_constrain(display_mode,1,no_graph,waterfallgraph);
           D_PRINTXY("DISPLAY_MODE:",setDisplay[display_mode])
           tft.setRotation( 0 );
           if (display_mode==no_graph)
             {  tft.setScroll(0);
              }
           if (display_mode==spectrumgraph)
             {  tft.setScroll(0);
              }
             tft.fillScreen(COLOR_BLACK); //blank the screen
          }
        #ifdef USE_SD    
        if (LeftButton_Mode==MODE_PLAY)
          {
            if (playActive==false) //button pressed but not playing so start
              { playActive=true;
                startPlaying(play_SR);
              }
              else //button pressed and playing so stop
              { stopPlaying();
                playActive=false;
              }
          }    

        if (LeftButton_Mode==MODE_REC) 
        {
          if (recorderActive==false)  // when recorder is active interaction gets picked up earlier !!
            {   recorderActive=true;
                 startRecording();
              
            }

        }
        #endif
      //no function yet
      update_screen(5,0);
    }

    /************  LEFT ENCODER BUTTON CONFIRMATION *******************/
    if (encoderButton_L.risingEdge())
    { D_PRINTLN("Update ENCButton L") 
      EncLeft_function=!EncLeft_function; 
      
      if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_SETTINGS)  )   //settings_page 
          { 
            if (display_mode==settings_page) //user pressed the encoderbutton so wants to leave settings mode
              {
               display_mode=last_display_mode; //restore previous display_mode
               detector_mode=last_detector_mode;
             
               tft.fillScreen(COLOR_BLACK); 
               //restore right encoder mode
               EncRight_menu_idx=last_RightMenuidx;
                            
                             
               }
            else  //user enters settings mode
            { //store current modes
              last_display_mode=display_mode;  
              last_detector_mode=detector_mode;

              last_RightMenuidx=EncRight_menu_idx;
              last_RightMenufunc=EncRight_function;
              
              display_mode=settings_page; //show the other user-defined settings
              
              tft.setScroll(0);
              tft.fillRect(0,TOP_OFFSET-50,ILI9341_TFTWIDTH,ILI9341_TFTHEIGHT-TOP_OFFSET+50-BOTTOM_OFFSET,COLOR_BLACK); 

              EncLeft_function=enc_value; // option selection
              //start at the top options and zero set_menu_pos array
              memset(set_menu_pos,0,sizeof(set_menu_pos));
              set_menu_id[0]=Settings0Menu[0].menu_id;// SET_MENU_PAGE;
              set_menu_id[1]=Settings1Menu[0].menu_id;
              set_menu_id[2]=Settings2Menu[0].menu_id;
              EncRight_function=enc_value; //Right encoder will changes values
              
              }
         
         }
      
     /************************* SD ***********************************/
     if (SD_ACTIVE)
     {
        //user has confirmed the choice for the leftbuttonmenu
        if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_REC) and (EncLeft_function==enc_value) )  
          {D_PRINT_FORMATLN(D_BOLDGREEN,"LBUTTON -> REC")
            LeftButton_Mode=MODE_REC; //select the choosen function for the leftbutton
           initREC();
           } 

        if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_AUTOREC) and (EncLeft_function==enc_value) )  
          {D_PRINT_FORMATLN(D_BOLDGREEN,"LBUTTON -> REC")
           LeftButton_Mode=MODE_REC; //select the choosen function for the leftbutton
           D_PRINT_FORMATLN(D_BOLDGREEN,"START AUTOREC")
           AUTO_REC=true;
           autocounter=0;
           signal_LoF_bin= int((AREC_F*1000.0)/(SR_FFTratio));
      
           last_display_mode=display_mode;
           last_detector_mode=detector_mode;
           display_mode=no_graph;
           
           update_screen(4,0); 
           tft.fillScreen(COLOR_BLACK);

           D_PRINT_FORMATLN(D_BOLDGREEN,"AUTOREC WAIT") 
                               
           initREC();

           } 

       //play menu is active, user is selecting files
        if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY) and (EncLeft_function==enc_value)) //choose to select values
         { //keep track of the SR
           //last_SR=oper_SR; //store the last set SR
           //SR=play_SR;
           D_PRINT_FORMATLN(D_BOLDGREEN,"LBUTTON -> PLAY")
           LeftButton_Mode=MODE_PLAY; // directly set LEFTBUTTON to play/stop mode
           last_osc_frequency=osc_frequency; //keep track of heterodyne setting
           initPlay(); //switch SD to playing
           D_PRINT_FORMAT(D_BOLDGREEN,SR[play_SR].txt);
           set_SR(play_SR); //switch to the play SR
           //shut down input
           set_InputMixer(in_player);    //switch on player as input

           if (play_SR!=MAX_play_SR)                      
              {set_OutputMixer(passivemixer); //no processing                       
              }

           countRAWfiles(); // update the filelist
           //set the right encoder to samplerate
           EncRight_menu_idx=MENU_SR;
           EncRight_function=enc_value;
          
         }
         
         
         if (EncLeft_function==enc_menu) //user has pressed to go back to the menu, restart the detector
         {
            //restore operational SR setting and last_osc for HT
            osc_frequency=last_osc_frequency;
            set_SR(oper_SR);
                                 
            if ((detector_mode==detector_heterodyne) or (detector_mode==detector_Auto_heterodyne))
                {set_OutputMixer(heterodynemixer);
                 }
            else 
                {set_OutputMixer(granularmixer);
                }    
        
           #ifdef USE_ADC_IN
             if (ADC_ON)
              {set_InputMixer(in_adc); //switch on the ADC-line
               set_mic_gain(mic_gain);
              }
             else
           #endif
             {set_InputMixer(in_mic); //switch on the mic_input
               set_mic_gain(mic_gain);
              }
            
         }

        //automatically change LEFTbutton back to displaymode if it was on play or record previously
        if ((EncLeft_function==enc_menu) and ((LeftButton_Mode==MODE_PLAY) or(LeftButton_Mode==MODE_REC)) )
          { D_PRINT_FORMATLN(D_BOLDGREEN,"LBUTTON -> DISPLAY")
            LeftButton_Mode=MODE_DISPLAY;
           
           set_SR(oper_SR);
           }
                
     } //END SD_ACTIVE

     update_screen(4,0);
    } //END encoder Left risingedge

    /************  RIGHT ENCODER BUTTON *******************/

    if (encoderButton_R.risingEdge())
    {  D_PRINTLN("Update ENCButton R") 
      if ((display_mode==settings_page) )
         {
          #ifdef USE_EEPROM 
           EEPROM_SAVE();
           showSaved();
          #endif  
            
         }
      else {
       EncRight_function=!EncRight_function; //switch between menu/value control
      }

      update_screen(4,0);
    }

  } // ************** END NORMAL BUTTON PROCESSING

 } //end IF !AUTOREC

}
// **************************  END BUTTONS

//###########################################################################
//###########################################################################
//##########################   MAIN ROUTINE   ###############################
//###########################################################################
//###########################################################################

void setup() {
  #ifdef DEBUG
     SERIALINIT
     D_PRINT_FORMATLN(D_BOLDGREEN,"********** START DEBUGGER ************");
    
  #endif  
  //setup Encoder and pushButtons with pullups
  ButtonsEncoders_pullup();

  //test if LEFT MICROPUSH is pressed only needed when EEPROM is used
  boolean reset_toDefault=false;
  #ifdef USE_EEPROM
  micropushButton_L.update();
  
  if (micropushButton_L.read()==0)
    {reset_toDefault=true;
     }

  #endif
  tft.begin();
  
  sgtl5000.enable();
  
  delay1.delay(0,LR_DELAY); //set LR channels for pseudo stereo
  sgtl5000.muteHeadphone(); //shut down headphones
  sgtl5000.volume(0);
  sgtl5000.inputSelect(myInput);
    
  sgtl5000.micGain (mic_gain);
  //sgtl5000.adcHighPassFilterDisable(); // does not help too much!
  sgtl5000.lineInLevel(0); //shutdown line-in as a default
  #ifdef USE_HIPASS_FILTER
    biquad1.setCoefficients(0, allpass);
  #endif
  delay(500);
  
  initTFT(); //blank startscreen 
        
  EncLeft_menu_idx=BaseMenu[0].menu_id; //default 1st option
  EncRight_menu_idx=BaseMenu[1].menu_id; //default 2nd option

  // Audio connections require memory.
  AudioMemory(300);

  //set time provider
  setSyncProvider(getTeensy3Time);
 
  #ifdef ADAPTED_SGTL_LIB //the adapted audio-library allows direct control of several registers of the SGTL5000
     D_PRINT_FORMATLN(D_BOLDGREEN,"ADAPTED SGTL USED")
     delay(10);
     sgtl5000.SET_CHIP_MIC_REF(sgtl5000_ref_ctrl); // default 0x1F2 requires adapted control_sgtl5000.h 
  #endif

#ifdef USE_ADC_IN //capture AUDIO over de ADC (A2, pin16) 
   D_PRINT_FORMAT(D_BOLDGREEN,"ADC_IN samplerate :")
   D_PRINT_FORMATLN(D_BOLDGREEN, F_SAMP);
   ADC_modification(F_SAMP,0);  //call to audio_mods
#endif 

//test if user requests to reset all to defaults (left-pushbutton down during startup)
if (reset_toDefault)
    { showresettoDefault();
      } 

/* EEPROM CHECK  */
#ifdef USE_EEPROM
if (reset_toDefault)
  { EEPROM_SAVE();
    D_PRINT_FORMATLN(D_BOLDRED,"EEPROM RESET")
    delay(100);
    } 

if (EEPROM_LOAD()==false) //load data fromEEprom, if it returns false (probably due to a change in structure) than start by saving default data"
   { EEPROM_SAVE();
     D_PRINT_FORMATLN(D_BOLDRED,"EEPROM RESET")
   }
 D_PRINT_FORMATLN(D_BOLDGREEN,"EEPROM LOADED")  
 #endif   

 StartupScreen();

#ifdef ADVANCEDMENU
  D_PRINT_FORMATLN(D_BOLDGREEN,"ADVANCEDMENU ON")
#endif

   //finalize by showing a startupscreen
   #ifndef USE_ADC_IN
    //ADC conflicts with ADC for audio !
    analogReference(DEFAULT);
    analogReadResolution(12); //10 bits so 0-1023 range readouts
    analogReadAveraging(32); //32 is max
    delay(500);
    
    int mv1 = 1195*4096/analogRead(71); //3v3 in at Teensy
    tft.print("VREF(3v3):");
    tft.println(mv1);
    if (mv1>3280)
       {tft.print(" -NORMAL");
       }
    else if (mv1>3250)
    { tft.print("  -LOWERED");
        
    }
    else if (mv1>3200)
      {tft.print(" -LOW");}
       

  
      #ifdef USE_VIN_ADC

        delay(500); // give the system some time to setup 
        adc->setAveraging(32, ADC_0); // set number of averages
        adc->setResolution(12,ADC_0); // set bits of resolution
        adc->setReference(ADC_REFERENCE::REF_1V2, ADC_0);
        adc->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED,ADC_0); // change the conversion speed
        adc->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED,ADC_0); // change the sampling speed
        //R1=17.33K R2=2.161 RT=19.49  2.161/19.49=0.111
        delay(500); // give the system some time to setup 
        VIN= (adc->analogRead(A3))*2.702;///1024; //rough estimate of battery on VIN based on measurements
        
        tft.print(" VIN ");
        tft.println(VIN);
      
      #endif 
  #endif 
  

  
  sgtl5000.volume(float(volume*0.01));
  sgtl5000.unmuteHeadphone();
  
  
  //Init SD card use
  // uses the SD card slot of the Teensy, NOT that of the audio board !!!!!
  if(!initSD())
    { SD_ACTIVE=false;
      D_PRINT_FORMATLN(D_BOLDRED,"NO SDCARD FOUND")
    }
  else  {
      SD_ACTIVE=true;
      D_PRINT_FORMATLN(D_BOLDGREEN,"SDCARD FOUND")
      countRAWfiles();
    }

/******* BUILD MENUSTRUCTURE BASED ON SETTINGS AND AVAILBLE SD_CARD ***************************************************/
build_menu_structures();

// ***************** SETUP AUDIO *******************************
D_PRINTLN("***SETUP AUDIO***")
set_SR (oper_SR); //set operational sample rate
//set_freq_Oscillator (osc_frequency);

set_InputMixer(in_mic); //microphone active
set_OutputMixer(heterodynemixer);

// the Granular effect requires memory to operate
granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

//switch to the preset or default detector_mode
changeDetector_mode(detector_mode); 

 // update all encoders/buttons before starting to prevent false changes
updateButtonStatus();
delay(2000); //wait with clearing the screen
//clear the screen gently
  for (uint16_t i=ILI9341_TFTHEIGHT; i>0; i--)
    { delay(1);
      tft.drawFastHLine(0,i,ILI9341_TFTWIDTH,COLOR_BLACK);
    }

update_screen(255,0);
    
} // END SETUP

//************************************************************************         LOOP         ******************
//start the processing loop !
void loop()
{
  // If we are recording, carry on...
  if (LeftButton_Mode == MODE_REC) 
  {
    #ifdef USE_SD
      writeREC(&recorder);
    #endif
        
    if (recorderActive)
    {
      //use FFT to check if incoming signals still are ultrasound
      if (myFFT.available()) //check if a sample is available
         { updateFFTanalysis(); //do the analysis
           if (sample_UltraSound)
             {time_since_EndDetection=0;
             if ((!AUTO_REC) and (record_detector==detector_Auto_heterodyne))
              {adjust_Heterodyne_Oscillator();   
              }
             } 
          }

     //we are autorecording then stop recording after AREC_D seconds or if we have not heard anything for AREC_B seconds
     if ((AUTO_REC) and ( (recording_running>(AREC_D*AREC_BLOCK) or (time_since_EndDetection>AREC_B*AREC_BLOCK) ) ))
     { 
      #ifdef USE_SD
        stopRecording();
        recorderActive=false;
        delay(100);
        recording_stopped=0;
      #endif
      }
     
    }
    
    
  }
  #ifdef USE_SD
  if (LeftButton_Mode == MODE_PLAY) 
    {if (playActive)
        {continuePlaying();
        }
    }
  #endif
 update_Buttons();
 
  // during recording screens are not updated to reduce interference !
 if (not recorderActive)
  { update_Encoders();
    update_Graphs();
    
    //update the time on screen regularly 
    struct tm tx = seconds2time(RTC_TSR);

    if (AUTO_REC)
      {
        if (tx.tm_sec/2!=old_time_sec)
          {
            updateAUTORECstatus();
            
          }
        old_time_sec=tx.tm_sec/2;  
        
      }
    
    if (tx.tm_min!=old_time_min) //minutes have changed
    { if (not recorderActive)
          {updateTime();
          } 
      old_time_min=tx.tm_min;
      #ifdef USE_VIN_ADC
        VIN= (adc->analogRead(A3))*2.702; //empirical !!
      #endif  
      
    }

  }
 
  
  
}