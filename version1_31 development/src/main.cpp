

/**********************************************************************
 * TEENSYBAT DETECTOR (build/tested on TEENSY 3.6/4.1) VERSION 1.3 202108XX
 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
 *
 *   Hardware and PCB development and a lot of testing was done by Edwin Houwertjes
 *     for hardware details see:  https://drive.google.com/drive/folders/1NRtWXN9gGVnbPbqapPHgUCFOQDGjEV1q
 *                           or:  https://www.teensybat.com
 *
 *   Development of V1.2 and V1.3 was done in close cooperation with:
 *        - Adrian Dexter
 *        - Thierry Arbault
 *
 *   Thanks for the ideas and feedback during testing!
 *
 *   Based on original code by DD4WH
 *   https://github.com/DD4WH/Teensy-Bat-Detector
 *
 *   TeensyForum-thread
 *   https://forum.pjrc.com/threads/38988-Bat-detector
 *
 *   Version 1.3 is build using Teensyduino 1.154
 *
 *   samplerate code design by Frank Boesing
 *   - Improved setI2S_freq(float fsamp) during replay from https://github.com/FrankBoesing/AudioTiming
 *   - Copyright (c) 2016, Frank Bösing, f.boesing@gmx.de
 *
 *   important tips on deepsleep modes with the SGTL5000 were supplied by WMXZ see https://github.com/WMXZ-EU/record_sgtl5000
 *
 *   sleepmodes for the T3.6 are based on https://github.com/duff2013/Snooze
 *
 *  startup image by paul van Hoof https://www.paulvanhoof.nl/ Watervleermuis (M.Daubentoni) Daubenton's bat
 *  *
 * ******************************    NOTICE    *********************************************************
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
 ********************************************************************************************************/
 /***********************************************************************
  *  TEENSY 3.6/4.1 BAT DETECTOR
  *  Copyright code (c) 2018/2019/2020/21, Cor Berrevoets, registax@gmail.com
  *
  *  HARDWARE:
  *     TEENSY 3.6/4.1
  *     TEENSY audio board
  *     Ultrasonic microphone with seperate preamplifier connected to mic/gnd on audioboard
  *       eg. Knowles MEMS SPU0410LR5H-QB
  *           ICS - 40730
  *     TFT based on ILI9341
  *     2 rotary encoders with pushbutton
  *     2 micro-pushbuttons
  *     SDCard
  *     optional hardware: DS18B20 temperaturesensor,
  *                        serial GPS module (eg Beitian 180/220)
  *
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
  *  Record WAV files
  *  Record data with GUANO specifications
  *  Play WAV-files(user selectable) from the SDcard both slowed down and as "live input"
  *
  * ************************LARGE MEMORY CONFIGURATION*********************
  * *  sample_buffer  uint8_t sample_buffer[BUFFSIZE]  defined in bat_vars, used to collect samples and write large chuncks of data to SD
  *       small configuration 96k (T36/T41 without PSRAM) large configuration 3072k (only on T41 with PSRAM)
  *
  * *  granularMemory defined in main.cpp and used during TimeExpansion live, larger means longer spans between pulses can be covered
  * *    in T36/T41 16k in RAM, in T41 with PSRAM 16k
  *
  * *  AUDIO
  *    rec_buffer_samples: defined in local version of AudioStream.h
  *      This buffer is important to cover the time elapsing during the write of sample_buffer to SD.
  *      If the buffer is smaller than the time it takes to write sample_buffer to SD some incoming data might be lost !!
  *      default size 200 sampleblocks (=25600 samples), but when on a T4.1 with PSRAM a default of 2000 are available.
  *      Example: 200 sampleblocks at 281K samplerate is around 9ms of sound, 2000 is 90ms of incoming sound. The data
  *               has to be written to SD in that timeframe.
  *
  *    ring_buffer_samples: defined in local version of AudioStream.h ONLY available for PSRAM (T4.1)
  *      This buffer always keeps track of incoming data even when not recording.
  *      When a recording starts data from this buffer can be used as "prebuffer" to get stored on the SD
  *      default size 10000 = 1.280.000 samples.
  *      Example: at 281K samplerate this translates to 4.5 seconds of the last incoming sounds.
  *
  * **********************************************************************/
  // **********  TODO LIST
  // ALLOW (HOME/...) LOCATION SETTING for "fake" GPS stamps, read location file from SD at startup.
  // -ALLOW SWAPPING OF THE REC_BUTTON FROM LEFT ->RIGHT (left/right handed people optiom)
  // -flexible triggerlevel for different frequencies at TE (eg. less sensitive in low frequencies and more at high frequencies)
  // -detect missing optional feature (NO GPS, NO DS18B20) and prevent problems due to this.

   // post V1_3
   // added VIN measurement for Teensy 4.1 on pin 22 (A8), needs resistor-dividor to bring test-voltage in 0..3v3 range !!
   // change in Sparkfun library due to false leapyear calculation
   

   // V1_3 CHANGES
   // added prebuffered recording to allow recording/saving of some data just before the RECbutton was pressed
   // improved recording to prevent dataloss during SDwrite moments
   // use filesize pre_allocate for filewrites (SdFAT greiman)
   // GPS mode improved
   // AGC with decay/attack in ms
   // adapted HIPASS setup to compensate the 23-26K SPU peak with different mode
   // added a deepsleep/hibernation mode

   //in V1_2 CHANGED
   // -ALLOW RECORDING OF FILES IN FOLDERS (YYMMDD) done 20210426
   // -LONG FILENAMES UNDER OTHER SD LIBRARY done 20210426
   // -TE REPLAY SPEED and read WAVFILE samplerate 20210410
   // -add direct access to recorded file powerspectrum 20210421
   // -WATERFALL during 1/10 replay  20210410
   // - change LEFT-micropushbutton to be dedicated RECORDING button 20210426
   // extend play_vol with value
   // startup with value-mode for both encoders  //change in MAIN routine change_detector_mode 20210427
   // clear powerspectrum at the start of a replay //cleared at start/end of replay 20210427
   // prevent SD recording/play when NO SD is present (at startup !) LButton in DISPLAYmode  20210427
   // unwanted action: when pressing (LEFTBUTTON) in the SETTINGS menu the SETTINGS close and the graph changes ! 20210429
   // allow changing directories without going to SETTINGS 20210501
   // correct IDX after selecting a directory to be always 1st idx 20210502

#define batversion " v1.3(post)"
#define versiontrack "09111500"
#define versionno 1030  

// used in EEProm storage <1000 is pre-release            
//  1010 is a the 2nd development version update          
//  1020 is a new release. Final testing started 20210515 
//  1030 development version. Final testing started 20210823 

#include "bat_defines.h"

#include <Arduino.h>

// *************************** CODE BLOCKS and EXTERNAL LIBRARIES **************************
#include "bat_debug.h" //DEBUG MACROS, these will turn into empty functions when DEBUG is not activated in bat_defines
#include "bat_vars.h"  //global variables


#ifdef DEEPSLEEP  // library to allow Teensybat to sleep with minimal powerusage and start recording (AUTOREC) at wakeup
#include "bat_hibernate.h"
#endif


#if defined(__IMXRT1062__) //T41 with PSRAM mounted allows a ringbuffer to be used to continously buffer incoming sound
#ifdef USE_PSRAM
#include "bat_ring_queue.h" //adapted ring_buffer library
#endif
#endif

//local source blocks
#include "bateffect_granular.h" //adapted effect_granular library
#include "bat_record_queue.h" //adapted recording_buffer library
#include "bat_audio.h"   //specific calls to SGTL5000 and audio-library
#include "bat_encoder.h" //setup and routines for encoder and micropushbuttons
#include "bat_timers.h"  //time and timers
#include "bat_tft.h"     //screen routines
#include "bat_fft.h"     // FFT routines and variables
#include "bat_menu.h"    //menu structures
#include "bat_EEPROM.h"  //routines to read/write EEprom

#include "bat_gps.h"

//#include "bat_lipo.h"

#include "bat_sd.h" //routines to read/write SD
#ifdef USE_SDCONFIG
#include "bat_config.h"
#endif

// ******************** BUFFER FOR TIMEEXPANSION ROUTINE ********************
#if defined(__MK66FX1M0__)
#define GRANULAR_MEMORY_SIZE 16 * 1024               // enough for 25 ms at 281kHz sampling
DMAMEM int16_t granularMemory[GRANULAR_MEMORY_SIZE]; //2 bytes per position !!
#endif

#if defined(__IMXRT1062__) //for T41 allow the bufffer in PSRAM 
#define GRANULAR_MEMORY_SIZE 16 * 1024 // enough for 25 ms at 281kHz sampling
#ifdef USE_PSRAM
EXTMEM int16_t granularMemory[GRANULAR_MEMORY_SIZE]; //2 bytes per position !!
#else
DMAMEM int16_t granularMemory[GRANULAR_MEMORY_SIZE]; //2 bytes per position !!
#endif
#endif

/**************************************      FUNCTIONS      ******************************************/

/**************************************  MAIN SCREEN UPDATE ******************************************/

void update_screen(uint8_t from, uint8_t update_type)
  {
  //update_type
  /*0=all
    1=change from encoder RIGHT rotation
    2=change from encoder LEFT rotation
  */
  //D_PRINT("UPDATESCREEN");

  //D_PRINTXY(from, update_type);

  bool update_header = true;          //top header with default information on volume gain etc.
  bool update_frequencygraph = true;  //the small graph below the header
  bool update_encoder_line = true;    //the encoders
  bool update_pushbutton_line = true; //the pushbuttons

  if ((display_mode == settings_page) or (recorderActive) or (AUTO_REC)) //we are in settings, no need to update headers etc
    {
    update_encoder_line = false; // no default update of the encoder/pushbuttons
    update_pushbutton_line = false;
    update_header = false;
    update_frequencygraph = false;
    }

  if ((update_type == 1) or (update_type == 2)) //change coming from RIGHT or LEFT encoder_rotation
    {
    //update_pushbutton_line=false; //dont update pushbuttons
    if ((EncRight_function == enc_menu) and (update_type == 1)) //update from RIGHT encoder but its in menu_mode so no header
      {
      update_header = false;
      }
    if ((EncLeft_function == enc_menu) and (update_type == 2)) //update from LEFT encoder but its in menu_mode so no header
      {
      update_header = false;
      }
    }

  //start screenupdates
  //select default color and font
  tft.setTextColor(ENC_MENU_COLOR);
  tft.setFont(MENU_FONT);

  if (update_header)
    //***************** HEADER  ********************/
    {
    showHeader();
    }

  if (not recorderActive)
    {
    updateTime(); //will only update every 10 seconds
    }

  if (update_frequencygraph)
    { // ******************* GRAPHS
// clear the area above the live graphs (spectrum/waterfall)
// show a scale with ticks for every 10kHz except for no_graph or settings_page
    if (((display_mode == spectrumgraph) or (display_mode == h_waterfallgraph) or (display_mode == waterfallgraph)) and (AUTO_REC == false))
      {
      drawScale();
      drawScaleMarker(); //position the dot on the scale
      }
    showNOSD(); //show that NO SD is mounted
    }

  // *********************** SETTINGS PAGE ****************************
  if (display_mode == settings_page) // display user settings
    {
    showSettings();
    showSettingsButtons();
    }

  //BOTTOM PART OF SCREEN
  /****************** SHOW ENCODER/BUTTON SETTING ***********************/
  if (update_encoder_line)
    {
    showEncoders();
    }

  if (update_pushbutton_line)
    {
    showPushButtons();
    }

  //inform user that settings were SAVED to EEProm (and SDCONFIG)
  if (showEESaved)
    {
    showSaved();
    showEESaved = false;
    }
  }

//* SET THE DISPLAY MODE (no_graph, spectrum, waterfall and on T41 horizontal_waterfall)

void setDisplayMode(uint8_t display_mode)
  {
  D_PRINTXY("DISPLAY_MODE:", setDisplay[display_mode]);
  if (display_mode == no_graph)
    {
    tft.setScroll(0);
    tft.setRotation(0);
    }
#if defined(__IMXRT1062__)  
  if (display_mode == h_waterfallgraph)
    {
    tft.setScroll(0);
    tft.setRotation(0);
    }
#endif
  if ((display_mode == waterfallgraph))
    {
    tft.setRotation(0);
    }
  if (display_mode == spectrumgraph)
    {
    tft.setScroll(0);
    tft.setRotation(0);
    }
  tft.fillScreen(COLOR_BLACK);
  }

//  ********************************************* MAIN MODE CHANGE ROUTINE *************************
void changeDetector_mode(int new_mode)
  {
  detector_mode = new_mode;
  D_PRINTXY("CHANGE DETECTOR:", DT[detector_mode]);
  granular1.stop(); //always stop TE detecting routines

  //restore default positions if we are not in REC or PLAY
  if ((LeftButton_Mode != MODE_PLAY))
    {
    defaultMenuPosition();
    }

  if (detector_mode == detector_heterodyne) //switchting to heterodyne
    {
    osc_frequency = last_osc_frequency; //set the frequency to the last stored frequency
    set_freq_Oscillator(osc_frequency); //set SINE
    set_OutputMixer(heterodynemixer);   //connect output to heterodyne
    sine_HT.amplitude(sine_amplitude);  //sine ON
    }

  if (detector_mode == detector_Auto_heterodyne)
    {
    set_OutputMixer(heterodynemixer);  //connect output to heterodyne
    sine_HT.amplitude(sine_amplitude); //sine ON
    }
  if (detector_mode == detector_Auto_TE)
    {
    granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE); //setup the memory for timeexpansion

  #ifdef USE_TEFACTOR
    if (detune_factor == 100)
      {
      set_OutputMixer(granularmixer);
      }
    else //connect output to granularHT
      {
      set_OutputMixer(granularHTmixer);
      }

  #else
    set_OutputMixer(granularmixer); //connect output to granular
  #endif

    granular1.setTESpeed(TE_speed); // set TE speed
    sine_HT.amplitude(0);           //shutdown HETERODYNE SINE
    sine_HT.frequency(0);
    }

  if (detector_mode == detector_divider)
    {
    granular1.beginDivider(GRANULAR_MEMORY_SIZE);
    set_OutputMixer(granularmixer);
    granular1.setdivider(FD_divider); //V112 changes to effect_granular have been made !!!
    sine_HT.amplitude(0);             //shutdown HETERODYNE SINE
    sine_HT.frequency(0);
    }

  if (detector_mode == detector_passive)
    {
    set_OutputMixer(passivemixer);
    sine_HT.amplitude(0); //shutdown HETERODYNE SINE
    sine_HT.frequency(0);
    }

  D_PRINTLN(" CHANGE DETECTOR DONE");

  }

/******************************************  SD FUNCTIONS ******************************/

// ****************************************  SD STARTS RECORDING
void startRecording()
  {

  D_PRINTLN_F(D_BOLDGREEN, "START RECORDER");


  nj = 0; //counter for recording blocks
  //CHANGE 20210712_1100
  //switch from buffer to recorder

#if defined(__IMXRT1062__)
#ifdef USE_PSRAM
  ringBuffer.stop();
  recorder.begin(); //start the recording to SD
#endif
#endif

  startREC(); //sets file ready creates new filename

  last_gain = mic_gain;
  last_AGC_mode = AGC_mode;

#ifdef USE_PSRAM
  boolean AREC_SRswitch = false;
#endif

  if (AUTO_REC)
    {
     D_PRINTLN_F("AUTOREC", AUTO_REC);
       
 //   if (oper_SR != Arec_SR)
      {

      D_PRINTLN_F("LAST_SR", last_SR);
      D_PRINTLN_F("OPER_SR", oper_SR);
      D_PRINTLN_F("AREC_SR", Arec_SR);

      last_SR = oper_SR; //keep track to allow switching back
      oper_SR = Arec_SR;

      set_SR(oper_SR);
    #ifdef USE_PSRAM
      AREC_SRswitch = true;
    #endif
      }
   
    mic_gain = AREC_G; //set to preset recording gain
    set_mic(mic_gain);
    AGC_mode = AREC_AGC_MODE;

    } //switch to recording samplerate

  active_recording_SR=oper_SR;
  
  //setup the recording screen
  if (AGC_mode == 1)
    {
    AGC_low = mic_gain; //start with current gain
    AGC_high = mic_gain;
    }

  //EXPERIMENTAL TEST to NOT switch DETECTOR off during recording
#if defined(__MK66FX1M0__)
#ifndef USE_FULLMODE_ON_REC
  granular1.stop();                     //stop granular
  outputMixer.gain(granularHTmixer, 0); //shutdown granular with HT output
  outputMixer.gain(granularmixer, 0);   //shutdown granular output
#endif
#endif

  if (!AUTO_REC) //during AUTODETECT several startRecording calls
    {
    last_detector_mode = detector_mode; // save last used detectormode
    last_osc_frequency = osc_frequency;
    last_display_mode = display_mode;
    }

#if defined(__MK66FX1M0__)
#ifndef USE_FULLMODE_ON_REC
  changeDetector_mode(record_detector); // can only listen to A-HT or heterodyne when recording
#endif
#endif

  //empty cumsumPowerspectrum
  for (uint8_t i = 0; i < 120; i++)
    {
    cumsumPowerspectrum[i] = 0;
    }

  // WITHOUT A RINGBUFFER WAIT WITH STARTING A RECORDING UNTIL THE FILE HAS BEEN CREATED
#if defined(__IMXRT1062__)
#ifndef USE_PSRAM
  recorder.begin(); //start the recording to SD
#endif
#endif

  
  startRecordScreen(); //write to TFT after start of the recording for T41 the maximize prebuffer. For T36 this should be viceversa
  drawScale();
  drawScaleMarker(); //position the dot on the scale

#if defined(__MK66FX1M0__) //start T36 only after all screenwrites have been done
  recorder.begin();
#endif

#if defined(__IMXRT1062__)
#ifdef USE_PSRAM
  if (!AREC_SRswitch) //when AREC switches (1st recording) to AREC recording SR the prebuffer is possibly recorded with a wrong SR
    {
    flushRing(&ringBuffer); //write prebuffer
    }
#endif
#endif
  }

// ******************************************************** SD STOPS RECORDING
void stopRecording()
  {
  D_PRINTLN_F(D_BOLDGREEN, "STOPRECORDING FUNCTION");

  //if PSRAM is used stop the recorder and switch to the ringbuffer
  recorder.end();

#if defined(__IMXRT1062__)
#ifdef USE_PSRAM
  ringBuffer.start(); //start ringbuffer directly after closing the recorder
#endif
#endif

  recorderActive = false;
  tft.fillScreen(COLOR_BLACK); //clear the screen
  //restore the detector

  stopREC(); //end the recording by empyting the buffer ##MOVED 20220415 
  
  if (!AUTO_REC) // during AUTOREC gain and samplerate should remain unchanged after recording
    {
    set_SR(oper_SR); //switch back to operational samplerate
    mic_gain = last_gain;
    set_mic(last_gain);

    D_PRINTLN_F(D_BOLDGREEN, "RESTORE MODES AFTER REC");
    osc_frequency = last_osc_frequency;
    changeDetector_mode(last_detector_mode);
    display_mode = last_display_mode;
    update_screen(2, 0);
    }

  //added !!20210425
  if (!AUTO_REC) // only update under "normal manual recording "
    {
    if (memcmp(savedir, active_dir, 8) == 0) //we are saving to the active playdir
      {
      D_PRINTLN("save to active, update filelist");

      D_PRINTXY(savedir, active_dir)
        countFilesinDir_byindex(dirindex[active_folder]);
      }
    }
  }
// ******************************************  END RECORDING *************************

// **************** ******************************SD STARTS PLAYING ************************************************
FLASHMEM
void startPlaying(int SRate)
  {
  //      String NAME = "Bat_"+String(file_number)+".raw";
  //      char fi[15];
  //      NAME.toCharArray(fi, sizeof(NAME));
  D_PRINTLN("START PLAYING");
  sgtl5000.volume(0);
  last_HI_pass = HI_pass;
  D_PRINTXY("last HIPASS", HI_pass);
  HI_pass = HI_PASS_OFF;
  setHiPass();

  fileselect = cyclic_constrain(fileselect, 0, 0, filecounter - 1); //filecounter starts at zero
  D_PRINTXY("FILESELECT ", fileselect);
  D_PRINTXY("INDEX ", fileindex[fileselect]);

  readFileInfo_byindex(fileindex[fileselect]);

  //get filename and shortfilename
  knownSR = 0;
  recorded_SR = 0;

  snprintf(longfilename, 80, "%s/%s", active_dir, filename);

  //read the samplerate of the shortfile WAV file
  if ((String(filename).endsWith(".WAV")) or (String(filename).endsWith(".wav")))
    {
    D_PRINTXY("longfilename ", longfilename);

    File audioFile = SD.open(longfilename, FILE_READ);
    byte audiobuffer[28];
    audioFile.seek(0);
    audioFile.read(audiobuffer, 28);
    audioFile.close();

    D_PRINTXY(" ", filename);
    recorded_SR = audiobuffer[24] + audiobuffer[25] * 256 + audiobuffer[26] * 256 * 256;
    D_PRINTXY("recordedSR ", recorded_SR);
    //check if the SR is part of our system
    for (int i = 0; i < SR_MAX + 1; i++)
      {
      if (SR[i].osc_frequency == recorded_SR)
        {
        D_PRINTXY("known SR ", SR[i].txt);
        knownSR = i;
        }
      }
    }

  playActive = true;
  //allow settling
  delay(100);

  if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY)
    {
    D_PRINTXY("PLAY SR ", SRate);

    if (recorded_SR > MAX_REPLAY_SR) //emperical max replayspeed for DIRECT
      {
      if (SRate == 1)
        {
        SRate = MAX_play_SR - 1;
        }
      }

    if (SRate < MAX_play_SR) //check if we play TE-style or DIRECT
      {
      SRate = constrain(play_SR, 5, MAX_play_SR);
      }
    else
      {            //playDirect
      SRate = 1; //samplerate equal to recording
      }

    set_SR_play(recorded_SR / SRate);
    }
  continousPlay = false;
  if (SRate > 1)
    {
    set_OutputMixer(passivemixer); //no processing
    playDirect = false;
    }
  else //user select to play direct (as if data comes in through the microphone)
    {
    D_PRINT("PLAY DIRECT ");
    changeDetector_mode(detector_mode);
    continousPlay = true;
    playDirect = true;
    }

  powerspectrumCounter = 0;
  for (uint8_t i = 0; i < 240; i++)
    {
    FFTpowerspectrum[i] = 0;
    }

  player.stop();

  player.play(longfilename);
  rec_len = player.lengthMillis();
  delay(30);
  sgtl5000.volume(float(volume * 0.01));
  D_PRINTXY("Play STARTED ", longfilename);
  }

// ********************************************************** SD STOPS PLAYING ***************************************
void stopPlaying()
  {
  sgtl5000.volume(0);
  player.stop();

  D_PRINTLN("STOP PLAYING");
  playActive = false;
  update_screen(3, 0);
  // //restore last SR setting when leaving the PLAY menu

  HI_pass = last_HI_pass;
  D_PRINTXY("HIPASS AFTER PLAY", HI_pass);
  setHiPass();

  if (continousPlay == false) //reset powerspectrum when only playing once
    {
    powerspectrumCounter = 0;
    for (uint8_t i = 0; i < 240; i++)
      {
      FFTpowerspectrum[i] = 0;
      }
    }

  if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY) and (EncLeft_function == enc_menu))
    {
    D_PRINTLN("SWITCH BACK TO DETECTOR");
    set_SR(oper_SR);
    osc_frequency = last_osc_frequency;
    //restore heterodyne frequency
    set_freq_Oscillator(osc_frequency);
    if ((detector_mode == detector_heterodyne) or (detector_mode == detector_Auto_heterodyne))
      {
      sine_HT.amplitude(sine_amplitude); //sine ON
      }
    }
  sgtl5000.volume(float(volume * 0.01));
  }

// ***********************************************SD CONTINUES PLAYING
void continuePlaying()
  {
  //the end of file was reached
  if (playActive)
    { //show position
    uint32_t rec_pos = player.positionMillis();
    float rec_pos_tft = float(rec_pos * 1.0f / rec_len * 1.0f) * ILI9341_TFTWIDTH;
    tft.drawFastHLine(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 17, uint8_t(rec_pos_tft), COLOR_YELLOW);
    tft.drawFastHLine(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 18, uint8_t(rec_pos_tft), COLOR_YELLOW);

    if (!player.isPlaying()) //not playing
      {
      stopPlaying();     //finished so stop
      if (continousPlay) //keep playing until stopped by the user
        {                  //startPlaying(play_SR);
        D_PRINTLN(" CONTINUE");
        HI_pass = HI_PASS_OFF;
        setHiPass();
        playActive = true;
        showPushButtons();
        showEncoders();
        D_PRINTXY("CONTINUE-FILE ", longfilename);
        player.play(longfilename);
        }
      }
    }
  }

//CALLBACK ROUTINE FOR SD to get proper DATETIME stamps
void dateTime(uint16_t* date, uint16_t* time)
  {
  *date = FAT_DATE(year(now()), month(now()), day(now()));
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(now()), minute(now()), second(now()));
  }


// routine to make a screencopy of the current TFT screen on SD as a BMP
FLASHMEM
void dumpScreenToSD(void)
  {
  /*
 *   - Analyze TFT Display, collect size details
 *   - For DEBUG,
 *     - Format file ID and find next available name
 *   - Draw file name to display (relates screen dump
 *     to created file)
 *   - Open File
 *   - Format file records and write to SD file.
 *   - Don't forget to close the file.
 */
 //-------------Local Declarations
  char dumpfile[80] = "SCREENDUMP.BMP";

  unsigned char bmpfileheader[14] =
    {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
  //  uint8_t bmpinfoheader[40] =
  //      {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};

#define BMP_SIGNATURE_WORD 0x4D42

  //-----xBITMAPFILEHEADER
  // kept for reference
  //  struct xBITMAPFILEHEADER
  //  { 
  //    uint16_t  bfType;       // signature - 'BM'
  //    uint32_t  bfSize;       // file size in bytes
  //    uint16_t  bfReserved1;  // 0
  //    uint16_t  bfReserved2;  // 0
  //    uint32_t  bfOffBits;    // offset to bitmap
  //  };

  //  union fh_data { 
  //    struct xBITMAPFILEHEADER fh;
  //    uint8_t fhData[sizeof(xBITMAPFILEHEADER)];
  //  } fhFrame;

  //  union fhKludge { 
  //    uint32_t bfSize;
  //    uint8_t kludge[4];
  //  } kludgeFrame;
  //-----xBITMAPFILEHEADER--end of reference

  //---------------xBITMAPINFOHEADER
  struct xBITMAPINFOHEADER
    {
    uint32_t biSize;         // size of this struct
    int32_t biWidth;         // bmap width in pixels
    int32_t biHeight;        // bmap height in pixels
    uint16_t biPlanes;       // numplanes - always 1
    uint16_t biBitCount;     // bits per pixel
    uint32_t biCompression;  // compression flag
    uint32_t biSizeImage;    // image size in bytes
    int32_t biXPelsPerMeter; // horz resolution
    int32_t biYPelsPerMeter; // vert resolution
    uint32_t biClrUsed;      // 0 -> color table size
    uint32_t biClrImportant; // important color count
    };

  union ih_data
    {
    struct xBITMAPINFOHEADER ih;
    uint8_t ihData[sizeof(xBITMAPINFOHEADER)];
    } ihFrame;

  const uint16_t width = 240;
  const uint16_t height = 320;
  const uint16_t linesize = 3 * width;
  uint8_t linebuf[linesize];
  //const uint32_t filesize = 54 + 3 * width * height;

  uint8_t r, g, b;

  File bmpFile;

  // note - 56 is to make the file size mod 4 (adds 2 bytes for pad)

  // try pixel array for readRect()
  uint16_t pixelColorArray[width + 3];

  // Clear the line buffer, linebuf is a fixed array length.
  memset(linebuf, 0, linesize);
  // Format File Name - determine if unique (so we can open)
  // Form: TFTDMP99.BMP then
  // Open File

  seconds2time(getRTC_TSR());
  snprintf(dumpfile, 80, "%04d%02d%02dT%02d%02d%02d.BMP", tm_year + 1970, tm_mon, tm_mday, tm_hour, tm_min, tm_sec);

  //boolean lastSDFAT=SDFATactive;
  //switch to SDFAT
  //switch2SDfat();

  if (!SD.exists(dumpfile))
    {
    // ony open a new file if it does not exist
    SdFile::dateTimeCallback(dateTime);
    bmpFile = SD.open(dumpfile, FILE_WRITE);
    }

  if (!bmpFile)
    {
    D_PRINTXY("Could not create file: ", dumpfile);
    // if (lastSDFAT!=true)
    //      {switch2fatfs();
    //      }

    return;
    }

  //********FORMAT FILE HEADER AND WRITE TO SD***********
  // fixed fileheader based on 320*240 pixels
  bmpfileheader[2] = 0x36; //(filesize);
  bmpfileheader[3] = 0x84; //(filesize>>8);
  bmpfileheader[4] = 0x03; //(filesize>>16);
  bmpfileheader[5] = 0x00; //(filesize>>24);
  bmpFile.write(bmpfileheader, sizeof(bmpfileheader));
#ifdef DEBUG
  Serial.println(bmpfileheader[2], HEX);
  Serial.println(bmpfileheader[3], HEX);
  Serial.println(bmpfileheader[4], HEX);
  Serial.println(bmpfileheader[5], HEX);
#endif
  // Here we put the write of the file header
  // in linebuf to SD
  // bmpFile.write(linebuf, 14);
  //memset(linebuf, 0, 16); // Zero the used portion of the linebuf

  //*******FORMAT INFO HEADER AND WRITE TO SD*******

  ihFrame.ih.biSize = sizeof(xBITMAPINFOHEADER);
  ihFrame.ih.biWidth = width;
  ihFrame.ih.biHeight = height;
  ihFrame.ih.biPlanes = (uint16_t)1;
  ihFrame.ih.biBitCount = (uint16_t)24;
  ihFrame.ih.biCompression = (uint32_t)0;
  ihFrame.ih.biSizeImage = width * height;
  ihFrame.ih.biXPelsPerMeter = (uint32_t)0;
  ihFrame.ih.biYPelsPerMeter = (uint32_t)0;
  ihFrame.ih.biClrUsed = (uint32_t)0;
  ihFrame.ih.biClrImportant = (uint32_t)0;

  // Copy the INFO Buffer to the linebuf
  for (uint8_t i = 0; i < 40; i++)
    {
    //bitmap info header is 40 bytes
    linebuf[i] = ihFrame.ihData[i];
    }

  //here we put the write of the info header
  bmpFile.write(linebuf, 40);

  //************SCREEN DUMP TO SD FILE ***************
  // Pixel dump to file here
  delay(10);
  // Build BMP file records from display & write to file

  boolean USERECT = false;
#if defined(__IMXRT1062__) //using readrect seems to work fine for T41
  USERECT = true;
#endif

  uint16_t pixCol;
  for (int16_t i = height - 1; i >= 0; i--)
    {
    // Here we write the pixels to the SD.
    // We fetch the pixel, trnslate to rgb, stuff the
    // line buf with the colors swapped to b,g,r order
    // as we walk the linebuf.

    // fetch whole line - rectangle height 1 pixel
    if (USERECT)
      {
      tft.readRect(0, i, width, 1, pixelColorArray);
      }
    // do stuff & write a line from bottom of screen up to top
    // as per normal BMP order

    for (int16_t j = 0; j < width - 1; j++)
      {
      //Fetch pixel colors & stuff BGR color order to linebuf
      //Fetch pixel colors & stuff BGR color order to linebuf
      //tft.color565toRGB(pixelColorArray[j],r,g,b);
      if (!USERECT)
        {
        pixCol = tft.readPixel(j, i); //slow method, fast method gives misaligned colours
        }
      else
        {
        pixCol = pixelColorArray[j];
        }

      tft.color565toRGB(pixCol, r, g, b);
      linebuf[j * 3] = b;
      linebuf[j * 3 + 1] = g;
      linebuf[j * 3 + 2] = r;
      }
    // write the linebuf
    bmpFile.write(linebuf, linesize);
    }

  // All records should be written now.
  // Close File
  bmpFile.close();

  // if (lastSDFAT!=true) //when entering Screendump we were using fatFS so switch back
  //        {switch2fatfs();
  //        }

  // Housekeeping
#ifdef DEBUG
  D_PRINTLN("DUMP File ready ");
#endif
  }

// **************** General graph and detector selective functions ******************************************************

// ************* updateUltrasoundSpectra is the core processing section to trigger TE and show incoming sounds on the TFT *********

void updateUltrasoundSpectra(boolean newFFT)
  {

  const uint16_t Y_OFFSET = TOP_OFFSET;
  static int count = TOP_OFFSET;

  if (newFFT) //ONLY if new FFTdata is available updateUltrasoundSpectra will be activated
    {
    tft.setCursor(0, 90);
    //get FFT data and check for ultrasound

    updateFFTanalysis(); //on T4.1 takes 5-10 microseconds, after changes 3 microseconds

    /************************** Time Expansion MAIN DETECTION **************************/

    /*************************  SIGNAL DETECTION ***********************/
    //signal detected in the Ultrasound range

    // takes on average <1microsecond
    if (sample_UltraSound) //ultrasound in the sample !
      {                      //delayMicroseconds(1);
    #ifdef USE_TEFACTOR
      if (detune_factor != 100)
        {
        DETUNE = int(FFT_peakF_bin * (SR_FFTratio) / TE_speed);
        uint8_t detune = constrain(99 - detune_factor + 50, 50, 99);
        //D_PRINT("DETUNE FFT");

        //D_PRINTLN(DETUNE)
        freq_Oscillator = (DETUNE) * (AUDIO_SAMPLE_RATE_EXACT / SR_real) * detune / 100.0f;
        //AudioNoInterrupts();
        sine_detune.frequency(freq_Oscillator);
        //AudioInterrupts();
        }
    #endif

      // if the previous sample was not Ultrasound we detected a new signal
      if (!lastSample_UltraSound)
        {
        time_since_StartDetection = 0;
        pulse_peakPower = current_peakPower;
        found_peakdrop = false;

        //restart the TimeExpansion only if replay of a previous call has ended
        if ((detector_mode == detector_Auto_TE) and (TE_ReplayEnded))
          {
          //safe routine could be more efficient probably !!
          granular1.stop();
          granular1.beginTimeExpansion(GRANULAR_MEMORY_SIZE);
          granular1.setTESpeed(TE_speed);
          TE_ReplayEnded = false;
          }
        } /* previously NO ultrasound detected */

        // lastSample_UltraSound=sample_UltraSound;
      if ((detector_mode == detector_Auto_heterodyne)) //keep Auto_HT on track
        {
        osc_frequency = int((FFT_peakF_bin * (SR_FFTratio) / 500.0f)) * 500.0f - 1000.0f; //round to nearest 500hz and shift 1000hz upwards to make the peak signal audible
        osc_frequency = constrain(osc_frequency, 7000, int(SR_real / 2000.0f) * 1000.0f - 1000.0f);
        set_freq_Oscillator(osc_frequency * AHT_factor / 100.0f); //AHT_factor allows auto_HT to go by default up to 20% lower than the peak
        sine_HT.amplitude(sine_amplitude);                        //sine ON
        last_osc_frequency = osc_frequency;
        //update screen
        if (not(recorderActive) & (display_mode != no_graph))
          {
          if (FFT_count % 100 == 0)
            {
            drawScale();
            drawScaleMarker();
            }
          }
        }
      }
    else // NO ultrasound detected in current sample !
      {    //previous sample was Ultrasound
      if (lastSample_UltraSound)
        { //callLength=time_since_StartDetection;
  //CLsum+=callLength;  // store the time since the start of the call  NOT USED !!
  //D_PRINTXY("CL",callLength);
        time_since_EndDetection = 0;
        showStart = false; //start timing endof call
        }
      // lastSample_UltraSound=sample_UltraSound; // set the end of the call
      }

    lastSample_UltraSound = sample_UltraSound;

    // restart TimeExpansion recording after at least TE_GAP milliseconds of time has passed since initial detection. This is to shortcut long calls
    // for instance a batcall click is often a few ms long. The replay takes call*TE_spd times longer so the longest call we want to hear fully
    // should be considered the best setting ? Eg. pipistrelle with 5-6ms of call using a 20x slowdown would require a TE_GAP of 100-120ms to hear the full
    // call.

    if ((!TE_ReplayEnded) and (time_since_StartDetection > TE_GAP))
      {
      TE_ReplayEnded = true;
      lastSample_UltraSound = false; // even if we have captured UltraSound in the current(and thus last)sample mark it als false to allow trigger of TE
      granular1.stopTimeExpansion();
      time_since_EndDetection = 0;
      showStart = false; //we ended the detection
      }

    // SWITCH displaymode takes <1 microsecond
    switch (display_mode)
      {
      case spectrumgraph:
        {
        if (fftSpectrumAvailable)
          {
          spectrum();
          fftSpectrumAvailable = false;
          }
        // update power-spectrumdisplay after every xth FFT sample with bat-activity
        if ((calls_detected > 10))
          //if ((powerspectrumCounter>powerspectrumMaxcounts)  )
          {
          if (!recorderActive) //on T41 recording can use TFT
            {
            showPowerSpectrum();
            drawScale();
            drawScaleMarker(); //position the dot on the scale
            }
          calls_detected = 0;
          }
        break;
        }

        // update power-spectrumdisplay after every xth FFT sample with bat-activity for both waterfall options
      case waterfallgraph:
      case h_waterfallgraph:

        //after either 10 seperate calls or 500 incoming spectra update the powerspectrum
        if ((calls_detected > 10) or (powerspectrumCounter > powerspectrumMaxcounts))
          {
          if (!recorderActive) //on T41 and T36 we can now use TFT
            {
            showPowerSpectrum();
            drawScale();
            drawScaleMarker(); //position the dot on the scale
            }
          calls_detected = 0;
          }
        break;
      }

    // when NO spectrum is availble this takes <1microsecond
    // when a spectrum is available this takes 50-200 microseconds
    if ((display_mode == waterfallgraph) or (display_mode == h_waterfallgraph))
      {
      // T41: EXCLUSIVE for a horizontal scroll the display of the framebuffer must be updated separately
    #ifdef SCROLL_HORIZONTAL  //ONLY POSSIBLE FOR THE T41
      if ((display_mode == h_waterfallgraph) and (updateFramebuffer))
        {
        if (time_since_EndDetection < 20)
          {
          for (uint8_t i = 0; i < (ILI9341_TFTWIDTH - BOTTOMSCALE); i++)
            {
            tft.writeRect(i, TOP_OFFSET, 1, SPECTRUM_HEIGHT, (uint16_t*)&frameBuffer[i * ILI9341_TFTWIDTH + 32]); //only use part of the spectrum to stay in the reserved space
            }
          updateFramebuffer = false; //signal that an update has been done
          }
        }
    #endif  

      if (fftSpectrumAvailable) // a new spectrum is available
        {

        if (display_mode == h_waterfallgraph)
          {
          //only on T4.1, slower and software based
        #ifdef SCROLL_HORIZONTAL
          updateFramebuffer = true; //new data was entered, a screen update will be possible
          //use a 240*250 buffer to store the data
          //shift all data one line to the left
          memmove(&frameBuffer[0], &frameBuffer[ILI9341_TFTWIDTH], ILI9341_TFTWIDTH * (240 - BOTTOMSCALE) * 2);
          //add the new FFT_pixels at the end
          memcpy(&frameBuffer[ILI9341_TFTWIDTH * (240 - BOTTOMSCALE)], FFT_pixels, ILI9341_TFTWIDTH * 2); //write pixels to active bufferline
        #endif
          }
        else //normal faster (hardware based) vertical scroll 
          {
          tft.writeRect(0, count, ILI9341_TFTWIDTH, 1, (uint16_t*)&FFT_pixels);
          tft.setScroll(count);
          }

        count++;
        fftSpectrumAvailable = false;
        }
      //keep track of the current waterfallline that was scrolled  
      if (display_mode == waterfallgraph)
        {
        if (count >= ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - BOTTOMSCALE)
          {
          count = Y_OFFSET;
          }
        }
      }

    //automatic recording of signals if we are NOT recording allready
    if (recorderActive == false)
      //if autorecording is on and a signal was found and the last recording ended more than AREC_P* 5seconds ago
      if ((AUTO_REC) and (sample_UltraSound) and (recording_stopped > (AREC_P * AREC_5SEC)))
        {
        D_PRINTLN("AutoREC");
        //start the recorder
        recorderActive = true;
        autocounter++;
        startRecording();
        recording_running = 0;
        time_since_EndDetection = 0;
        showStart = false;
        display_mode = no_graph;
        }
    }
  }

//*****************************************************update encoders
//read the Encoders and update the states
void updateEncoder(uint8_t Encoderside)
  {
  D_PRINT("Update Enc ");

  /************************setup vars*************************/
  int encodermode = -1; // menu=0 value =1;
  int change = 0;
  int menu_idx = 0;
  int choices = 0;

  //get encoderdata depending on the selected encoder 
  //******LEFT
  if (Encoderside == enc_leftside)
    {
    encodermode = EncLeft_function;
    change = EncLeftchange;
    menu_idx = EncLeft_menu_idx;
    menu_id = LeftBaseMenu[menu_idx].menu_id;
    choices = LeftMenuOptions; //available menu options
    D_PRINT(" LEFT ");

    if (encodermode == enc_menu)
      {
      D_PRINT("MENU ");

      }
    else
      {
      D_PRINT("VALUE ");

      }
    D_PRINT(menu_idx)
      D_PRINT(" ");

    D_PRINTLN(menu_id)
    }

  //******RIGHT
  if (Encoderside == enc_rightside)
    {
    encodermode = EncRight_function;
    change = EncRightchange;
    menu_idx = EncRight_menu_idx;

    if ((LeftBaseMenu[EncLeft_menu_idx].menu_id != MENU_SETTINGS)) //regular functionality left Encoder not on SETTINGS
      {
      menu_id = RightBaseMenu[menu_idx].menu_id;
      }
    else
      {
      if (EncLeft_function != enc_value) //not inside the settings menu but left Encoder is on SETTINGS
        {
        menu_id = RightBaseMenu[menu_idx].menu_id;
        }
      else //active inside settings menu
        {    //NOP
        }
      }
    choices = RightMenuOptions; //available menu options
    D_PRINT(" RIGHT ");

    if (encodermode == enc_menu)
      {
      D_PRINT("MENU ");

      }
    else
      {
      D_PRINT("VALUE ");

      }
    D_PRINT(menu_idx)
      D_PRINT(" ");


    D_PRINTLN(menu_id)
    }

  /************************react to changes from the encoder*************************/

  //encoder is in menumode
  if (encodermode == enc_menu)
    {
    menu_idx = menu_idx + change;

    //allow revolving choices limited to a specific encoder (left or right !!)
    menu_idx = cyclic_constrain(menu_idx, 0, 0, choices - 1);

    if (Encoderside == enc_leftside)
      {
      EncLeft_menu_idx = menu_idx; //limit the menu
      menu_id = LeftBaseMenu[menu_idx].menu_id;
      D_PRINT(" MENU ");

      D_PRINTLN(LeftBaseMenu[menu_idx].menu_txt)
      }

    //limit the changes of the rightside encoder for specific functions
    if (Encoderside == enc_rightside)
      {
      EncRight_menu_idx = menu_idx; //limit the menu
      menu_id = RightBaseMenu[menu_idx].menu_id;
      D_PRINT(" MENU ");

      D_PRINTLN(LeftBaseMenu[menu_idx].menu_txt)
      }
    }

  //encoder is in valuemode and has changed position so change an active setting
  if ((encodermode == enc_value) and (change != 0))
    { //changes have to be based on the choosen menu position
/******************************VOLUME  ***************/
    if (menu_id != MENU_SETTINGS)
      {
      menuAction(menu_id, change); //act on changes when not in settings menu
      if (menu_id == MENU_DISPLAY)
        {
        setDisplayMode(display_mode);
        }
      }
    /******************************SETTINGS MENU  LEFTSIDE ENCODER INDEX ***************************/

    // for the settings menu the left encoder selects the option and the right encoder allows changes the values
    if ((menu_id == MENU_SETTINGS) and (Encoderside == enc_leftside))
      {
      uint8_t idx = settings_page_nr;
      set_menu_pos[idx] += change;
      set_menu_pos[idx] = cyclic_constrain(set_menu_pos[idx], 0, 0, settings_MenuOptions[idx] - 1);

      if (idx == 0)
        {
        set_menu_id[idx] = Settings0Menu[set_menu_pos[idx]].menu_id;
        }
      if (idx == 1)
        {
        set_menu_id[idx] = Settings1Menu[set_menu_pos[idx]].menu_id;
        }
      if (idx == 2)
        {
        set_menu_id[idx] = Settings2Menu[set_menu_pos[idx]].menu_id;
        }
      if (idx == 3)
        {
        set_menu_id[idx] = Settings3Menu[set_menu_pos[idx]].menu_id;
        }
    #ifdef SHOW_SDMENU
      if (idx == setpage_SDCARD)
        {
        set_menu_id[idx] = SettingsSDCARDMenu[set_menu_pos[idx]].menu_id;
        }
    #endif
    #ifdef USE_GPS
      if (idx == setpage_GPS)
        {
        set_menu_id[idx] = SettingsGPSMenu[set_menu_pos[idx]].menu_id;
        }
    #endif

      D_PRINT("SETTINGS PAGE ");

      if (idx < page_titles)
        {
        D_PRINT(settings_page_name[idx]);
        }
      else
        {
        D_PRINTXY(" nr:", idx);
        }
      D_PRINT(" MENU- ID ");

      D_PRINTLN(set_menu_id[idx]);
      }

    /******************************SETTINGS MENU RIGHTSIDE ENCODER VALUES ***************************/

    // change the settings on the SETTINGSPAGE when turning the right encoder
    if ((menu_id == MENU_SETTINGS) and (Encoderside == enc_rightside))
      {
      //change 20210326
      if (set_menu_id[settings_page_nr] == SET_MENU_PAGE)
        {
        show_next_settings_page(change);
        D_PRINT(" SWITCH SETTINGS TO PAGE ");

        D_PRINTLN(settings_page_nr)
        }

      //change settings for the active page and item
      else
        {
        settingsMenuAction(set_menu_id[settings_page_nr], change);
        }

      //**************************** SPECIAL MENU ACTIONS  **********************************/
      // these are actions that often need other parts of the code and do not only depend on bat_menu //

      // if (set_menu_id[settings_page_nr]==SET_MENU_SD_PLAYFOLDER) //update filecounter when changing playfolder
      //      {showSettings();
      //      }

      if (set_menu_id[settings_page_nr] == SET_MENU_TIME) //we are on the time menusection
        {
        updateTimeMenu(change);
        }
      //set date
      if (set_menu_id[settings_page_nr] == SET_MENU_DATE)
        {
        updateDateMenu(change);
        }

      /* COLOR MENU */
      if (set_menu_id[settings_page_nr] == SET_MENU_COLORSCHEME) //or (set_menu_id[settings_page_nr]==SET_MENU_colorscheme_gamma_preset))
        {
        for (uint8_t i = 0; i < 240; i++)
          {
          tft.drawFastVLine(i, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - 8, 8, colourmap[i * 4]);
          }
        }

      if ((set_menu_id[settings_page_nr] >= SET_MENU_COLORS) and (set_menu_id[settings_page_nr] <= SET_MENU_BCKCOLORS))
        {
        cwheelpos[set_menu_id[settings_page_nr] - SET_MENU_COLORS] += change * 4;
        uint8_t cwheel = cyclic_constrain(cwheelpos[set_menu_id[settings_page_nr] - SET_MENU_COLORS], 0, 0, cwheelmax);
        cwheelpos[set_menu_id[settings_page_nr] - SET_MENU_COLORS] = cwheel;
        if (set_menu_id[settings_page_nr] == SET_MENU_COLORS)
          {
          ENC_MENU_COLOR = rainbow(cwheel, 2);
          if (cwheel == 0)
            {
            ENC_MENU_COLOR = COLOR_YELLOW; //defaults
            }
          }
        if (set_menu_id[settings_page_nr] == SET_MENU_HICOLORS)
          {
          HILIGHT_MENU_COLOR = rainbow(cwheel, 2);
          if (cwheel == 0)
            {
            HILIGHT_MENU_COLOR = COLOR_WHITE; //defaults
            }
          }
        if (set_menu_id[settings_page_nr] == SET_MENU_BCKCOLORS)
          {
          BCK_MENU_COLOR = rainbow(cwheel, 1);
          if (cwheel == 0)
            {
            BCK_MENU_COLOR = COLOR_DARKRED; //defaults
            }
          }
        }
      }
    //******************************END SETTINGS MENU  RIGHTSIDE ENCODER  ***************************/

    /******************************SELECT A FILE  ***************/
    if (((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY)) and (EncLeft_function == enc_value)) //menu selected file to be played
      {
      if (!playActive)
        {
        fileselect = cyclic_constrain(fileselect, EncLeftchange, 0, filecounter - 1);
        D_PRINTXY("fileselect:", fileselect);
        }
      }
    /***************************** CHANGE THE VOLUME DURING REPLAY */
    if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY) and (playActive))
      {                         //allow volume to be changed from the LEFT encode during replay
      if (EncLeftchange != 0) //change the volume when replaying
        {
        volume += EncLeftchange;
        volume = constrain(volume, 0, 90); //not cyclic
        set_vol(volume);
        }
      }
    /****************************** CHANGE PLAY SR   ***************/
    if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY) and (RightBaseMenu[EncRight_menu_idx].menu_id == MENU_SR) and (EncRight_function == enc_value)) //menu play selected on the left and right
      {
      if ((LeftButton_Mode == MODE_PLAY))
        {
        if (EncRightchange != 0)
          {
          if ((recorded_SR == 0) and (!playActive)) //file was not read yet !
            {
            readFileInfo_byindex(fileindex[fileselect]);
            }

          play_SR = cyclic_constrain(play_SR, EncRightchange, 5, MAX_play_SR);
          int SRate = play_SR;

          if (play_SR >= MAX_play_SR)
            {
            SRate = 1;
            }

          if (playActive)
            {
            stopPlaying();
            }

          set_SR_play(recorded_SR / SRate);
          }
        }
      }
    }
  }
// **************************  UPDATE ENCODERS *******************************************/
// update all encoder data
void update_Encoders()
  {
  getEncoderchanges();
  if ((EncLeftchange != 0))
    {
    updateEncoder(enc_leftside);
    }
  if ((EncRightchange != 0))
    {
    updateEncoder(enc_rightside);
    }

  //update display only if a change has happened to at least one encoder
  if ((EncRightchange != 0) or (EncLeftchange != 0))
    {
    if (EncRightchange != 0)
      {
      update_screen(4, 1);
      }
    else
      {
      update_screen(4, 2);
      }
    }
  }
// ******************************************************************************  BUTTONS

// ************************************** get button states ************************************/

void update_Buttons()
  {
  // try to make the interrupts as short as possible when recording
  if (AUTO_REC) //during autorecording
    {
    micropushButton_L.update();          //ONLY check the left micro button
    if (micropushButton_L.fallingEdge()) //************* left pushbutton was pressed so end current recording
      {
      tft.sleep(false);
    #ifdef USE_PWMTFT
      set_backlight(tft_backlight);
    #endif
      D_PRINTLN_F(D_BOLDGREEN, "MANUAL STOP AUTOREC FUNCTION");

      AUTO_REC = false; //stop auto_recording
     
      if (recorderActive) //manual stop during a recording
        {
        D_PRINTLN(" STOP ACTIVE AUTO_REC");
        stopRecording();
        }
      //stop recording and bring back the previous detector_mode and display_mode
      else //detector is in autorec mode but was not recording so in between two recordings stopped
        {
        D_PRINTLN(" STOP PAUSE AUTO_REC");
        stopRecording();
        }

      delay(100); //wait a bit
    
      oper_SR = last_SR;
      mic_gain = last_gain;     //restore gain
      AGC_mode = last_AGC_mode; //set AGC back
      display_mode = last_display_mode;
      recorderActive = false;
      update_screen(5, 0);
      }
    }

  //recorder is active but not in AUTOREC mode, only check the left micro button
  if (!AUTO_REC)
    {
    if (recorderActive) // RECORDING MODE so do minimal checks !! if AUTOREC was on the check has allready been done
      {
      micropushButton_L.update();            //ONLY check the left encoderbutton
      if ((micropushButton_L.fallingEdge())) //end current recording
        {
        D_PRINTLN_F(D_BOLDGREEN, "MANUAL STOP RECORDING");
        stopRecording();
        delay(100); //wait a bit
        recorderActive = false;
        update_screen(5, 0);
        }
      }
    else // ****************** SYSTEM IS NOT RECORDING and allows full interaction
      {
      updateButtonStatus(); //update all buttons
      //use a timer to check for a possible longpress on left micro button
      if ((ButtonR_state == 0) and (lastButtonR_state == 1)) //button is starting to get pressed
        {
        ButtonR_down = millis();
        }

      if ((ButtonR_state == lastButtonR_state) and (ButtonR_state == 0) and (longpressR == false) and (SD_ACTIVE)) //buttonR is still pressed
        {
        if (!playActive)
          {
          if ((millis() - ButtonR_down) > 2000)
            {
            D_PRINTXY("LONGPRESS", millis() - ButtonR_down);
            longpressR = true;
            tft.setCursor(4, -5);
            //tft.fillRect(0,0,ILI9341_TFTWIDTH/4,TFT_FONT.cap_height+2,BCK_MENU_COLOR);
            tft.setTextColor(COLOR_WHITE);
            tft.print("*");
            dumpScreenToSD(); //make a screendump as BMP
            delay(100);
            showHeader();
            }
          }
        }

      lastButtonR_state = ButtonR_state;
      //rightbutton is mainly dedicated to detectormode
      if (micropushButton_R.risingEdge()) //and (LeftButton_Mode!=MODE_PLAY))
        {
        D_PRINTLN("Update PButton R");
        if (display_mode != settings_page) //NOT IN SETTINGS
          {                                  //not in the settings menu so default setting of detectormode
            //allow when not in mode_play or in mode_play_direct

          if ((LeftButton_Mode != MODE_PLAY) or ((LeftButton_Mode == MODE_PLAY) and (play_SR == MAX_play_SR)))
            {
            if (detector_mode == detector_heterodyne)
              {
              last_osc_frequency = osc_frequency; //store osc_frequency when leaving HT mode
              }
            changeDetector_mode(cyclic_constrain(detector_mode, 1, 0, detector_passive));
            update_screen(6, 0);
            }
          }
        else //in the settings menu for time/date the right_micropush can be used to step through hrs/min & yr/month/day
          {
          if (set_menu_id[settings_page_nr] == SET_MENU_TIME)
            { //jump to hours or minutes
            timemenu_pos = cyclic_constrain(timemenu_pos, 1, 0, 1);
            update_screen(6, 0);
            }
          if (set_menu_id[settings_page_nr] == SET_MENU_DATE)
            { //jump to hours or minutes
            timemenu_pos = cyclic_constrain(timemenu_pos, 1, 0, 2);
            update_screen(6, 0);
            }

          }
        }

      //leftbutton function is based on leftbutton_mode)
      if ((micropushButton_L.fallingEdge()))
        {
        D_PRINTLN("Update PButton L");
        D_PRINTXY("LbuttonMODE", LeftButton_Mode)

          if (LeftButton_Mode == MODE_DISPLAY) // NO SD MOUNTED
            {
          #ifdef SCROLL_HORIZONTAL
            display_mode = cyclic_constrain(display_mode, 1, no_graph, h_waterfallgraph);

          #else
            display_mode = cyclic_constrain(display_mode, 1, no_graph, waterfallgraph);
          #endif

            D_PRINTXY("DISPLAY_MODE:", setDisplay[display_mode])
              tft.setRotation(0);
            if (display_mode == no_graph)
              {
              tft.setScroll(0);
              }
            if (display_mode == spectrumgraph)
              {
              tft.setScroll(0);
              }
            if (display_mode == h_waterfallgraph)
              {
              tft.setScroll(0);
              }

            tft.fillScreen(COLOR_BLACK); //blank the screen
            }

        if (LeftButton_Mode == MODE_PLAY)
          {
          if (!root_active)
            {
            if (playActive == false) //button pressed but not playing so start Playing
              {                        //playActive=true;
              startPlaying(play_SR);
              }
            else //button pressed and playing so stop
              {
              stopPlaying();
              playActive = false;
              }
            }
          else //root_active so user is selecting a directory
            {    //change the active dir
            D_PRINT_F(D_BOLDGREEN, "SELECTING DIR ");

            playfolder = fileselect;
            last_active_folder = fileselect;
            countFilesinDir_byindex(dirindex[playfolder]);
            fileselect = 0; //20210502
            dir.open("/");
            file.open(&dir, dirindex[playfolder], O_RDONLY);
            file.getName(active_dir, 80);
            D_PRINTLN(active_dir);
            file.close();
            dir.close();
            dir.open(active_dir);
            active_folder = playfolder;
            root_active = false;
            }
          }

        if ((LeftButton_Mode == MODE_REC) and (display_mode != settings_page))
          {
          if (recorderActive == false) // when recorder is active interaction gets picked up earlier !!
            {
            D_PRINTLN("normal REC");
            recorderActive = true;
            startRecording();
            }
          }

        //no function yet
        update_screen(5, 0);
        }
      else //check if longpress was used
        {
        if ((micropushButton_R.risingEdge()) and (longpressR))
          {
          longpressR = false;
          }
        }

      /************  LEFT ENCODER BUTTON CONFIRMATION *******************/
      if (encoderButton_L.fallingEdge() and (!playActive)) //do not allow pressing the LEncoder when playing files
        {
        D_PRINTLN("Update EButton L");

        EncLeft_function = !EncLeft_function;

        if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_SETTINGS)) //settings_page
          {
          if (display_mode == settings_page) //user pressed the encoderbutton so wants to leave settings mode
            {
            D_PRINTLN_F(D_BOLDGREEN, "LEAVING SETTINGS");


          #ifdef USE_GPS
            I2SON(); // switch ON I2S
          #endif

            display_mode = last_display_mode; //restore previous display_mode
            detector_mode = last_detector_mode;

            tft.fillScreen(COLOR_BLACK);
            //restore right encoder mode
            EncRight_menu_idx = last_RightMenuidx;
            EncRight_function = last_RightMenufunc;
            }
          else //user enters settings mode
            {    //store current modes
            D_PRINTLN_F(D_BOLDGREEN, "ENTERING SETTINGS PAGE #");

            D_PRINTLN(settings_page_nr)

            #ifdef USE_GPS
              I2SOFF(); // switch OFF I2S
          #endif

            last_display_mode = display_mode;
            last_detector_mode = detector_mode;

            last_RightMenuidx = EncRight_menu_idx;
            last_RightMenufunc = EncRight_function;

            display_mode = settings_page; //show the other user-defined settings

            tft.setScroll(0);
            tft.fillRect(0, TOP_OFFSET - 50, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT - TOP_OFFSET + 50 - BOTTOM_UPPERPART, COLOR_BLACK);

            EncLeft_function = enc_value; // option selection
            //start at the top options and zero set_menu_pos array
            memset(set_menu_pos, 0, sizeof(set_menu_pos));

            for (uint8_t i = 0; i < settings_menu_pages; i++)
              {
              set_menu_id[i] = SET_MENU_PAGE; //Settings0Menu[0].menu_id;// RESET SET_MENU_PAGE to TOP;
              }

            EncRight_function = enc_value; //Right encoder will changes values
            }
          }

        /*************************  SD CARD IS AVAILABLE FOR RECORDING ***********************************/
        if ((SD_ACTIVE))
          {
          if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_AUTOREC) and (EncLeft_function == enc_value))
            {
            D_PRINTLN_F(D_BOLDGREEN, "LBUTTON -> REC");

            LeftButton_Mode = MODE_REC; //select the choosen function for the leftbutton
            D_PRINTLN_F(D_BOLDGREEN, "START AUTOREC");

            AUTO_REC = true;
            autocounter = 0;
            last_gain = mic_gain;
            mic_gain = AREC_G; //set to preset gain

            AGC_maxGAIN = AREC_G; //make AGC follow the AREC_G setting

            signal_LoF_bin = int((AREC_F * 1000.0) / (SR_FFTratio));
            last_display_mode = display_mode;
            last_detector_mode = detector_mode;
            display_mode = no_graph;

            update_screen(4, 0);

            tft.fillScreen(COLOR_BLACK);
            if (tft_sleep)
              {
              tft.setFont(Arial_18);
              tft.setCursor(0, 90);

            #ifdef USE_PWMTFT
              tft.println("      Screen OFF");
              tft.println("");
              tft.println(" Lft. Button to restart");
              delay(2000);
              set_backlight(0);
              tft.sleep(true);
            #else
              tft.println(" Screen inactive");
              tft.println("");
              tft.println(" Left Button to restart");
              delay(2000);
              tft.fillRect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, COLOR_BLACK);
            #endif
              }

            D_PRINTLN_F(D_BOLDGREEN, "AUTOREC WAIT");


            //initREC();
            }

          //play menu is active, user is selecting files
          if ((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY) and (EncLeft_function == enc_value)) //choose to select values
            {
            D_PRINTLN_F(D_BOLDGREEN, "LBUTTON -> PLAY");

            LeftButton_Mode = MODE_PLAY;        // directly set LEFTBUTTON to play/stop mode
            last_osc_frequency = osc_frequency; //keep track of heterodyne setting
            fileselect = 0;

            // initPlay(); //switch SDsystem to be ready for playing

            play_SR = constrain(play_SR, 5, MAX_play_SR); //make sure play_SR is within the set boundaries

            int SRate = play_SR;

            if (play_SR >= MAX_play_SR)
              {
              D_PRINTLN("SET DIRECTPLAY");
              SRate = 1;
              }
            else
              {
              D_PRINTLN("SET PLAYRATE ");
              D_PRINTXY("RATE", play_SR);
              D_PRINTLN_F(D_BOLDGREEN, SR[play_SR].txt);
              }
            if (recorded_SR > 0)
              {
              set_SR_play(recorded_SR / SRate); //switch to the play SR
              }
            else
              {
              set_SR_play(SR[play_SR].osc_frequency / SRate);
              }
            //shut down input
            set_InputMixer(in_player); //switch on player as input

            if (play_SR < MAX_play_SR)
              {
              set_OutputMixer(passivemixer); //no processing
              }
            last_RightMenuidx = EncRight_menu_idx;
            last_RightMenufunc = EncRight_function;

            EncRight_menu_idx = MENU_SR;
            EncRight_function = enc_value;
            }

          if ((EncLeft_function == enc_menu) and (LeftButton_Mode == MODE_PLAY)) //user wants to stop playing or step back into selecting a dir
            {
            if (memcmp(active_dir, "/", 1)) //activedir is NOT yet root
              {
              D_PRINTLN_F(D_BOLDGREEN, "ENTERING ROOT DIR (PLAY_MODE)");

              countDirs_inroot(); //update
              root_active = true; //mark that we are in root
              //move indexes from dirindex to fileindex
              filecounter = dircounter;
              fileselect = 0; //set to beginning
              if (last_active_folder < dircounter)
                {
                fileselect = last_active_folder;
                }
              snprintf(active_dir, 80, "/"); //switch to root
              for (uint16_t i = 0; i <= dircounter; i++)
                {
                fileindex[i] = dirindex[i];
                }

              EncLeft_function = enc_value;
              }
            else //active_dir was root so leave PLAY menu
              {
              D_PRINTLN_F(D_BOLDGREEN, "LEAVING PLAY MODE");

              root_active = false;
              active_folder = last_active_folder;
              countFilesinDir_byindex(dirindex[active_folder]);
              snprintf(active_dir, 80, dirname);
              D_PRINTXY("ACTIVE DIR", active_dir)
                tft.fillRect(0, TOP_OFFSET - 50, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT - TOP_OFFSET + 50 - BOTTOM_UPPERPART, COLOR_BLACK);

              EncRight_menu_idx = last_RightMenuidx;  //THIERY
              EncRight_function = last_RightMenufunc; //THIERY
              }
            }

          if (EncLeft_function == enc_menu) //user wants to go back to the menu, restart the detector
            {
            //restore operational SR setting and last_osc for HT
            osc_frequency = last_osc_frequency;
            set_SR(oper_SR);
            D_PRINTXY("DETECTORMODE ", DT[detector_mode])
              if ((detector_mode == detector_heterodyne) or (detector_mode == detector_Auto_heterodyne))
                {
                set_OutputMixer(heterodynemixer);
                }
              else
              #ifdef USE_TEFACTOR
                {
                D_PRINTXY("DETUNE ", detune_factor)
                  if (detune_factor == 100)
                    {
                    set_OutputMixer(granularmixer);
                    sine_detune.amplitude(0);
                    }
                  else
                    {
                    set_OutputMixer(granularHTmixer);
                    sine_detune.amplitude(sine_amplitude);
                    }
                }
          #else
            {
            set_OutputMixer(granularmixer);
            }
          #endif

            set_InputMixer(in_mic); //switch on the mic_input
            set_mic(mic_gain);
            powerspectrum_Max = 0;
            }

          //automatically change LEFTbutton back to displaymode if it was on play or record previously
          if ((EncLeft_function == enc_menu) and ((LeftButton_Mode == MODE_PLAY) or (LeftButton_Mode == MODE_REC)))
            {
            D_PRINTLN_F(D_BOLDGREEN, "LBUTTON -> SET TO REC");

            if (SD_ACTIVE)
              {
              LeftButton_Mode = MODE_REC;
              }
            set_SR(oper_SR);
            }

          } //END SD_ACTIVE

        update_screen(4, 0);
        } //END encoder Left risingedge

        /************  RIGHT ENCODER BUTTON *******************/

      if (encoderButton_R.risingEdge())
        {
        D_PRINTLN("Update EButton R");

        if ((display_mode == settings_page))
          {
          EEPROM_SAVE();
        #ifdef USE_SDCONFIG
          writeConfiguration();
        #endif
          showEESaved = true;
          //showSaved();
          }
        else
          {
          EncRight_function = !EncRight_function; //switch between menu/value control
          }

        update_screen(4, 0);
        }

      } // ************** END NORMAL BUTTON PROCESSING

    } //end IF !AUTOREC
  }
// **************************  END BUTTONS

//###########################################################################
//##########################   ROUTINES FOR DEEPSLEEP #######################
//###########################################################################

// ******** show a countdown on TFT/SERIAL before starting the SLEEP
void deepSleepCountdown()
  {
  uint8_t Yp = tft.getCursorY();
  uint8_t countdown = 5;
  while (countdown > 0)
    {
    countdown--;
    tft.setCursor(0, Yp);
    tft.fillRect(0, Yp, ILI9341_TFTWIDTH, 20, COLOR_DARKRED);
    tft.print("seconds left ");
    D_PRINTXY("DEEPSLEEP IN ", countdown);
    tft.println(countdown);
    if (countdown == 0)
      {
      D_TIME();
      }
    delay(1000);
    }
  D_TIME();
  D_PRINTLN("ACTIVEDEEPSLEEP START");
  }

// ********** powerdown SD, SGTL, I2S, TFT
#ifdef USE_PWMTFT
void powerOff_beforeSleep()
  {
  D_PRINTLN("POWERDOWN for DEEPSLEEP");
  dir.close();
  file.close();
  root.close();
  set_backlight(0); //TFT OFF
  tft.sleep(true);
  sgtl5000.powerOff();
  I2SOFF();
  delay(50);
  }
#endif

// *********** MONITOR VIN 
#ifdef VIN_ON
uint16_t VIN_NOW;

#if defined(__MK66FX1M0__)
uint16_t VIN_ADC()
  {
  mv1 = 9999;
  analogReference(DEFAULT);
  analogReadResolution(12); //12 bits so 0-4095 range readouts
  analogReadAveraging(32); //32 is ax

  mv1 = uint16_t(1195.0f * 4096.0f / analogRead(71));
  }
#endif

#if defined(__IMXRT1062__)
uint16_t VIN_ADC()
  {
  // uint8_t ch = pin_to_channel[VIN_ON];  // convert Arduino style channel number to Teensy internal numbering A8 =13
  uint8_t ch = 13;
  if (!(ch & 0x80))
    {
    ADC2_HC0 = ch;                       // set ADC channel number
    while (!(ADC2_HS & ADC_HS_COCO0));  // wait until ADC result ready
    mv1 = ADC2_R0;                      // return new ADC result
    }
  else
    mv1 = 0;

  return mv1;
  }
#endif

#endif


//###########################################################################
//###########################################################################
//##########################   MAIN STARTUP ROUTINE  ########################
//###########################################################################
//###########################################################################



void setup()
  {

  //start with compiler-error 
#if (TEENSYDUINO < 154)
#error "This version of TeensyBat was ONLY tested with Teensyduino 1.54"
#endif

#if (F_CPU<120'000'000)
#error " CPU speeds less than 120 Mhz are NOT supported " 
#endif

  // now start setup
  boolean reset_toDefault = false;

  //ACTIVATE NVRAM STORAGE FOR SPECIAL DATA THAT CAN BE RETAINED AND IS DIRECTLY AVAILABLE AT STARTUP
#if defined(__IMXRT1062__)
  SNVS_LPCR |= (1 << 24); //Enable NVRAM - documented in SDK
#ifdef DEEPSLEEP
  rtc_clearAlarm(); //clear the ALARM on the T41
#endif
#endif

  //ALWAYS CHECK IF A RESET IS REQUESTED at startup 
  pinMode(MICROPUSH_LEFT, INPUT_PULLUP); //activate button
  delay(40); //allow to settle
  micropushButton_L.update();
  delay(5);

  if (micropushButton_L.read() == 0) //user wants to interrupt the sleep @ wakeup
    {
    reset_toDefault = true;
    AREC_DEEPSLEEP_SLEEP = NVRAM_DATA[1]; //store
    NVRAM_DATA[1] = 0; //reset NVRAM SLEEP  and WAKEUP
    AREC_DEEPSLEEP_WAKEUP = NVRAM_DATA[2];
    NVRAM_DATA[2] = 0;
    AREC_DEEPSLEEP = false;
    AUTO_REC = false;
    }

  if (!reset_toDefault) //user has not tried to interrupt the wakeup-sequence
    {
  #ifdef DEEPSLEEP 
    setSyncProvider(getTeensy3Time);
    //get the current time
    seconds2time(getRTC_TSR());
    current_minutes = tm_hour * 60 + tm_min;
    //if both NVRAM1 and NVRAM2 are NOT 0 this means we are using a sleep sequence
    if ((NVRAM_DATA[1] != 0) & (NVRAM_DATA[2] != 0))
      {
      //test if we are inside the sleep window or not
      //initial valid test is on a sleepwindow completely during the day eg. from 7h to 20h.
      //2nd valid test is where the wakeup time is BEFORE the sleep time during the day.
      // in this scenario sleeptime is ment for the night. eg. Sleep starts at 23h and the system has to wake up at 6h.
      // this 2nd mode is used to protect LIPO batteries from completely draining
      // daysleep is the "normal mode"

      //example1 NVRAM1= 5h NVRAM2=22h
      //example2 NVRAM1= 22h NVRAM2=5h
      boolean daysleep = NVRAM_DATA[1] < NVRAM_DATA[2];
      //example1 TRUE
      //example2 FALSE 

      boolean keepsleeping = false;
      if (daysleep) //EXAMPLE1
        { //at 0 hr current_minutes is < NVRAM1 so the system will not sleep
          //at 12 hr current_minutes is > NVRAM1 and <NVRAM2 so the system will sleep
        if ((current_minutes > NVRAM_DATA[1]) & (current_minutes < NVRAM_DATA[2]))
          {
          keepsleeping = true;
          }
        }
      else // sleep passes the 0hr boundary EXAMPLE2 NVRAM1 22 NVRAM2 5 so sleep from 22h to 5h
        {//reverse the condition test so this sets awake time on the window (SWAP TESTS ON NVRAM)
        keepsleeping = true;
        if ((current_minutes > NVRAM_DATA[2]) & (current_minutes < NVRAM_DATA[1]))
          {
          keepsleeping = false;
          }
        }

      if (keepsleeping)
        { //currently between start and end of the sleep so keep sleeping
      #ifdef DEEPSLEEP_LED //allow a visisble BLIP on the deepsleep LED
        pinMode(DEEPSLEEP_LED, OUTPUT);
        digitalWriteFast(DEEPSLEEP_LED, HIGH);
        delay(10);
        digitalWriteFast(DEEPSLEEP_LED, LOW);
      #endif

        setWakeupCallandSleep(DEEPSLEEP_TIMER); //sleep further
        }
      else // WAKEUP !!!
        { //start by storing NVRAM[1] and [2]
        AREC_DEEPSLEEP_SLEEP = NVRAM_DATA[1]; //store
        AREC_DEEPSLEEP_WAKEUP = NVRAM_DATA[2];
        NVRAM_DATA[1] = 0;
        NVRAM_DATA[2] = 0;
        //set autorecord and arec_deepsleep so the system will record and also be read to deepsleep again
        AUTO_REC = true; //go into AUTORECORDING MODE
        AREC_DEEPSLEEP = true; // keep DEEPSLEEP active timeslots will be SET for the next sleep
        }
      }
  #endif
    }

#ifdef DEBUG
  SERIALINIT;
  D_PRINTLN_F(D_BOLDGREEN, "********** START DEBUGGER ************");
  for (int ii = 0; ii < NVRAM_words; ii++)
    {
    Serial.print("NVRAM ");
    Serial.print(ii);
    Serial.print(":");
    Serial.println(NVRAM_DATA[ii]);
    }
#endif
  
  D_PRINTLN_F(D_BOLDGREEN, "COMPILER INFORMATION");

  D_PRINTXY("ARDUINO ", ARDUINO);
  D_PRINTXY("C++ ", __cplusplus);
  D_PRINTXY("CLIBCXX ", __GLIBCXX__);
  D_PRINTXY("Version ", __VERSION__);
  D_PRINTXY("F_CPU ", F_CPU);
  
#ifdef VIN_ON
  D_PRINTXY("VIN", VIN_ADC());

#endif

  D_PRINTLN();
  D_PRINTLN_F(D_BOLDGREEN, "MEMORY ");
  D_PRINTXY("STARTUP FREERAM", freeram());
  D_PRINTXY("SAMPLE_BUFFER", BUFFSIZE);
  D_PRINTXY("GRANULAR_BUFFER", GRANULAR_MEMORY_SIZE * 2);
#ifdef DEBUG
  uint32_t samplemem = AUDIOMEM_samples;
  samplemem = samplemem * 256;
  D_PRINTXY("AUDIO_MEMORY", samplemem);
  D_PRINTXY("TOTAL USED RAM", BUFFSIZE + GRANULAR_MEMORY_SIZE * 2 + samplemem);
#endif


#if defined(__IMXRT1062__)
  D_PRINTLN("TEENSY 4.1");

#ifdef USE_PSRAM
  if (PSRAMsize > 0)
    {
    D_PRINTLN_F(D_BOLDGREEN, "USING PSRAM");
    D_PRINTXY("-- Size (MB)", PSRAMsize);
    PSRAM_ON = true;
    //switch to 132Mhz PSRAM frequency
    CCM_CCGR7 |= CCM_CCGR7_FLEXSPI2(CCM_CCGR_OFF);
    CCM_CBCMR = (CCM_CBCMR & ~(CCM_CBCMR_FLEXSPI2_PODF_MASK | CCM_CBCMR_FLEXSPI2_CLK_SEL_MASK)) | CCM_CBCMR_FLEXSPI2_PODF(4) | CCM_CBCMR_FLEXSPI2_CLK_SEL(2); // 528/5 = 132 MHz
    CCM_CCGR7 |= CCM_CCGR7_FLEXSPI2(CCM_CCGR_ON);
    }
  else
    {
    D_PRINTLN_F(D_BOLDRED, "PSRAM WRONG CONFIG");
    D_PRINTLN("-- NO PSRAM FOUND ");
    PSRAM_ON = false;
    }

#endif
#endif

  //setup Encoder and pushButtons with pullups to allow restart
  ButtonsEncoders_pullup();
  D_PRINTLN("ENCODER PULLUPS DONE");

  EncLeft_menu_idx = BaseMenu[0].menu_id;  //default 1st option
  EncRight_menu_idx = BaseMenu[1].menu_id; //default 2nd option

  //initialize AUDIOBOARD processing

  sgtl5000.enable();
  D_PRINTLN("AUDIOBOARD enabled");

  sgtl5000.muteHeadphone(); //shut down headphones
  sgtl5000.volume(0);

  sgtl5000.lineInLevel(0); //shutdown line-in as a default

  sgtl5000.inputSelect(myInput);
  sgtl5000.micGainnew(mic_gain);

  //sgtl5000.adcHighPassFilterDisable(); // does not help too much!
  //hipass OFF
  HiPass_biquad.setCoefficients(0, allpass);
  HiPass_biquad.setCoefficients(1, allpass);
  HiPass_biquad.setCoefficients(2, allpass);
  HiPass_biquad.setCoefficients(3, allpass);

  tft.begin();
  D_PRINTLN("TFT active");

#ifdef USE_PWMTFT
  set_backlight(tft_backlight);
  if (AUTO_REC) //turn OFF backlight when startup in AUTOREC
    {
    tft_sleep = true;
    D_PRINTLN("AUTOREC ON AFTER SLEEPING, TFT SHOULD BE OFF ");
    set_backlight(0);
    }
#endif

  initTFT(); //blank startscreen
  //stop startup if PSRAM was expected but not found

#ifdef USE_PSRAM
  if (PSRAM_ON == false)
    {
    D_PRINTLN_F(D_BOLDRED, "PSRAM WRONG CONFIG");
    tft.setCursor(0, 0);
    tft.setFont(Arial_18_Bold);
    tft.println("    PSRAM");
    tft.println("    NOT FOUND");
    tft.println("");
    tft.print("!!    STOPPED    !!");
    //endless LOOP
    while (true)
      {
      }
    }

#endif

  D_PRINTXY("SETUP AUDIOMEM FREERAM", freeram());
  D_PRINTXY("AUDIOMEM sampleblocks", AUDIOMEM_samples);
  AudioMemory(AUDIOMEM_samples);

  D_PRINTXY("POST  AUDIOMEM FREERAM", freeram());
  //set time provider
  setSyncProvider(getTeensy3Time);
  FsDateTime::setCallback(dateTime);
  NVRAM_DATA[0] = getRTC_TSR(); //last startup
   //test if LEFT MICROPUSH is pressed, if so restart without reading the EEPROM settings (all defaults)
  // micropushButton_L.update();
  // boolean reset_toDefault = false;
  // if (micropushButton_L.read() == 0)
  //   {
  //   reset_toDefault = true;
  //   }

  //if RIGHT MICROPUSH is pressed, if so restart by tring to read the config from SD
#ifdef USE_SDCONFIG
  micropushButton_R.update();
  boolean reset_toSDconfig = false;
  if (micropushButton_R.read() == 0)
    {
    reset_toSDconfig = true;
    }
#endif

  //test if user has requested to reset to defaults (left-pushbutton down during startup)
  if (reset_toDefault)
    {
    showresettoDefault(0);
    NVRAM_DATA[1] = 0;
    NVRAM_DATA[2] = 0;
    AREC_DEEPSLEEP = false;
    AUTO_REC = false;
    }

  SD_ACTIVE = SD.begin(BUILTIN_SDCARD);

  /* RESET TO DEFAULTS from SD config*/
#ifdef USE_SDCONFIG
  /* check if a config file is present */
  if ((SD_ACTIVE) and (reset_toSDconfig))
    {
    if (readConfiguration())
      {
      D_PRINTLN_F(D_BOLDGREEN, "GOT CONFIG FROM SD");
      showresettoDefault(1);
      }
    delay(2000);
    }
#endif

  /* EEPROM CHECK  */
  if (reset_toDefault)
    {
    delay(2000);
    EEPROM_SAVE();
    D_PRINTLN_F(D_BOLDRED, "EEPROM RESET");

    delay(100);
    }

  if (EEPROM_LOAD() == false) //load data fromEEprom, if it returns false (probably due to a change in structure) than start by trying to read config data"
    {
  #ifdef USE_SDCONFIG
    if (SD_ACTIVE)
      {
      if (readConfiguration())
        {
        D_PRINTLN_F(D_BOLDGREEN, "GOT CONFIG FROM SD");

        }
      }
  #endif

    EEPROM_SAVE();
    D_PRINTLN_F(D_BOLDRED, "EEPROM RESET");

    }
  D_PRINTLN_F(D_BOLDGREEN, "EEPROM LOADED");



  StartupScreenimage();
  StartupScreen();


  //initSD();

  if (!SD.begin(BUILTIN_SDCARD))
    {
    SD_ACTIVE = false;
    D_PRINTLN_F(D_BOLDRED, "NO SDCARD FOUND");

    LeftButton_Mode = MODE_DISPLAY; //force into play
    }
  else
    {
    SD_ACTIVE = true;
    D_PRINTLN_F(D_BOLDGREEN, "SDCARD FOUND");

    D_PRINTLN("SDFAT");
    D_PRINTLN_F(D_BOLDGREEN, SD_FAT_VERSION_STR);
    SD_SIZE = SD.totalSize();
    SD_USED = SD.usedSize();
    D_PRINTXY("CARD USED", SD_USED / (1024 * 1024));
    D_PRINTXY("CARD TOTAL", SD_SIZE / (1024 * 1024));

    countDirs_inroot();

    //find highest YYYYMMDD directory and select that
    uint32_t hidate = 1; // prevent on a empty card or card with non-numerical dirs
    dir.open("/");
    boolean hidateFound = false;
    D_PRINTLN("check directories");

    for (uint8_t i = 0; i <= dircounter; i++)
      {
      file.open(&dir, dirindex[i], O_RDONLY);
      file.getName(dirname, 80); //save selected directory filename
      file.close();
    #ifdef DEBUG_DETAIL
      D_PRINTXY("found ", dirname);
    #endif
      if (uint32_t(atoi(dirname)) > hidate)
        {
        hidate = atoi(dirname);
        snprintf(active_dir, 80, dirname);
        playfolder = i;
        active_folder = i; //added !!20210425
        hidateFound = true;
        }
      }

    if (hidateFound)
      {
      D_PRINTXY("selected", active_dir);
      countFilesinDir_byindex(dirindex[playfolder]);
      }
    }

#ifdef USE_GPS
  I2SOFF(); //shutdown I2S to minimize disturbance during GPS acquisition

  delay(50);

  tft.setCursor(0, 255);
  tft.fillRect(0, 250, 240, 60, ILI9341_BLACK);

  tft.setFont(MENU_FONT);
  tft.println("Trying to connect to GPS");
  tft.println("Keep LftBtn pressed to skip GPS");

  initGPS(); //setup a serial connection to the GPS and settings

  tft.setCursor(0, 255);
  tft.fillRect(0, 250, 240, 60, ILI9341_BLACK);

  if (GPSbaudOK)
    {
    tft.println("press LftBtn to skip");
    tft.setCursor(0, 275);
    tft.println("GPS connected");

    boolean nofix = true;
    byte countr = 0;
    //wait until we have a fix
    while (nofix)
      {
      countr++;

      micropushButton_L.update(); //ONLY check the left micro button
      if (micropushButton_L.fallingEdge())
        {
        countr = 15;
        }

      if (countr < 15)
        {
        delay(300);
        if (readGPS())
          {
          if ((gps_fix == 2) or (gps_fix == 3)) //only for 2d/3d
            {
            nofix = false;
            GPSfix_time = now();
            }

          tft.setCursor(0, 300);
          tft.fillRect(0, 300, 120, 20, ILI9341_BLACK);
          tft.printf("Sats %02d", gps_SIV);
          }
        }

      tft.setCursor(200, 270);
      tft.fillRect(200, 270, 40, 20, ILI9341_BLACK);
      tft.printf("%02d", countr);
      if (countr >= 15) //after 15 tries move on
        {
        break;
        }
      }
    //combi1
    //myGNSS.powerSaveMode(false);
    //myGNSS.setPowerMode(2);
    //combi1a
    //myGNSS.powerSaveMode(false);
    //myGNSS.cyclicTrack1s();
    //combination 2

    myGNSS.powerSaveMode(false);
    myGNSS.setPowerMode(3);
    //combi3
    // myGNSS.OnOff120s();
    // myGNSS.setMeasurementRate(30000);
    // myGNSS.powerSaveMode(true);
    myGNSS.saveConfiguration();
    }
  else
    {
    tft.printf("GPS NOT connected");
    delay(1000);
    }

  I2SON();

#else
  delay(2000);
#endif

  tft.setFont(MENU_FONT);

#ifdef USE_DS18B20
  sensors.begin();
#ifdef DEBUG
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  Serial.print("Parasite power is: ");

  if (sensors.isParasitePowerMode())
    Serial.println("ON");
  else
    Serial.println("OFF");
#endif

  if (!sensors.getAddress(batThermo, 0))
    {
    D_PRINTLN("Unable to find address for Device 0");
    return;
    }
  active_batThermo = true;
  sensors.setResolution(batThermo, 12);
  bat_tempC = sensors.getTempC(batThermo);
  if (bat_tempC == DEVICE_DISCONNECTED_C)
    {
    D_PRINTLN("Error: Could not read temperature data");
    return;
    }

  sensors.requestTemperatures();
  sensors.setWaitForConversion(true);
  //wait for the conversion to arrive
  int resolution = 12;
  delay(750 / (1 << (12 - resolution)));
  bat_tempC = sensors.getTempCByIndex(0);
  D_PRINTXY("Temp C: ", bat_tempC);

#endif

#ifdef USE_LIPO
  testLipo();
#endif



  /******* BUILD MENUSTRUCTURES ***************************************************/
  build_menu_structures();

  // ***************** SETUP AUDIO *******************************
  D_PRINTLN("***SETUP AUDIO***");


  set_SR(oper_SR); //set operational sample rate

  set_InputMixer(in_mic); //microphone active
  set_OutputMixer(heterodynemixer);

  D_PRINTLN("***SETUP GRANULAR***");

  // the Granular effect requires memory to operate
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

  //switch to the preset or default detector_mode
  changeDetector_mode(detector_mode);

  sgtl5000.volume(float(volume * 0.01));
  sgtl5000.unmuteHeadphone();

  // update all encoders/buttons status values before starting
  // the initial state of buttons is therefore not triggering actions at startup
  updateButtonStatus();
  //finalize by showing a startupscreen

  //clear the screen gently
  for (uint16_t i = ILI9341_TFTHEIGHT; i > 0; i--)
    {
    delay(1);
    tft.drawFastHLine(0, i, ILI9341_TFTWIDTH, COLOR_BLACK);
    }

  update_screen(255, 0);
  seconds2time(getRTC_TSR());
  old_time_sec30 = tm_sec / 30;
  old_time_sec5 = tm_sec / 5;
  old_time_min = tm_min;
#ifdef USE_DS18B20
  showTime();
#endif

#if defined(__IMXRT1062__)
#ifdef USE_PSRAM
  D_PRINT("START RINGBUFFER");

  ringBuffer.begin();
#endif
#endif

  D_PRINTXY("FREERAM END OF SETUP", freeram());
  // set_arm_clock(120000000);
   //while (1);
  pinMode(5, OUTPUT);
#ifdef DEEPSLEEP_LED
  digitalWriteFast(DEEPSLEEP_LED, HIGH);
  delay(1000);
  digitalWriteFast(DEEPSLEEP_LED, LOW);
#endif

  } // END SETUP

  //###########################################################################
  //###########################################################################
  //##########################   LOOP #########################################
  //###########################################################################
  //###########################################################################

  //loop time T4.1 is around 3 microseconds, with looplen=100, loopcheck=90 this becomes 1.7 microseconds with additional changes <1microsecond
  //loop time T3.6 is around 30 microseconds

  //start the processing loop !

uint8_t loopcount = 0;
uint8_t looplen = 100;            //max no of loops
uint8_t loopcheck = looplen - 75; //start of checks on buttons. (1-loopcheck/looplen) is the % of time the loop tests for buttonpresses.

void loop()
  {

#ifdef DEEPSLEEP
  if ((AREC_DEEPSLEEP) and (AUTO_REC)) //DEEPSLEEP is only used for autorecording, start with AUTOREC
    {
    seconds2time(getRTC_TSR());
    current_minutes = tm_hour * 60 + tm_min;
    if ((current_minutes > AREC_DEEPSLEEP_SLEEP) and (current_minutes < AREC_DEEPSLEEP_WAKEUP))
      {
      activeDeepSleep = true; //keep or set the activedeepsleep status 
      deepSleepStartScreen(); //show that we are going to deepsleep
      deepSleepCountdown();

      NVRAM_DATA[0] = getRTC_TSR(); //set current time in seconds NVRAM0
      NVRAM_DATA[1] = AREC_DEEPSLEEP_SLEEP; //also keep track of the wakeup and sleep time
      NVRAM_DATA[2] = AREC_DEEPSLEEP_WAKEUP;
#ifdef USE_PWMTFT
      powerOff_beforeSleep();
#endif      
      setWakeupCallandSleep(DEEPSLEEP_TIMER);
      }

    }   //END AUTOREC BLOCK
#endif

  if (!activeDeepSleep) //test if the system is in NORMAL
    {
    // If we are recording, carry on... and only check the left microbutton
    if (LeftButton_Mode == MODE_REC)
      {
      if (recorderActive)
        {
        writeREC(); //20210817 WAS BEFORE RECORDERACTIVE !!!
        //use FFT to check if incoming signals still are ultrasound
      #ifndef USE_FULLMODE_ON_REC
        if (myFFT.available()) //check if a sample is available
          {
          updateFFTanalysis(); //do the analysis for ultrasound
          if (sample_UltraSound)
            {
            time_since_EndDetection = 0;
            showStart = false;
            if ((!AUTO_REC) and (record_detector == detector_Auto_heterodyne))
              {
              adjust_Heterodyne_Oscillator();
              }
            }
          }
      #else
        updateUltrasoundSpectra(myFFT.available());
      #endif

        //we are autorecording then stop recording after AREC_D* 5seconds or if we have not heard anything for AREC_B seconds
        if ((AUTO_REC) and ((recording_running > (AREC_D * AREC_5SEC) or (time_since_EndDetection > AREC_B * AREC_1SEC))))
          {
          stopRecording();
          recorderActive = false;
          delay(100); //minor delay
          recording_stopped = 0;
          }
        }
      } //END OF MODE_REC

    if (LeftButton_Mode == MODE_PLAY)
      { //D_PRINT('P')
      if (playActive)
        {
        continuePlaying();
        }
      }

    //only check for part of the loops
    if (loopcount > loopcheck)
      {
      update_Buttons(); //0.5microseconds
      }

    // T4.1 next section tool 2 microseconds if no data is presented on screen (after loopcheck <1microsecond)
    if (not recorderActive)
      {
      if (loopcount > loopcheck)
        {
        update_Encoders(); //<0.5microsec
        }

      //always check for available incoming data, wait for it
      if (LeftButton_Mode == MODE_REC)  //default mode
        {
      #ifndef DEEPSLEEP 
        elapsedMicros waitforFFT = 0;
        //on T4.1 waiting time is around 450-500 microseconds
        while (!myFFT.available() and (waitforFFT < 2000)) //wait up to 2ms for incoming samples
          {
          };

        updateUltrasoundSpectra(true);
      #else
        updateUltrasoundSpectra(myFFT.available());
      #endif
        }
      else //in other modes dont wait but just check
        {
        updateUltrasoundSpectra(myFFT.available());
        }
      //<0.5microsec and around 50microsec when screen gets updated
      //update the time on screen regularly

      if (loopcount > loopcheck)
        {
        seconds2time(getRTC_TSR());

      #ifdef USE_GPS
        boolean PVTavailable = false;
      #endif
        if (AUTO_REC) //in autorec state but not recording update every 2 seconds
          {
          if (tm_sec / 2 != old_time_sec2)
            {
            updateAUTORECstatus();
            }
          old_time_sec2 = tm_sec / 2;
          }

        else
          {

          /*************************************************1 SECOND *******************************/
          if (tm_sec != old_time_sec)
            { //every second
    //tft.updateScreenAsync();
            old_time_sec = tm_sec;
            if (AGC_change)
              {
              showEncoders();
              AGC_change = false;

              }

          #ifdef USE_GPS
            if (display_mode == settings_page) //use this every second
              if (settings_page_nr == setpage_GPS)
                {
                if (GPSbaudOK) //only when we have a uart connection
                  {
                  PVTavailable = readGPS();
                  if (PVTavailable) //only update with active PVT
                    {
                    if (gps_fix != gps_lastfix)
                      {
                      showSettings();
                      }
                    }
                  }
                }
          #endif

            /*************************************************30 SECONDS *******************************/
            if ((tm_sec / 30) != old_time_sec30) // when the GPS is ON but not in its datascreen try to update
                                                 // every 30 seconds
              {
              old_time_sec30 = tm_sec / 30;
            #ifdef USE_GPS
              if ((display_mode != settings_page) and (GPSbaudOK))
                {
                D_PRINT("readGPS(ms)");
                //uint32_t start = millis();

                readGPS(); //just read and update and powerdown
                showTime();

                //D_PRINTXY(millis() - start, " DONE");
                }
            #endif
              }
            /*************************************************5 SECONDS *******************************/
            if (tm_sec / 5 != old_time_sec5)
              {
              showTime();

              if (AudioMemoryUsageMax() > MAX_AUDIO_SAMPLES)
                {
                AudioMemorywarning = true;
                showHeader();
                }

              old_time_sec5 = tm_sec / 5;
            #ifdef USE_GPS
              if (display_mode == settings_page) //ACTIVE UPDATING
                {
                if ((settings_page_nr == setpage_GPS) and (GPSbaudOK)) //update when checking the settings page
                  {
                  tft.setFont(MENU_FONT);
                  tft.setCursor(0, 260);
                  tft.fillRect(0, 260, 150, 20, COLOR_BLACK);
                  tft.setTextColor(COLOR_WHITE);
                  tft.print("UPDATE");

                  if (!PVTavailable) // no NAV solution found in previous 5 seconds
                    {
                    readGPS(); //try again
                    }
                  else //make sure the user can see the update remark
                    {
                    delay(100);
                    PVTavailable = false;
                    }

                  if ((gps_fix > 0) and (gps_fix < 6))
                    {
                    tft.printf("fix %1dd", gps_fix);
                    }

                  if (gps_lastfix != gps_fix) // a change in the fix has happend during the last 5 seconds
                    {
                    gps_lastfix = gps_fix;
                    if ((gps_fix == 2) or (gps_fix == 3)) //update time if we have a "real" fix
                      {
                      showTime();
                      }
                    }
                  tft.fillRect(0, 260, 80, 20, COLOR_BLACK);
                  showSettings();
                  }
                }
            #endif

            #ifdef USE_DS18B20
              if (activeTrequest)
                {
                showTime();
                bat_tempC = sensors.getTempCByIndex(0);
                D_PRINTXY("Temp C", bat_tempC);
              #if defined(__IMXRT1062__)
                D_PRINTXY("Temp CPU", tempmonGetTemp());
              #endif
                activeTrequest = false;
                }

            #endif
              }
            }
          }

        /*************************************************1 MINUTE *******************************/
        if (tm_min != old_time_min) //minutes have changed
          {
          D_PRINTXY("UPDATE AUDIOMEM usage", AudioMemoryUsageMax())
          #ifdef USE_DS18B20
            if (active_batThermo)
              {
              D_PRINTLN("GET DS TEMP");

              Serial.printf("%02d:%02d:%02d", tm_hour, tm_min, tm_sec);
              sensors.requestTemperatures();
              sensors.setWaitForConversion(true);
              //int resolution = 12;
              //delay(750/ (1 << (12-resolution)));
              activeTrequest = true;
              }
        #endif


          old_time_min = tm_min;
          }
        /*************************************************10 MINUTE *******************************/
        if (tm_min / 10 != old_time_min10) // minutes have changed
          {
          old_time_min10 = tm_min / 10;

        #ifdef VIN_ON //MONITOR VIN
          VIN_NOW = VIN_ADC();
          if (SD_ACTIVE)//SD mounted
            {
            //append readout to file
            File csvFile = SD.open("VIN_LOGGER.TXT", FILE_WRITE);

            D_PRINTXY("LOGGING VIN", VIN_NOW);

            csvFile.printf("%02d:%02d:%02d,", tm_hour, tm_min, tm_sec);
            csvFile.print(VIN_NOW);
            csvFile.println();
            csvFile.close();

            }
          showHeader();

          if (VIN_NOW < VIN_LOW) // VIN is dropping TOO low for normal functions on SGTL/T36.
            { // startup DEEPSLEEP mode 
          #ifdef DEEPSLEEP
            deepSleepCountdown(); //announce the deepsleep
            seconds2time(getRTC_TSR());
            current_minutes = tm_hour * 60 + tm_min;
            //setup the wakeup in the past (so effectively one day ahead) and go to sleep now 
            AREC_DEEPSLEEP_SLEEP = current_minutes - 1; //this will make the system sleep immediately 
            AREC_DEEPSLEEP_WAKEUP = current_minutes - 3; //wakeup will be tomorrow 3 minutes earlier than now and only for 2 minutes.
            NVRAM_DATA[0] = getRTC_TSR(); //set current time in seconds NVRAM0

            NVRAM_DATA[1] = AREC_DEEPSLEEP_SLEEP; //also keep track of the wakeup and sleep time
            NVRAM_DATA[2] = AREC_DEEPSLEEP_WAKEUP;
#ifdef USE_PWMTFT
            powerOff_beforeSleep();
#endif            

            setWakeupCallandSleep(DEEPSLEEP_TIMER);
          #else
            showHeader(); //warn 
          #endif

            }


        #endif



          }

        /*************************************************1 hour *******************************/
        if (tm_hour != old_time_hour) //hours have changed

          {
          old_time_hour = tm_hour;


          }


        }
      }
    loopcount++;
    if (loopcount > looplen)
      {
      loopcount = 0;
      }


    } //END !ACTIVEDEEPSLEEP


  }
