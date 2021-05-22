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

#ifndef _BAT_VARS_H
#define _BAT_VARS_H


String versionStr=batversion;

//***************************************** available DETECTOR modes

//#include "sec2time.h" // separate time routine library

int	tm_sec;
int	tm_min;
int	tm_hour;
int	tm_mday;
int	tm_mon;
int	tm_year;
int	tm_wday;
int	tm_yday;
int	tm_isdst;

#define EPOCH_YEAR 1970 //T3 RTC
#define LEAP_YEAR(Y) (((EPOCH_YEAR+Y)>0) && !((EPOCH_YEAR+Y)%4) && ( ((EPOCH_YEAR+Y)%100) || !((EPOCH_YEAR+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 
void seconds2time(uint32_t tt)
{ 
  tm_sec = tt % 60;
  tt /= 60; // now it is minutes
  tm_min = tt % 60;
  tt /= 60; // now it is hours
  tm_hour = tt % 24;
  tt /= 24; // now it is days
  tm_wday = ((tt + 4) % 7) + 1;  // Sunday is day 1 

  // tt is now days since EPOCH_Year (1970)
  uint32_t year = 0;  
  uint32_t days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= tt) year++;

  tm_year = year; // year is offset from 1970 

  // correct for last (actual) year
  days -= (LEAP_YEAR(year) ? 366 : 365);
  tt  -= days; // now tt is days in this year, starting at 0
  
  uint32_t month=0;
  uint32_t monthLength=0;
  for (month=0; month<12; month++) 
  {
    monthLength = monthDays[month];
    if ((month==1) & LEAP_YEAR(year)) monthLength++; 
    if (tt<monthLength) break;
    tt -= monthLength;
  }
  tm_mon = month + 1;  // jan is month 1  
  tm_mday = tt + 1;     // day of month

}

uint32_t lastmillis;
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

const char* setDisplay[3] =
  { "no graph",
    "spectrum",
    "waterfall",
  };


//default modes
int startup_detector=detector_Auto_TE;
int record_detector=detector_Auto_heterodyne; //default auto_heterodyne
int detector_mode=detector_Auto_TE;
int last_detector_mode=detector_Auto_TE;

int osc_frequency = 45000; // start heterodyne detecting at this frequency
int last_osc_frequency=osc_frequency; //used to return to proper listening setting after using the play_function
float freq_Oscillator =50000;
bool wavPSpectrum=false;
byte wavPowerSpectrum[128];
float cumsumPowerspectrum[128];
uint16_t current_peakPower;
uint16_t pulse_peakPower;
boolean found_peakdrop=false;
char spec[9]="........";
int startup_display=waterfallgraph;
int display_mode=waterfallgraph; //default display mode
int last_display_mode=waterfallgraph;
bool showEESaved=false;
char versionEE[5];
uint8_t use_presets=1; //0= default values; 1=user values stored on EEprom;
int16_t EEsaved_count=0;

int8_t def_vol=50;
int8_t def_gain=30;

int8_t mic_gain = def_gain; // start detecting with this MIC_GAIN
int8_t volume=def_vol;

uint8_t TE_speed=10; //default auto TE is 1/10 of normal speed sounds
uint8_t TE_low=15 ; //lowest frequency to trigger auto TE
uint8_t TE_GAP=30; //minimal 30 ms between signals
uint8_t TE_peak=20; //20*25 =500
uint8_t FD_divider=10; //default freq_divider

//factor to follow frequency-peaks in Auto_heterodyne mode
const uint8_t AHT_factor=100; //default AHT 100= HT equals peak

//uint16_t FFT_peak=512;

uint8_t tft_backlight=128;  //PWM SETTING but this needs to be available since it is a part of the EEPROM section
bool tft_sleep=false; //this needs to be available since it is a part of the EEPROM section 

#ifdef USE_PWMTFT
void set_backlight(uint8_t bck)
 {
  analogWriteResolution(8);
  //analogWriteFrequency(USE_PWMTFT, 375000);
  analogWrite(USE_PWMTFT,bck);
 }                
#endif

uint32_t freeram(){ // for Teensy 3.0
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is (approximately) the free, available ram.
    return stackTop - heapTop;
}


char* strlwr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = tolower((unsigned char) *tmp);
    }

    return s;
}

// ****************** TEENSY 3.6  BASED BUFFERS ***********************************

// WARNING: this setup uses a lot of memory in buffers (2x 96K !!)
// a large buffersize reduces the times data is written to the SDcard 
#if defined(__MK66FX1M0__) // Teensy 3.6 // will result in errors if not compiled for teensy3.6 
  #define BUFF 96
  #define BUFFSIZE (BUFF*1024) // size of buffer to be written
#endif

#if defined(__IMXRT1062__) // Teensy 4.1 
  #define BUFF 96
  #define BUFFSIZE (BUFF*1024) // size of buffer to be written
#endif


uint32_t getRTC_TSR()
{ 
  #if defined(__IMXRT1062__)
    return rtc_get();
  #endif 
  #if defined(__MK66FX1M0__)
    return RTC_TSR;
  #endif
}

 //automatic and  recording setup
uint wr; 
uint32_t nj = 0;

uint8_t sample_buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );

char tRecStart[20];

uint32_t rec_len; //length of the current recording in millis;

boolean AUTO_REC=false; // AUTORECORDING ACTIVE
boolean SD_ACTIVE=false; //SD card available

boolean continousPlay=false;
boolean recorderActive=false;
boolean playActive=false;
boolean playDirect=false;

const uint16_t AREC_5SEC=5000;
const uint16_t AREC_1SEC=1000;

uint8_t AREC_D=2; //automated recording duration in 5 second blocks =10
uint8_t AREC_P=2; //automated recording minimal pause between recordings in 5 second blocks =10
uint8_t AREC_B=5;  //automated recording break after silence in 1 second blocks =5
uint8_t AREC_F=20; //automated recording minimal frequency in kHz
uint8_t AREC_S=2000/25; //automated recording minimal signalstrength (4x higher than default threshold for TE_peak)

uint8_t colorscheme=0;
uint8_t max_colorscheme=0;
int cwheelpos[3]={0,0,0}; //colorwheel for menu
int cwheelmax=127;

const uint16_t FFT_points = 256;
float SR_FFTratio=1;

volatile const uint8_t spec_hi=240; //default 120
volatile const uint8_t spec_lo=2; //default 2
uint8_t spec_width=2;
uint8_t powerSpectrum_Maxbin=0;
uint8_t signal_LoF_bin;
uint8_t signal_HiF_bin;
uint8_t FFT_peakF_bin=0;

bool ADC_ON=false; //can be used with ADC_IN
#define AUDIO_INPUT_ADC     2

uint8_t menu_id;


  const uint8_t HI_PASS_OFF=10;
  uint8_t HI_pass=HI_PASS_OFF; 
  uint8_t last_HI_pass=HI_PASS_OFF;


#ifdef USE_TEFACTOR
  uint16_t DETUNE=1000; //1Khz detune steps 
  uint8_t detune_factor=100;
#endif

float sine_amplitude=1.0;

//*************************************ADDITION COLORS FOR THE TFT

#define RGB888_RGB565(color) ((((color) >> 19) & 0x1f) << 11) | ((((color) >> 10) & 0x3f) << 5) | (((color) >> 3) & 0x1f)

#define COLOR_BLACK          RGB888_RGB565(0x000000u)
#define COLOR_WHITE          RGB888_RGB565(0xFFFFFFu)
#define COLOR_BLUE           RGB888_RGB565(0x0000FFu)
#define COLOR_GREEN          RGB888_RGB565(0x00FF00u)
#define COLOR_RED            RGB888_RGB565(0xFF0000u)
#define COLOR_NAVY           RGB888_RGB565(0x000080u)
#define COLOR_DARKBLUE       RGB888_RGB565(0x00008Bu)
#define COLOR_DARKGREEN      RGB888_RGB565(0x006400u)
#define COLOR_DARKCYAN       RGB888_RGB565(0x008B8Bu)
#define COLOR_CYAN           RGB888_RGB565(0x00FFFFu)
#define COLOR_TURQUOISE      RGB888_RGB565(0x40E0D0u)
#define COLOR_INDIGO         RGB888_RGB565(0x4B0082u)
#define COLOR_DARKRED        RGB888_RGB565(0x800000u)
#define COLOR_OLIVE          RGB888_RGB565(0x808000u)
#define COLOR_GRAY           RGB888_RGB565(0x808080u)
#define COLOR_SKYBLUE        RGB888_RGB565(0x87CEEBu)
#define COLOR_BLUEVIOLET     RGB888_RGB565(0x8A2BE2u)
#define COLOR_LIGHTGREEN     RGB888_RGB565(0x90EE90u)
#define COLOR_DARKVIOLET     RGB888_RGB565(0x9400D3u)
#define COLOR_YELLOWGREEN    RGB888_RGB565(0x9ACD32u)
#define COLOR_BROWN          RGB888_RGB565(0xA52A2Au)
#define COLOR_DARKGRAY       RGB888_RGB565(0xA9A9A9u)
#define COLOR_SIENNA         RGB888_RGB565(0xA0522Du)
#define COLOR_LIGHTBLUE      RGB888_RGB565(0xADD8E6u)
#define COLOR_GREENYELLOW    RGB888_RGB565(0xADFF2Fu)
#define COLOR_SILVER         RGB888_RGB565(0xC0C0C0u)
#define COLOR_LIGHTGREY      RGB888_RGB565(0xD3D3D3u)
#define COLOR_LIGHTCYAN      RGB888_RGB565(0xE0FFFFu)
#define COLOR_VIOLET         RGB888_RGB565(0xEE82EEu)
#define COLOR_AZUR           RGB888_RGB565(0xF0FFFFu)
#define COLOR_BEIGE          RGB888_RGB565(0xF5F5DCu)
#define COLOR_MAGENTA        RGB888_RGB565(0xFF00FFu)
#define COLOR_TOMATO         RGB888_RGB565(0xFF6347u)
#define COLOR_GOLD           RGB888_RGB565(0xFFD700u)
#define COLOR_ORANGE         RGB888_RGB565(0xFFA500u)
#define COLOR_SNOW           RGB888_RGB565(0xFFFAFAu)
#define COLOR_YELLOW         RGB888_RGB565(0xFFFF00u)

/************************************** COLOR SCHEME SETUP *******************************/
#include "bat_colorschemes.h"

uint16_t colourmap[1005];
const char*  colorscheme_name;
uint16_t colorscheme_gamma_preset=1000;
uint16_t colorschemegamma_set=1000;

//setup a colourmap for the display of 1000 colours. 
#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) )

void convertArraytoColourMap(const float inarray[][3], uint16_t rows, float multiplier)
{ D_PRINTXY("ROWS",rows);
  uint8_t divider=uint8_t(1000/rows)+1;  //setup the divider to fill 1000 based on the inarray dimensions
   //we aim to fill 1000 values in colourmap
  for (uint16_t i = 0; i < 999; i++) 
    { uint16_t val=i/divider; //val is used as an index in the colourscheme //LINEAR
      if (colorscheme_gamma_preset!=100)
         { val= uint16_t((pow(i/1000.0f,colorscheme_gamma_preset/1000.0f)*rows)); //GAMMA
         } 
      uint8_t red=uint8_t(inarray[val][0]*multiplier);
      uint8_t green=uint8_t(inarray[val][1]*multiplier);
      uint8_t blue=uint8_t(inarray[val][2]*multiplier);
      colourmap[i]= color565(red,green,blue);
     }
}

void setcolourmap(uint8_t scheme)
{ 
  max_colorscheme=9; // how many palettes are available
  
  D_PRINTXY("SCHEME",scheme);
  
   if (scheme==0)
   {
	colorscheme_name="Default";
	for (uint16_t i = 0; i < 999; i++)
   { float val=i*10;
     if (colorschemegamma_set!=1000)
       { colorscheme_gamma_preset=colorschemegamma_set;
         val= pow(i/1000.0f,colorschemegamma_set/1000.0f)*10000;
        }
     colourmap[i]= color565(
                              min(255, val/3),
                              min(255, val/10),
                              min(255, val/40) 
							);
   }
   }
   
   if (scheme==1)
   {
	colorscheme_name="Grayscale";
	for (uint16_t i = 0; i < 999; i++)
   { float val=i*10;
     if (colorschemegamma_set!=1000)
       { colorscheme_gamma_preset=colorschemegamma_set;
         val= pow(i/1000.0f,colorschemegamma_set/1000.0f)*10000;
        }
     colourmap[i]= color565(
                              min(255, val/4),// /3),
                              min(255, val/4),// /10),
                              min(255, val/4) // /40) 
							);
   }
   }
   
   if (scheme==2)
   {
	colorscheme_name="Rainbow";
	for (uint16_t i = 0; i < 999; i++)
   { float val=i*10;
    
     if (colorschemegamma_set!=1000)
       { colorscheme_gamma_preset=colorschemegamma_set;
         val= pow(i/1000.0f,colorschemegamma_set/1000.0f)*10000;
        }
     colourmap[i]= GetColor(val);
   }
   }

   if (scheme==3)
   {
   colorscheme_name="Fire";
	 for (uint16_t i = 0; i < 999; i++)
     { float val=i*10;

     if (colorschemegamma_set!=1000)
       { colorscheme_gamma_preset=colorschemegamma_set;
         val= pow(i/1000.0f,colorschemegamma_set/1000.0f)*10000;
        }
     colourmap[i]= GetHotColor(val);
     }
   }   
   
   if (scheme==4)
   {    
	colorscheme_name="Inferno";
	colorscheme_gamma_preset=800;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
	convertArraytoColourMap(Inferno, arr_len(Inferno),255);
   }

   if (scheme==5)
   {
	colorscheme_name="Moreland";
	colorscheme_gamma_preset=1100;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
    convertArraytoColourMap(Moreland, arr_len(Moreland),255);
   }
   
   if (scheme==6)
   {
	colorscheme_name="Viridis";
	colorscheme_gamma_preset=900;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
    convertArraytoColourMap(Viridis, arr_len(Viridis),255);
    }
	
   if (scheme==7)
   {
	colorscheme_name="Plasma";
	colorscheme_gamma_preset=1100;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
	convertArraytoColourMap(Plasma, arr_len(Plasma),255);
   }
  
   if (scheme==8)
   {
	colorscheme_name="Rainbow1";
	//colorscheme_gamma_preset=1000;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
    convertArraytoColourMap(Rainbow1, arr_len(Rainbow1),1);
   }
    if (scheme==9)
   {
	colorscheme_name="ColorBlind";
	//colorscheme_gamma_preset=1000;
  if (colorschemegamma_set!=1000)
    {colorscheme_gamma_preset=colorschemegamma_set;}
    convertArraytoColourMap(ColorBlind, arr_len(ColorBlind),1);
   }
 
 D_PRINTXY("schemename:",colorscheme_name);

}

/* *************************************DS18B20 SETUP *********************************************/
#ifdef USE_DS18B20
  #include "OneWire.h"
  //#include "DallasTemperature.h"
  // Data wire is plugged into port 2 on the Arduino
  //#define ONE_WIRE_BUS USE_DS18B20
  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  OneWire ds(USE_DS18B20);
  // Pass our oneWire reference to Dallas Temperature. 
  //DallasTemperature sensors(&ds);
  // arrays to hold device address
  //DeviceAddress batThermo;
  byte DSaddr[8];
  float bat_tempC=-127; //start as disconnected
  boolean active_batThermo=false;

#endif

/* *************************************GPS SETUP *********************************************/
#ifdef USE_GPS
  #include "TinyGPS.h"
  /* This sample code demonstrates the normal use of a TinyGPS object. */
  TinyGPS gps;
  #if defined(__MK66FX1M0__) 
  HardwareSerial GPSuart = HardwareSerial(0);
  #endif
  #if defined(__IMXRT1062__) 

  HardwareSerial GPSuart=Serial1;
  
  #endif
  unsigned long gps_date=0;
  unsigned long gps_time=0;
  unsigned long gps_age=0;
  byte gps_fix=9;
  byte no_gps=10; 
  long gps_latitude=0;
  long gps_longitude=0;
  long gps_altitude=-9999;
  unsigned long gps_fixage=99999;

  unsigned short gps_sentences=999;
  unsigned short gps_failed_cs=999;
  unsigned long gps_HDOP=9999;

#endif  



boolean sample_UltraSound= false;  
boolean lastSample_UltraSound=false;//triggers when an ultrasonic signalpeak is found during FFT
boolean TE_ReplayEnded=true; //when a TEcall is played this signals the end of the call
uint32_t callLength=0; //changed in V114
float avgPD;
float avgIPI;
float avgPeakP;


/* *************************************GPS ROUTINES *********************************************/
#ifdef USE_GPS
void readGPS()
{
                unsigned long start = millis();
                // Every 1 seconds we receive an update
                while (millis() - start < 1000) {
                    if (GPSuart.available()) { 
                        char c = GPSuart.read();
                        if (gps.encode(c)) {
                           }
                       }
                   }
                unsigned long chars;
                gps.stats(&chars,&gps_sentences,&gps_failed_cs);
                gps.get_datetime(&gps_date,&gps_time,&gps_age);
                
                gps.get_position(&gps_latitude, &gps_longitude, &gps_fixage);
                
                if (gps_latitude==999999999)
                  {gps_latitude=0;}
                if (gps_longitude==999999999)
                  {gps_longitude=0;}
                
                //gps.get_fix0(&gps_fix);
                
                no_gps=gps_fixage/(60000); 
                gps_HDOP=gps.hdop();
                gps_altitude=gps.altitude();
        
            
}
#endif
/* *************************************DS18B20 ROUTINES *********************************************/
#ifdef USE_DS18B20
float readDS(bool serialOn)
{
  byte i;

  byte present = 0;
  byte type_s=0;
  byte data[12];
  
  float celsius=-127;
    
  if (serialOn)
  {Serial.print("ROM =");
   for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(DSaddr[i], HEX);
   }
  }

  bool connected=true;
  // if (OneWire::crc8(DSaddr, 7) != DSaddr[7]) {
  //     if (serialOn) {Serial.println("CRC is not valid!");}
  //     connected=false;
  // }
  if (connected)
  {
    switch (DSaddr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      type_s=9; //signal error
      if (serialOn)
        {Serial.println("Device is not a DS18x20 family device.");
        }
      
    } 

    ds.reset();
    ds.select(DSaddr);
    ds.write(0x44, 0);        // start conversion, NOT USING parasite power
    
    delay(100);     //based on 9bits data
    present = ds.reset();
    
    ds.select(DSaddr);   
    ds.write(0xBE);         // Read Scratchpad
    if (serialOn)
      { Serial.print("  Data = ");
        Serial.print(present, HEX);
        Serial.print(" ");
      }

    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      if (serialOn) 
      {Serial.print(data[i], HEX);
        Serial.print(" ");
      }
    }
    if (serialOn)
      {  Serial.print(" CRC=");
        Serial.print(OneWire::crc8(data, 8), HEX);
        Serial.println();
      }
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    }
  return celsius;
}
#endif


#endif