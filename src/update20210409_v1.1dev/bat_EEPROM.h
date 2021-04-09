/* TEENSYBAT DETECTOR (for TEENSY 3.6) 
 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
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
 */

//*******************************EEPROM *********************************************************
// original routines copied from https://github.com/DD4WH/Teensy-ConvolutionSDR/blob/master/Teensy_Convolution_SDR.ino

#ifndef _BAT_EEPROM_H
#define _BAT_EEPROM_H

#include <EEPROM.h>
#include "bat_vars.h"
#include "bat_audio.h"
#include "bat_encoder.h"

#include <util/crc16.h>

#define EE_CONFIG_VERSION "0017"  //4 char ID of the EE structure, will change with each change in EEprom structure
//0017 added te_detune

#define EE_CONFIG_START 0    
//general variables stored in EEprom

struct config_t {
  uint16_t BatVersion; 
  uint16_t EEsaved_count; //number of times the settings have been saved (for this version!!)
  uint8_t detector_mode; //detectormode at startup
  uint8_t display_mode; //displaymode at startup
  uint8_t rec_detector_mode; //detectormode at recorder
  
  uint8_t SD_FFormat; //files saved as RAW(0) or WAV(1)
  uint8_t ply_SR; //default SampleRate for replay
  uint8_t rec_SR; //default SampleRate during recording
  uint8_t oper_SR; //default SampleRate for detector
  int8_t mic_gain; //default GAIN
  int8_t volume; //default Volume
  int osc_frequency; // for Heterodyne the default oscillator frequency
  int HiPass; //hipass frequency
  uint8_t TE_speed; // for TimeExpansion the slowdownspeed
  uint8_t TE_low;  //for TimeExpansion the lowest frequency to allow triggering
  uint8_t TE_GAP;  //minimum time between TE replays 
  uint8_t TE_peak; //lowest peaksetting to trigger TE
  uint8_t FD_divider; //default divider setting for FD
  uint8_t TE_DETUNE; //detune setting
  uint8_t AREC_D; //automated maximum recording duration in seconds
  uint8_t AREC_P; //automated recording minimal pauze between recordings in seconds
  uint8_t AREC_B; //automated recording break due to no signal for X seconds
  uint8_t AREC_F; //automated recording minimal frequency in kHz
  uint8_t AREC_S; //automated recording minimal signalstrength

  uint8_t use_presets; //0= default values; 1=user values;
  uint8_t Encoder_direction; //1 or -1
  uint8_t Encoder_divider; //steps on the encoder that equal a change. Default =4

  uint8_t COLOR_MENU; //default menucolor
  uint8_t COLOR_HILIGHTMENU; //highlights
  uint8_t COLOR_BACKMENU; //background
  uint8_t tft_backlight; //PWM SETTING
  bool tft_sleep; //PWM SETTING
  
  char version_of_settings[4]; //versionstring to track changes in the EEstructure
  uint16_t crc;   // added when saving
} E;

boolean loadFromEEPROM(struct config_t *ls) {  //mdrhere
#if defined(EEPROM_h)
  char this_version[] = EE_CONFIG_VERSION;
  unsigned char thechar = 0;
  uint8_t thecrc = 0;
  config_t ts, *ts_ptr;  //temp struct and ptr to hold the data
  ts_ptr = &ts;

  // To make sure there are settings, and they are YOURS! Load the settings and do the crc check first
  for (unsigned int t = 0; t < (sizeof(config_t) - 1); t++) {
    thechar = EEPROM.read(EE_CONFIG_START + t);
    *((char*)ts_ptr + t) = thechar;
    thecrc = _crc_ibutton_update(thecrc, thechar);
  }
  if (thecrc == 0) { // have valid data
    //printConfig_t(ts_ptr);
    #ifdef DEBUG_SERIAL
      Serial.printf("Found EEPROM version %s", ts_ptr->version_of_settings);  //line continued after version
    #endif
    if (ts.version_of_settings[3] == this_version[3] &&    // If the latest version
        ts.version_of_settings[2] == this_version[2] &&
        ts.version_of_settings[1] == this_version[1] &&
        ts.version_of_settings[0] == this_version[0] ) {
      for (int i = 0; i < (int)sizeof(config_t); i++) { //copy data to location passed in
        *((unsigned char*)ls + i) = *((unsigned char*)ts_ptr + i);
      }
      #ifdef DEBUG_SERIAL
      Serial.println(", loaded");
      #endif

      return true;
    } else { // settings are old version
      #ifdef DEBUG_SERIAL
      Serial.printf(", not loaded, current version is %s\n", this_version);
      #endif
      return false;
    }
  } else {
    #ifdef DEBUG_SERIAL
    Serial.println("Bad CRC, settings not loaded");
    #endif
    return false;
  }
#else
  return false;
#endif
}

//routine to save data to EEprom but only those bytes that need to be saved, to conserve EEprom
boolean saveInEEPROM(struct config_t *pd) {
#if defined(EEPROM_h)
  int byteswritten = 0;
  uint8_t thecrc = 0;
  boolean errors = false;
  unsigned int t;
  for (t = 0; t < (sizeof(config_t) - 2); t++) { // writes to EEPROM
    thecrc = _crc_ibutton_update(thecrc, *((unsigned char*)pd + t) );
    if ( EEPROM.read(EE_CONFIG_START + t) != *((unsigned char*)pd + t) ) { //only if changed
      EEPROM.write(EE_CONFIG_START + t, *((unsigned char*)pd + t));
      // and verifies the data
      if (EEPROM.read(EE_CONFIG_START + t) != *((unsigned char*)pd + t))
      {
        errors = true; //error writing (or reading) exit
        break;
      } else {
        //Serial.print("EEPROM ");Serial.println(t);
        byteswritten += 1; //for debuggin
      }
    }
  }
  EEPROM.write(EE_CONFIG_START + t, thecrc);   //write the crc to the end of the data
  if (EEPROM.read(EE_CONFIG_START + t) != thecrc)  //and check it
    errors = true;
  if (errors == true) {
    #ifdef DEBUG_SERIAL
    Serial.println(" error writing to EEPROM");
    #endif
  } else {
    #ifdef DEBUG_SERIAL
    Serial.printf("%d bytes saved to EEPROM version %s \n", byteswritten, EE_CONFIG_VERSION);  //note: only changed written
    #endif
  }
  return errors;
#else
  return false;
#endif
}

//specific load/save functions based on main-variables etc. 
boolean EEPROM_LOAD() { 
  config_t E;
  if (loadFromEEPROM(&E) == true) {
    
    EEsaved_count=E.EEsaved_count;

    for (int i = 0; i < 4; i++)
        versionEE[i]= E.version_of_settings[i];
    
    if (E.use_presets==1) //user has set use_presets to default to usersettings
      {
        play_SR=E.ply_SR; //replay speed
        rec_SR=E.rec_SR;
        oper_SR=E.oper_SR; //sample rate 
        record_detector=E.rec_detector_mode;
        use_presets=E.use_presets; // default or user settings at startup
        if (use_presets!=0) 
          {use_presets=1;} //enforce possible wrong saving
        detector_mode=E.detector_mode; // detector mode
        startup_detector=detector_mode;
        display_mode=E.display_mode; //display mode
        startup_display=display_mode;
        tft_backlight=E.tft_backlight;
        tft_sleep=E.tft_sleep;
        
        #ifdef USE_PWMTFT
          set_backlight(tft_backlight);
        #endif

        if (display_mode==no_graph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
        if (display_mode==waterfallgraph)
              {  tft.setRotation( 0 );
              }
        if (display_mode==spectrumgraph)
             {  tft.setScroll(0);
                tft.setRotation( 0 );
              }
        tft.fillScreen(COLOR_BLACK); //blank the screen

        tft.print(display_mode);  
        FFORMAT=E.SD_FFormat;

        TE_speed=E.TE_speed; //replay speed for TE
        TE_low=E.TE_low; //low frequency for TE
        TE_GAP=E.TE_GAP;
        detune_factor=E.TE_DETUNE;
        FFT_peak=E.TE_peak*25;

        #ifdef USE_HIPASS_FILTER
          HI_pass=E.HiPass;
          setHiPass();
        #endif
         
        FD_divider=E.FD_divider;
        osc_frequency=E.osc_frequency; // set centre frequency
        def_vol=E.volume;
        volume=def_vol;
        //set_vol(volume);
        mic_gain=E.mic_gain;
        def_gain=mic_gain;
        set_mic(mic_gain);

        ENCODER_TURN=E.Encoder_direction;
        enc_dn=ENCODER_TURN*-1;
        enc_up=ENCODER_TURN;

        ENCODER_DIVIDER=E.Encoder_divider; //steps on the encoder that equal a change. Default =4

        AREC_D=E.AREC_D; //automated recording duration in seconds
        AREC_P=E.AREC_P; //automated recording minimal pauze between recordings in seconds
        AREC_F=E.AREC_F; //automated recording minimal frequency in kHz
        AREC_S=E.AREC_S; //automated recording minimal signalstrength
        AREC_B=E.AREC_B; //automated recording break time
        cwheelpos[0]=E.COLOR_MENU;
        cwheelpos[1]=E.COLOR_HILIGHTMENU;
        cwheelpos[2]=E.COLOR_BACKMENU;
        if ((cwheelpos[0]+cwheelpos[1]+cwheelpos[2])>0)
         {ENC_MENU_COLOR=rainbow(cwheelpos[0],2);
         HILIGHT_MENU_COLOR =rainbow(cwheelpos[1],2);
          BCK_MENU_COLOR =rainbow(cwheelpos[2],1);
        } 

      }

    return true;
    }
    else
    {
      return false; // crc was wrong so probably a changed setup of the EE structure
    }
    
}   

void EEPROM_SAVE() {
  config_t E;
  E.EEsaved_count=EEsaved_count+1; //increase the save counter
  E.BatVersion = versionno;
  E.ply_SR=play_SR;
  E.rec_SR=rec_SR;
  E.oper_SR=oper_SR;
  E.use_presets=use_presets;
  E.detector_mode=startup_detector;
  E.rec_detector_mode=record_detector;
  E.display_mode=startup_display;
  E.SD_FFormat=FFORMAT;
  E.TE_low=TE_low;
  E.TE_speed=TE_speed;
  E.TE_GAP=TE_GAP;
  E.TE_DETUNE=detune_factor;
  E.FD_divider=FD_divider;
  E.TE_peak=FFT_peak/25;
  E.osc_frequency=osc_frequency;
   
  #ifdef USE_HIPASS_FILTER
  E.HiPass=HI_pass;
  #endif
  E.volume=def_vol; //use predefined and not current settings !!
  E.mic_gain=def_gain;
  E.Encoder_direction=ENCODER_TURN;
  E.Encoder_divider=ENCODER_DIVIDER; //steps on the encoder that equal a change. Default =4
  E.AREC_D=AREC_D; //automated recording duration in seconds
  E.AREC_P=AREC_P; //automated recording minimal pauze between recordings in seconds
  E.AREC_F=AREC_F; //automated recording minimal frequency in kHz
  E.AREC_S=AREC_S; //automated recording minimal signalstrength
  E.AREC_B=AREC_B; //automated recording break after silence
  
  E.COLOR_MENU=cwheelpos[0];
  E.COLOR_HILIGHTMENU=cwheelpos[1];
  E.COLOR_BACKMENU=cwheelpos[2];
  E.tft_backlight=tft_backlight;
  E.tft_sleep=tft_sleep;
  E.crc = 0; //will be overwritten
  //printConfig_t(&E);  //for debugging
  char theversion[] = EE_CONFIG_VERSION;
  for (int i = 0; i < 4; i++)
    E.version_of_settings[i] = theversion[i];

  saveInEEPROM(&E);
} // end void eeProm SAVE
 

#endif