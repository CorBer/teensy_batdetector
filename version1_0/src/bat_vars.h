/* BatDetector on Teensy 3.6
 * Copyright (c) 2020, Cor Berrevoets
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

#ifndef _BAT_VARS_H
#define _BAT_VARS_H

String versionStr=batversion;

//***************************************** available DETECTOR modes
#include "sec2time.h" // separate time routine library

#define detector_heterodyne 0
#define detector_Auto_heterodyne 1
#define detector_Auto_TE 2 // time expansion
#define detector_divider 3
#define detector_passive 4 // no processing at all

const char* DT [5] =
  {
    "Heterodyne",
    "AHeterodyne",
    "TExpansion",
    "FreqDiv",
    "Passive"
    
  };
//SHORT texts for modes
const char* DT_short[5]=
{
  "HT",
  "A-HT",
  "TE",
  "F_D",
  "Pass"
};


#define settings_page 3 //settings page display
#define waterfallgraph 2
#define spectrumgraph 1
#define no_graph 0

const char* setDisplay[4] =
  { "no graphs",
    "spectrum",
    "waterfall",
    "settings",
  };


//default modes
int startup_detector=detector_heterodyne;
int record_detector=detector_Auto_heterodyne; //default auto_heterodyne
int detector_mode=detector_heterodyne;
int last_detector_mode=detector_heterodyne;

int osc_frequency = 45000; // start heterodyne detecting at this frequency
int last_osc_frequency=osc_frequency; //used to return to proper listening setting after using the play_function
float freq_Oscillator =50000;

int startup_display=spectrumgraph;
int display_mode=spectrumgraph; //default display mode
int last_display_mode=spectrumgraph;

char versionEE[5];
int use_presets=1; //0= default values; 1=user values stored on EEprom;
int16_t EEsaved_count=0;

int8_t def_vol=40;
int8_t def_gain=30;

int8_t mic_gain = def_gain; // start detecting with this MIC_GAIN
int8_t volume=def_vol;

uint8_t TE_speed=10; //default auto TE is 1/10 of normal speed sounds
uint8_t TE_low=15 ; //lowest frequency to trigger auto TE
uint8_t TE_GAP=80; //minimal 80 ms between signals

uint8_t FD_divider=10; //default freq_divider

uint8_t LR_DELAY=0; //pseudo stereo

// ****************** TEENSY 3.6  BASED BUFFERS ***********************************

// WARNING: this setup uses a lot of memory in buffers (2x 96K !!)
#if defined(__MK66FX1M0__) // Teensy 3.6 // will result in errors if not compiled for teensy3.6 
  #define BUFF 96
  #define BUFFSIZE (BUFF*1024) // size of buffer to be written
#endif

 //automatic and  recording setup
uint wr;
uint32_t nj = 0;

uint8_t sample_buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
 
boolean AUTO_REC=false; // AUTORECORDING ACTIVE
boolean SD_ACTIVE=false; //SD card available

boolean continousPlay=false;
boolean recorderActive=false;
boolean playActive=false;
 
const uint16_t AREC_BLOCK=5000;
uint8_t AREC_D=2; //automated recording duration in 5 second blocks
uint8_t AREC_P=2; //automated recording minimal pauze between recordings in 5 second blocks
uint8_t AREC_B=1;  //automated recording break after silence in 5 second blocks
uint8_t AREC_F=20; //automated recording minimal frequency in kHz
uint8_t AREC_S=10; //automated recording minimal signalstrength

int cwheelpos[3]={0,0,0}; //colorwheel for menu
int cwheelmax=127;

const uint16_t FFT_points = 256;
float SR_FFTratio=1;

uint8_t spec_hi=240; //default 120
uint8_t spec_lo=2; //default 2
uint8_t spec_width=2;
uint powerSpectrum_Maxbin=0;
uint signal_LoF_bin;
uint signal_HiF_bin;
uint FFT_peakF_bin=0;

bool ADC_ON=false; //can be used with ADC_IN

uint8_t menu_id;

#ifdef USE_HIPASS_FILTER
  const uint8_t HI_PASS_OFF=10;
  uint8_t HI_pass=HI_PASS_OFF; 
#endif


#ifdef ADAPTED_SGTL_LIB
  boolean mic_ctrl_default=false;
  unsigned int sgtl5000_mic_ctrl=0x170; 
  boolean ref_ctrl_default=false;
  unsigned int sgtl5000_ref_ctrl=0x01F2;  //normal defaults
  unsigned int BIAS=0x0002;
  unsigned int VAG=0x1F;
#endif

boolean sample_UltraSound= false;  
boolean lastSample_UltraSound=false;//triggers when an ultrasonic signalpeak is found during FFT
boolean TE_ReplayEnded=true; //when a TEcall is played this signals the end of the call
uint long callLength=0; //changed in V114



#endif