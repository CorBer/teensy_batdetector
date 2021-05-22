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

#ifndef _BAT_TIMERS_H
#define _BAT_TIMERS_H
// ************************************  TIME *****************************
#include "core_pins.h"

#ifdef UNIXBASE //switch timebased calculations for different OS systems, LINUX/MAC are UNIXBASE
  #include <TimeLib.h>
#else                 //WINDOWS OS
  #include <TimeAltLib.h>
#endif

time_t getTeensy3Time()
{  return Teensy3Clock.get();
}

int helpmin; // definitions for time and date adjust - Menu
int helphour;
int helpday;
int helpmonth;
int helpyear;
int helpsec;
uint8_t old_time_min=0;
uint8_t old_time_sec=0;

uint8_t hour10_old;
uint8_t hour1_old;
uint8_t minute10_old;
uint8_t minute1_old;
uint8_t second10_old;
uint8_t second1_old;
bool timeflag = 0;
int16_t deltachange=0;


// **************************** TIMER VARS ********************************************

uint8_t calls_detected=0;
uint8_t ticks_detected=0;
  
uint8_t pulseD=0;
uint16_t pulseIPI=0;

float total_callduration=0;
float intercallduration=0;
float total_peakpower=0;
//continous timers
boolean showStart=false;

elapsedMillis time_since_StartDetection; //start timing directly after FFT detects an ultrasound
elapsedMillis time_since_EndDetection;
elapsedMillis time_since_tick; //start timing directly after FFT detects the end of the ultrasound
//elapsedMillis since_heterodyne=1000; //timing interval for auto_heterodyne frequency adjustments
elapsedMillis recording_running=0;
elapsedMillis recording_stopped=60000; // set at a high value so the detector is armed when auto_rec is started
elapsedMillis timeupdate=0;
void updateTimeMenu(int change)
{
              time_t time_tst = now();
              int8_t Tminute = minute(time_tst);
              int8_t Thour = hour(time_tst);
             
              if (timemenu_pos==0) 
                 { Thour=cyclic_constrain(Thour,change,0,23);
                 }
              if (timemenu_pos==1) 
                 { Tminute=cyclic_constrain(Tminute,change,0,59);
                   
                 }
              tmElements_t tmtm;
              breakTime(time_tst, tmtm);
              tmtm.Minute=Tminute;
              tmtm.Hour=Thour;
               
              Teensy3Clock.set(makeTime(tmtm));
              setTime(makeTime(tmtm));

}
void updateDateMenu(int change)
{
              time_t time_tst = now();
              int16_t Tyear = year(time_tst);
              int8_t Tmonth = month(time_tst);
                           
              if (timemenu_pos==0) 
                  { Tyear=cyclic_constrain(Tyear,change,2019,2025);
                    
                  }

              if (timemenu_pos==1) 
                  { Tmonth=cyclic_constrain(Tmonth,change,1,12);
                   }

               if (timemenu_pos==2) //possible to change to a wrong day so use a simple normal addition
                   { time_tst = now()+change*60*60*24;
                     Tyear = year(time_tst);
                     Tmonth = month(time_tst);
                 }   
              
              tmElements_t tmtm;
              breakTime(time_tst, tmtm);
              tmtm.Month=Tmonth;
              tmtm.Year=Tyear-1970;
              
              Teensy3Clock.set(makeTime(tmtm));
              setTime(makeTime(tmtm));

}


#endif