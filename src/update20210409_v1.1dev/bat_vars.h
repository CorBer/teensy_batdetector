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

// union {
//     const char DOUBLE_DATE[18];
//     const char PAD[19];
// } DATE_HELPER = { __DATE__ " " __DATE__ };

// const char *MY_DATE = DATE_HELPER.DOUBLE_DATE + 7;

String versionStr=batversion;//+String(MY_DATE);

//***************************************** available DETECTOR modes
#include "sec2time.h" // separate time routine library
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
  { "no graphs",
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

uint8_t FD_divider=10; //default freq_divider

//factor to follow frequency-peaks in Auto_heterodyne mode
const uint8_t AHT_factor=100; //default AHT 100= HT equals peak

uint16_t FFT_peak=512;

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

char tRecStart[20];

boolean AUTO_REC=false; // AUTORECORDING ACTIVE
boolean SD_ACTIVE=false; //SD card available

boolean continousPlay=false;
boolean recorderActive=false;
boolean playActive=false;
boolean playDirect=false;

const uint16_t AREC_5SEC=5000;
const uint16_t AREC_1SEC=1000;

uint8_t AREC_D=2; //automated recording duration in 5 second blocks =10
uint8_t AREC_P=2; //automated recording minimal pauze between recordings in 5 second blocks =10
uint8_t AREC_B=5;  //automated recording break after silence in 1 second blocks =5
uint8_t AREC_F=20; //automated recording minimal frequency in kHz
uint8_t AREC_S=10; //automated recording minimal signalstrength 10*lowpeak

int cwheelpos[3]={0,0,0}; //colorwheel for menu
int cwheelmax=127;

const uint16_t FFT_points = 256;
float SR_FFTratio=1;

uint8_t spec_hi=240; //default 120
uint8_t spec_lo=2; //default 2
uint8_t spec_width=2;
uint powerSpectrum_Maxbin=0;
uint8_t signal_LoF_bin;
uint signal_HiF_bin;
uint FFT_peakF_bin=0;

bool ADC_ON=false; //can be used with ADC_IN
#define AUDIO_INPUT_ADC     2

uint8_t menu_id;

#ifdef USE_HIPASS_FILTER
  const uint8_t HI_PASS_OFF=10;
  uint8_t HI_pass=HI_PASS_OFF; 
  uint8_t last_HI_pass=HI_PASS_OFF;
#endif

#ifdef USE_TEFACTOR
  uint16_t DETUNE=1000; //1Khz detune steps 
  uint8_t detune_factor=100;
#endif

float sine_amplitude=1.0;

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

#ifdef USE_GPS
  #include "TinyBatGPS.h"
  /* This sample code demonstrates the normal use of a TinyGPS object. */
  TinyGPS gps;
  HardwareSerial GPSuart = HardwareSerial(0);
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
uint long callLength=0; //changed in V114

#ifdef USE_GPS
void readGPS()
{
                unsigned long start = millis();
                // Every 1 seconds we print an update
                while (millis() - start < 1000) {
                    if (GPSuart.available()) { 
                        char c = GPSuart.read();
                        if (gps.encoder(c)) {
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
             
                gps.get_fix0(&gps_fix);
                no_gps=gps_fixage/(60000); 
                gps_HDOP=gps.hdop();
                gps_altitude=gps.altitude();
        
            
}
#endif
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