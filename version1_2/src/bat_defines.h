/* TEENSYBAT DETECTOR (for TEENSY 3.6) 
 *
 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
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

#ifndef _BAT_DEFINES_H
#define _BAT_DEFINES_H

//USE THIS FILE TO SETUP PERSONAL DEFINES
//#define DEBUG //default OFF

#define UNIXBASE //define if OS is LINUX/MAC(UNIXBASE) or WINDOWS based (comment out !!)

#define USE_TEFACTOR //optional heterodyne module after TimeExpansion to lower the frequency on the output(headphone) (RAM 140 bytes Flash 700 bytes)

//HARDWARE SPECIFIC 

#define PWM //use PWM to control the backlight intensity of the TFT
#ifdef PWM
 #if defined(__MK66FX1M0__) //teensy 3.6
  #define USE_PWMTFT 17 //use PWM controlled backlight (pin 17) (8bytes RAM 1kb Flash)
  //#define USE_ADC_IN // use A2 (pin16) for direct microphone input //default OFF
 #endif
 #if defined(__IMXRT1062__) //teensy 4.1
  #define USE_PWMTFT 14
 #endif
#endif

// non model-specific hardware usage

//#define USE_GPS // use a GPS with serial on pin 0,1 (430 bytes RAM 7kB Flash) //default OFF
//#define USE_DS18B20 32 //use a thermometer with wire-one on pin 32. (50 bytes RAM 4kB Flash) //default OFF

//************(THIS WAS NOT USED DURING TESTING OF VERSION V1_2) ******************
//#define ADVANCEDMENU // will allow setting of INPUTSOURCE MIC/LINE_IN/ADC_IN 


#endif