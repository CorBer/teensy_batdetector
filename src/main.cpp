
/**********************************************************************
 * TEENSYBAT DETECTOR (for TEENSY 3.6) 
 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
 * 
 *   Hardware and PCB by Edwin Houwertjes 
 *     see:  https://drive.google.com/drive/folders/1NRtWXN9gGVnbPbqapPHgUCFOQDGjEV1q
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
 *  startup image by paul van Hoof https://www.paulvanhoof.nl/ Watervleermuis (M.Daubentoni) Daubenton's bat
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

/* TEENSY 3.6 PINSETUP (20210309) 
    pins marked with X are default in use 
    pins marked with * are used for optional functions
    pins mared with . are not (yet) in use
   
                 GND X                X Vin  - PREAMP V+  
       GPS TX -> RX0 *                X  Analog GN D    
       GPS RX -> TX1 *                X  3.3V - MEMS MIC         
                   2 .                X  23 AUDIO -LRCLK         
      I2C reserved 3 *                X  22 AUDIO -TX             
      I2C reserved 4 *                X  21 <-TFT CS                
                   5 .                X  20 <-TFT DC             
       AUDIO MEMCS 6 X                X  19 AUDIO - SCL         
       AUDIO MOSI  7 X                X  18 AUDIO - SDA         
                   8 .                *  17 A3                <  TFT-PWM option        
       AUDIO BCLK  9 X                *  16 A2 - ADC          <- ADC_IN function                               
       AUDIO SDCS 10 X                X  15 AUDIO -VOL                      
       AUDIO MCLK 11 X                X  14 AUDIO -SCLK                     
       AUDIO MISO 12 X                X  13 AUDIO -RX                       
                3.3V X                X  GND                 
                  24 .                .  A22 DAC1
                  25 .                .  A21 DAC0 
                  26 .                X  39  TFT MISO
      TFT SCLK->  27 X                X  38  MICROPUSH_L
      TFT MOSI->  28 X                X  37  MICROPUSH_R
     ENC_L-BUTTON 29 X                X  36  ENC_R-BUTTON
     ENC_L A      30 X                X  35  ENC_R A
     ENC_L B      31 X                X  34  ENC_R B
     DS18B20  T   32 *                .  33

***********************************************************************
 *  TEENSY 3.6 BAT DETECTOR 
 *  Copyright code (c) 2018/2019/2020/21, Cor Berrevoets, registax@gmail.com
 *
 *  HARDWARE:
 *     TEENSY 3.6
 *     TEENSY audio board
 *     Ultrasonic microphone with seperate preamplifier connected to mic/gnd on audioboard
 *       eg. Knowles MEMS SPU0410LR5H-QB
 *           ICS - 40730 
 *     //added july2020: allow ADC-input for microphones
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
 *                       Frequency divider (1/10 implemented)
 *                       Passive (no processing)
 *
 *  Sample rates up to 384k
 *   
 *  Record raw data/WAV files
 *  Record data with GUANO specifications
 *  Play raw data and WAV-files(user selectable) from the SDcard using time_expansion 
 *
 * 
 * **********************************************************************/
 

// FUTURE UPDATE IDEAS
// -ALLOW SWAPPING OF THE REC_BUTTON FROM LEFT ->RIGHT
// -ALLOW RECORDING OF FILES IN FOLDERS (YYMMDD)
// -SPLIT SETTING WINDOW INTO 2 SECTIONS WITH A SMALL WATERFALL AREA
// -LONG FILENAMES UNDER OTHER SD LIBRARY
// -TE REPLAY SPEED and read WAVFILE samplerate
// -WATERFALL during 1/10 replay

// all defines are in BAT_DEFINES 
#define batversion "v1.1DEV 20210409" 
#define versionno 1010 // used in EEProm storage <1000 is pre-release 
                      //  1010 is a the 2nd development version update 

#include "bat_defines.h"

#include <Arduino.h>
#ifndef __MKL26Z64__
	// only SIM_UIDML will probably change !!
  unsigned long chipNum[4] = { SIM_UIDH, SIM_UIDMH, SIM_UIDML, SIM_UIDL }; //read unique chipno from Teensy
#else
	unsigned long chipNum[4] = { 0L, SIM_UIDMH, SIM_UIDML, SIM_UIDL };
#endif

//HARDWARE settings

//allows usage of ADC as the primary audio-source 
#ifdef USE_ADC_IN  //audio in over A2 (pin 16) 0-1.2v max (see https://forum.pjrc.com/threads/31700-Audio-Library-recommended-circuit-for-adc 
  #include "audio_mods.h" // setup of high-speed ADC sampling 
  #define AUDIO_INPUT_ADC     2
  #define F_SAMP 384000 
  #define ADC_PIN A2
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
#include "bat_EEPROM.h" //routines to read/write EEprom

#include <SPI.h>

#ifdef USE_SD
  #include "bat_sd.h" //routines to read/write SD
#endif

// ******************** BUFFER FOR TIMEEXPANSION ROUTINE ********************
#define GRANULAR_MEMORY_SIZE 16*1024  // enough for 25 ms at 281kHz sampling 
int16_t granularMemory[GRANULAR_MEMORY_SIZE]; //2 bytes per position !!

/**************************************      FUNCTIONS      ******************************************/

/**************************************  MAIN SCREEN UPDATE ******************************************/

void update_screen(uint8_t from, uint8_t update_type) {
  //update_type 
  /*0=all
    1=change from encoder RIGHT rotation
    2=change from encoder LEFT rotation
  */
  //D_PRINT("UPDATESCREEN")
  //D_PRINTXY(from, update_type);

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
    //***************** HEADER  ********************/
   {  showHeader();
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
       
   if (showEESaved)     
       {
         showSaved();
         showEESaved=false;
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
           sine1.amplitude(sine_amplitude);//sine ON
         }

  if (detector_mode==detector_Auto_heterodyne)
         { set_OutputMixer(heterodynemixer);//connect output to heterodyne
           sine1.amplitude(sine_amplitude);//sine ON
         }

  if (detector_mode==detector_Auto_TE)
         { granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE); //setup the memory for timeexpansion
           
           #ifdef USE_TEFACTOR
              if (detune_factor==100)
                 {set_OutputMixer(granularmixer);
                 }
              else//connect output to granularHT
                 { set_OutputMixer(granularHTmixer); 
                  }
                  
           #else
              set_OutputMixer(granularmixer); //connect output to granular
           #endif
           
           granular1.setTESpeed(TE_speed); // set TE speed
           sine1.amplitude(0); //shutdown HETERODYNE SINE 
           sine1.frequency(0);
          
         }

  if (detector_mode==detector_divider)
         { granular1.beginDivider(GRANULAR_MEMORY_SIZE);
           set_OutputMixer(granularmixer);
           granular1.setdivider(FD_divider); //V112 changes to effect_granular have been made !!!
           sine1.amplitude(0); //shutdown HETERODYNE SINE
           sine1.frequency(0);
         }

  if (detector_mode==detector_passive)
         { set_OutputMixer(passivemixer);
           sine1.amplitude(0); //shutdown HETERODYNE SINE
           sine1.frequency(0);
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
  outputMixer.gain(granularHTmixer,0);  //shutdown granular with HT output
  outputMixer.gain(granularmixer,0);  //shutdown granular output
  
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

void checkSRinwav()
{
  

}

// **************** ******************************PLAYING ************************************************
void startPlaying(int SRate) {
//      String NAME = "Bat_"+String(file_number)+".raw";
//      char fi[15];
//      NAME.toCharArray(fi, sizeof(NAME));
  D_PRINTLN("START PLAYING");
 last_HI_pass=HI_pass;
 HI_pass=HI_PASS_OFF;
 setHiPass();
 
 fileselect=cyclic_constrain(fileselect,0,0,filecounter-1);
 strncpy(filename, filelist[fileselect],  13);
 knownSR=0;
 recorded_SR=0;
//read the samplerate of the WAV file
 
 if ((String(filename).indexOf(".WAV")>=0) or (String(filename).indexOf(".wav")>=0))
 {
      File audioFile = SD.open(filename, FILE_READ);
      byte audiobuffer[28];
      audioFile.read(audiobuffer, 28); 
      D_PRINTXY(" ",filename);
      recorded_SR= audiobuffer[24]+audiobuffer[25]*256+audiobuffer[26]*256*256;
      D_PRINTXY("recordedSR ",recorded_SR);
      //check if the SR is part of our system
      for (int i=0; i<SR_MAX+1; i++) 
          {if (SR[i].osc_frequency==recorded_SR) 
            { D_PRINTXY("known SR ",SR[i].txt);
              knownSR=i;
            }  
          }
        
 }
  playActive=true;
  //direct play is used to test functionality based on previous recorded data
  //this will play a previous recorded raw file through the system as if it were live data coming from the microphone
  //a SRATE of 192K is used to replay the raw data

  //allow settling
  delay(100);
  
  if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY)
   {
    D_PRINT("SET PLAY SR ");
    
    if (SRate<MAX_play_SR) //check if we play TE-style or DIRECT
         {SRate=cyclic_constrain(play_SR,0,SR_8K,MAX_play_SR);
          }
    else { //playDirect
        if (knownSR!=0)
         { D_PRINTLN("USE KNOWN SR"); 
           SRate=knownSR; //recording is based on a known system samplerate
          }
         else //unknown base
         { D_PRINTLN("UNKNOWN, USE 192K"); 
          SRate=SR_192K; //recording is based on a non-system samplerate so use default 192K
         } 
     }      
    D_PRINTLN(SRate); 
    D_PRINT(SR[SRate].txt);   
    set_SR(SRate);
   }
   
  continousPlay=false; 
  if (SRate<MAX_play_SR)                      
              {set_OutputMixer(passivemixer); //no processing    
               playDirect=false;                   
              } 
          else //user select to play direct (as if data comes in through the microphone)
          {  D_PRINT("PLAY DIRECT ");
             changeDetector_mode(detector_mode);
             continousPlay=true;
             playDirect=true;
          }
   
 player.play(filename);
 
}

// ********************************************************** STOP PLAYING ***************************************
void stopPlaying() {
  
  player.stop();
  D_PRINTLN("STOP PLAYING");
  playActive=false;
  update_screen(3,0); 
  // //restore last SR setting when leaving the PLAY menu
  HI_pass=last_HI_pass;
  setHiPass();

  if ((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY) and (EncLeft_function==enc_menu))
    {
    D_PRINTLN("SWITCH BACK TO DETECTOR");
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
          { //startPlaying(play_SR); 
            D_PRINTLN(" CONTINUE");
            playActive=true;
            player.play(filename);
            }
     }
   }
}
#endif //USE SD

// **************** General graph and detector selective functions *******************************************************
void update_Graphs(void) 
{ tft.setCursor(0,90);
// code for 256 point FFT
//TODO MOVE TO BAT_FFT for the calcs
 if (myFFT.available()) {
  const uint16_t Y_OFFSET = TOP_OFFSET;
  static int count = TOP_OFFSET;
         updateFFTanalysis(); //get FFT data and check for ultrasound 
         updateTime();
      
//
         
         
/************************** TE MAIN DETECTION **************************/

/*************************  SIGNAL DETECTION ***********************/
    //signal detected in the Ultrasound range
    if (sample_UltraSound) //ultrasound in the sample !
      { 
             #ifdef USE_TEFACTOR
             if (detune_factor!=100)
                {DETUNE=int(FFT_peakF_bin*(SR_FFTratio)/TE_speed);
                uint8_t detune=constrain(99-detune_factor+50,50,99);
                //D_PRINT("DETUNE FFT")
                //D_PRINTLN(DETUNE)
                freq_Oscillator = (DETUNE) * (AUDIO_SAMPLE_RATE_EXACT / SR_real)*detune/100.0f;
                AudioNoInterrupts();
                sine_detune.frequency(freq_Oscillator);
                sine_detune.amplitude(sine_amplitude);
                AudioInterrupts();
                }
             else
             { sine_detune.amplitude(0);
             }   
              #endif
              
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
                 set_freq_Oscillator(osc_frequency*AHT_factor/100.0f); //adapt AHT on the peak or up to 20% lower than the peak
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
                // update power-spectrumdisplay after every xth FFT sample with bat-activity    
                if ((powerspectrumCounter>powerspectrumMaxcounts)  )
                      { D_PRINTLN("PS")
                        showPowerSpectrum();
                          drawScale();
                          drawScaleMarker(); //position the dot on the scale
                        }
                  
                break;   
              }   

              // update power-spectrumdisplay after every xth FFT sample with bat-activity
              case waterfallgraph:
                if ((powerspectrumCounter>powerspectrumMaxcounts)  )
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
      
      
     if (count >= ILI9341_TFTHEIGHT-BOTTOM_UPPERPART) 
       {count = Y_OFFSET;
       }
   }

   //automatic recording of signals if we are NOT recording allready
   if (recorderActive==false)
        //if autorecording is on and a signal was found and the last recording ended more than AREC_P* 5seconds ago
        if ((AUTO_REC) and (sample_UltraSound) and (recording_stopped>(AREC_P*AREC_5SEC)) )
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
   D_PRINT("Update Enc ") 

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
      D_PRINT(" LEFT ")
      if (encodermode==enc_menu)
         {D_PRINT("MENU ")
         }
      else
        {D_PRINT("VALUE ")
        }
       D_PRINT(menu_idx)
       D_PRINT(" ")
       D_PRINTLN(menu_id)  
    }

   if (Encoderside==enc_rightside)
    { encodermode=EncRight_function;
      change=EncRightchange;
      menu_idx=EncRight_menu_idx;

      if ((LeftBaseMenu[EncLeft_menu_idx].menu_id!=MENU_SETTINGS))  //regular functionality left Encoder not on SETTINGS
             { menu_id=RightBaseMenu[menu_idx].menu_id;
               }
      else
          {if (EncLeft_function!=enc_value) //not inside the settings menu but left Encoder is on SETTINGS 
              { menu_id=RightBaseMenu[menu_idx].menu_id;
               }        
           else //active inside settings menu
              { 
              }   
          }     
      choices=RightMenuOptions; //available menu options
      D_PRINT(" RIGHT ")
      if (encodermode==enc_menu)
         {D_PRINT("MENU ")
         }
      else
        {D_PRINT("VALUE ")
        }
      D_PRINT(menu_idx)
      D_PRINT(" ")
       
      D_PRINTLN(menu_id) 
                
    }


  /************************react to changes from the encoder*************************/

  //encoder is in menumode
  if (encodermode==enc_menu)
    { menu_idx=menu_idx+change;
         
     //allow revolving choices limited to a specific encoder (left or right !!)
      menu_idx=cyclic_constrain(menu_idx,0,0,choices-1);
      
      if (Encoderside==enc_leftside)
          { EncLeft_menu_idx=menu_idx; //limit the menu
            menu_id=LeftBaseMenu[menu_idx].menu_id;
            D_PRINT(" MENU ")
            D_PRINTLN(LeftBaseMenu[menu_idx].menu_txt)  
               }

     //limit the changes of the rightside encoder for specific functions
      if (Encoderside==enc_rightside)
          { EncRight_menu_idx=menu_idx; //limit the menu
            menu_id=RightBaseMenu[menu_idx].menu_id;
            D_PRINT(" MENU ")
            D_PRINTLN(LeftBaseMenu[menu_idx].menu_txt)
               }
    }

  //encoder is in valuemode and has changed position so change an active setting
  if ((encodermode==enc_value) and (change!=0))
    { //changes have to be based on the choosen menu position
      /******************************VOLUME  ***************/
      if (menu_id!=MENU_SETTINGS)
           {menuAction(menu_id,change); //act on changes when not in settings menu
           }
      /******************************SETTINGS MENU  LEFTSIDE ENCODER INDEX ***************************/

        // for the settings menu the left encoder selects the option and the right encoder allows changes the values
        if ((menu_id==MENU_SETTINGS) and (Encoderside==enc_leftside))
          { uint8_t idx=settings_page_nr;
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
            if (idx==3)
              {set_menu_id[idx]=Settings3Menu[set_menu_pos[idx]].menu_id;
              }   
            #ifdef USE_GPS  
            if (idx==4)
              {set_menu_id[idx]=Settings4Menu[set_menu_pos[idx]].menu_id;
              }    
            #endif  

            D_PRINT("SETTINGS PAGE ")
            D_PRINT(settings_page_name[idx]);
            D_PRINT(" ID ")
            D_PRINTLN(set_menu_id[idx]);
            
          }
     
      /******************************SETTINGS MENU RIGHTSIDE ENCODER VALUES ***************************/
      
        // change the settings on the SETTINGSPAGE when turning the right encoder
        if ((menu_id==MENU_SETTINGS) and (Encoderside==enc_rightside))
        { 
          //  //to prevent a cascade of changes!
          // if ((set_menu_id[0]==SET_MENU_PAGE) and (settings_page_nr==0))
          //    { show_next_settings_page(change);
          //     } 
          //  else  if ((set_menu_id[1]==SET_MENU_PAGE) and (settings_page_nr==1)) 
          //   { show_next_settings_page(change);
          //     }    
          //    else  if ((set_menu_id[2]==SET_MENU_PAGE) and (settings_page_nr==2)) 
          //      { show_next_settings_page(change);
          //        }  
          //      else  if ((set_menu_id[3]==SET_MENU_PAGE) and (settings_page_nr==3)) 
          //           { show_next_settings_page(change);
          //             }  
          //        else  if ((set_menu_id[4]==SET_MENU_PAGE) and (settings_page_nr==4)) 
          //           { show_next_settings_page(change);
          //             }     
           
           //change 20210326
           if (set_menu_id[settings_page_nr]==SET_MENU_PAGE)
                   { show_next_settings_page(change);
                       D_PRINT(" SWITCH SETTINGS TO ")
                       D_PRINTLN(settings_page_name[settings_page_nr])
                    } 
       
          //change settings for the active page and item
          else
            {settingsMenuAction(set_menu_id[settings_page_nr],change);
            }
          //SPECIAL MENU ACTIONS 
          if (set_menu_id[settings_page_nr]==SET_MENU_TIME) //we are on the time menusection
            {
             updateTimeMenu(change);
            }
             //set date
          if (set_menu_id[settings_page_nr]==SET_MENU_DATE)
            { updateDateMenu(change);
            }

           /* COLOR MENU */
           if ((set_menu_id[settings_page_nr]>=SET_MENU_COLORS) and (set_menu_id[settings_page_nr]<=SET_MENU_BCKCOLORS))
           {
                cwheelpos[set_menu_id[settings_page_nr]-SET_MENU_COLORS]+=change*4;
                uint8_t cwheel=cyclic_constrain(cwheelpos[set_menu_id[settings_page_nr]-SET_MENU_COLORS],0,0,cwheelmax);
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
                   {if (play_SR>MAX_play_SR)
                      {play_SR=MAX_play_SR;}
                    play_SR=cyclic_constrain(play_SR,EncRightchange,SR_8K,MAX_play_SR);
                    if (play_SR>=MAX_play_SR)
                        {play_SR=SR_192K;}
                    
                    if (playActive)
                         #ifdef USE_SD
                          {stopPlaying();
                          }
                         #endif  
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
       { tft.sleep(false);
         
         #ifdef USE_PWMTFT
           set_backlight(tft_backlight);
         #endif

          D_PRINT_FORMATLN(D_BOLDGREEN,"MANUAL STOP AUTOREC FUNCTION")
         
         #ifdef USE_SD
          AUTO_REC=false; //stop auto_recording 
          if (recorderActive) //manual stop during a recording 
              { D_PRINTLN(" STOP ACTIVE AUTO_REC")
                stopRecording();
               }
                //stop recording and bring back the previous detector_mode and display_mode
          else  //detector is in autorec mode but was not recording so in between two recordings stopped
              { D_PRINTLN(" STOP PAUSE AUTO_REC")
                stopRecording();
              }

          delay(100); //wait a bit
          D_PRINTLN(" LBUTTON-> DISPLAY")
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
              if ( (LeftButton_Mode!=MODE_PLAY) or ( (LeftButton_Mode==MODE_PLAY) and (play_SR>=SR_192K) ))
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
            if (playActive==false) //button pressed but not playing so start Playing
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
               D_PRINTLN("LEAVING SETTINGS") 
               display_mode=last_display_mode; //restore previous display_mode
               detector_mode=last_detector_mode;
             
               tft.fillScreen(COLOR_BLACK); 
               //restore right encoder mode
               EncRight_menu_idx=last_RightMenuidx;
               EncRight_function=last_RightMenufunc;                            
               }
            else  //user enters settings mode
            { //store current modes
              D_PRINT("ENTERING SETTINGS ") 
              D_PRINTLN(settings_page_name[settings_page_nr])
              last_display_mode=display_mode;  
              last_detector_mode=detector_mode;

              last_RightMenuidx=EncRight_menu_idx;
              last_RightMenufunc=EncRight_function;
              
              display_mode=settings_page; //show the other user-defined settings
              
              tft.setScroll(0);
              tft.fillRect(0,TOP_OFFSET-50,ILI9341_TFTWIDTH,ILI9341_TFTHEIGHT-TOP_OFFSET+50-BOTTOM_UPPERPART,COLOR_BLACK); 

              EncLeft_function=enc_value; // option selection
              //start at the top options and zero set_menu_pos array
              memset(set_menu_pos,0,sizeof(set_menu_pos));

              for (uint8_t i=0; i<settings_menu_pages; i++)
              { set_menu_id[i]=SET_MENU_PAGE; //Settings0Menu[0].menu_id;// RESET SET_MENU_PAGE to TOP;
               }
              // set_menu_id[1]=//Settings1Menu[0].menu_id;
              // set_menu_id[2]=//Settings2Menu[0].menu_id;
              // set_menu_id[3]=//Settings3Menu[0].menu_id;

              // #ifdef USE_GPS
              //   set_menu_id[4]=Settings4Menu[0].menu_id;
              // #endif
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
           if (tft_sleep) 
              {
              tft.setFont(Arial_18);
              tft.setCursor(0,90);
             #ifdef USE_PWMTFT 
              tft.println("      Screen OFF");
              tft.println("");
              tft.println(" Lft. Button to restart");
              delay(3000);
              set_backlight(0);
             #else
              tft.println(" Screen will sleep ");
              tft.println("");
              tft.println(" Lft. Button to restart");
             #endif 
              tft.sleep(true);
              }
          
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
           if (play_SR>=MAX_play_SR)
               { 
                D_PRINTLN("SET DIRECTPLAY");
                play_SR=SR_192K;
               if ( knownSR >0) //recording uses a known Samplerate, use that !
                 {play_SR=knownSR;
                  D_PRINTXY("USE KNOWN SR ",knownSR);
                 }
               }

           D_PRINT_FORMAT(D_BOLDGREEN,SR[play_SR].txt);
          
           set_SR(play_SR); //switch to the play SR
           //shut down input
           set_InputMixer(in_player);    //switch on player as input

           if (play_SR<MAX_play_SR)                      
              {set_OutputMixer(passivemixer); //no processing                       
              }

           count_SDfiles(); // update the filelist
           //set the right encoder to samplerate
           EncRight_menu_idx=MENU_SR;
           EncRight_function=enc_value;
          
         }
         
         
         if (EncLeft_function==enc_menu) //user has pressed to go back to the menu, restart the detector
         {
            //restore operational SR setting and last_osc for HT
            osc_frequency=last_osc_frequency;
            set_SR(oper_SR);
            D_PRINTXY("DETECTORMODE ",DT[detector_mode])                     
            if ((detector_mode==detector_heterodyne) or (detector_mode==detector_Auto_heterodyne))
                {set_OutputMixer(heterodynemixer);
                 }
            else 
              #ifdef USE_TEFACTOR
                { D_PRINTXY("DETUNE ",detune_factor)
                  if (detune_factor==100)
                     {set_OutputMixer(granularmixer);
                     }
                   else
                       {set_OutputMixer(granularHTmixer);
                     }
                }    
              #else
                {set_OutputMixer(granularmixer);
                }    
              #endif  
          
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
        
           EEPROM_SAVE();
           showEESaved=true;
           //showSaved();
         
            
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

  //test if LEFT MICROPUSH is pressed, if so restart without reading the EEPROM settings (all defaults)
  micropushButton_L.update();
  boolean reset_toDefault=false;
  if (micropushButton_L.read()==0) 
    {reset_toDefault=true;
     }

  //initialize AUDIOBOARD processing
  sgtl5000.enable();
  sgtl5000.volume(0);
  sgtl5000.muteHeadphone(); //shut down headphones
  sgtl5000.lineInLevel(0); //shutdown line-in as a default   
  
  sgtl5000.inputSelect(myInput);
  sgtl5000.micGain (mic_gain);
  //sgtl5000.adcHighPassFilterDisable(); // does not help too much!
 
  #ifdef USE_HIPASS_FILTER
    biquad1.setCoefficients(0, allpass);
  #endif
  //delay(100);

  //initialize TFT
  tft.begin();
#ifdef USE_PWMTFT
  set_backlight(tft_backlight);
#endif  
  initTFT(); //blank startscreen 
  
  EncLeft_menu_idx=BaseMenu[0].menu_id; //default 1st option
  EncRight_menu_idx=BaseMenu[1].menu_id; //default 2nd option

  // Audio connections require memory.
  AudioMemory(300);

  //set time provider
  setSyncProvider(getTeensy3Time);
 

#ifdef USE_ADC_IN //capture AUDIO over de ADC (A2, pin16) 
   D_PRINT_FORMAT(D_BOLDGREEN,"ADC_IN samplerate :")
   D_PRINT_FORMATLN(D_BOLDGREEN, F_SAMP);
   ADC_modification(F_SAMP,0);  //call to audio_mods
#endif 

//test if user has requested to reset to defaults (left-pushbutton down during startup)
if (reset_toDefault)
    { showresettoDefault();
      } 

/* EEPROM CHECK  */
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
 
 //finalize by showing a startupscreen
 StartupScreen();

//additional hardware settings
 #ifndef USE_ADC_IN
    //ADC conflicts with ADC for audio !
    D_PRINT_FORMATLN(D_BOLDGREEN,"ADC_READOUT") 
    analogReference(DEFAULT);
    analogReadResolution(12); //12 bits so 0-4095 range readouts
    analogReadAveraging(32); //32 is max
    //delay(100);
    uint16_t mv1 = 1195*4096/analogRead(71); //3v3 in at Teensy
    tft.setFont(Arial_14);
    tft.print("VREF(3v3):");
    tft.println(mv1);
    if (mv1>3280)
       {tft.print(" -NORMAL");
       }
    else if (mv1<3250)
      {tft.print(" -LOW");}
 
  #endif 
   
  
  //Init SD card use
  // uses the SD card slot of the Teensy, NOT that of the audio board !!!!!
  if(!initSD())
    { SD_ACTIVE=false;
      D_PRINT_FORMATLN(D_BOLDRED,"NO SDCARD FOUND")
    }
  else  {
      SD_ACTIVE=true;
      D_PRINT_FORMATLN(D_BOLDGREEN,"SDCARD FOUND")
      count_SDfiles();
    }

/******* BUILD MENUSTRUCTURE BASED ON SETTINGS AND AVAILABLE SD_CARD ***************************************************/
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

sgtl5000.volume(float(volume*0.01));
sgtl5000.unmuteHeadphone();

 // update all encoders/buttons before starting to prevent false changes
updateButtonStatus();
//delay(2000); //wait with clearing the screen
//clear the screen gently
for (uint16_t i=ILI9341_TFTHEIGHT; i>0; i--)
    { delay(1);
      tft.drawFastHLine(0,i,ILI9341_TFTWIDTH,COLOR_BLACK);
    }

update_screen(255,0);

#ifdef USE_GPS

  GPSuart.begin(9600);
  
#endif
    
#ifdef USE_DS18B20
  ds.reset_search();

    if ( !ds.search(DSaddr)) {
      //Serial.println("No more addresses.");
      //Serial.println();
        ds.reset_search();
        delay(250);
      D_PRINTLN("NO DS18B20 FOUND")
      
    }
    else
    {  // set precision to lower necessary delay
        // 0   -   9  // 1   -  10 // 2   -  11 // 3   -  12 (default)

        // For 9 bit precision
        active_batThermo=true;
        int t_precision = 0;
        ds.select(DSaddr);    
        ds.write(0x4E);
        // write zero into the alarm registers
        ds.write(0);
        ds.write(0);
        // and write t_precision into the configuration register
        // to select the precision of the temperature
        ds.write(t_precision << 5);
        // Write them to the EEPROM
        ds.write(0x48);
        D_PRINTLN("GET DS TEMP")
        bat_tempC = readDS(true);
    }
#endif


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
         { updateFFTanalysis(); //do the analysis for ultrasound
           if (sample_UltraSound)
             {time_since_EndDetection=0;
             if ((!AUTO_REC) and (record_detector==detector_Auto_heterodyne))
              {adjust_Heterodyne_Oscillator();   
              }
             } 
          }

     //we are autorecording then stop recording after AREC_D* 5seconds or if we have not heard anything for AREC_B seconds
     if ((AUTO_REC) and ( (recording_running>(AREC_D*AREC_5SEC) or (time_since_EndDetection>AREC_B*AREC_1SEC) ) ))
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
    else
      {
          //30 second check for special functionality
          if (tx.tm_sec/30!=old_time_sec)
            {
              old_time_sec=tx.tm_sec/30;  
              #ifdef USE_GPS
              readGPS();
              #endif 
              
            }


      }  

   
    if (tx.tm_min!=old_time_min) //minutes have changed
    { 
      #ifdef USE_DS18B20
      if (active_batThermo)
       {
         D_PRINTLN("GET DS TEMP")
         bat_tempC = readDS(false);}
      #endif

      if (not recorderActive)
          {updateTime();
          } 
      old_time_min=tx.tm_min;
  
    }

  }
 
  
  
}