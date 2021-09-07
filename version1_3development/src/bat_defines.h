/* TEENSYBAT DETECTOR (for TEENSY 3.6/4.1)

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

 /* TEENSY 3.6 CURRENT PINSETUP (20210309)
     pins marked with X are default in use
     pins marked with * are used for optional functions
     pins mared with . are not (yet) in use
    [P]=PCB
    [A]=AUDIOBOARD
    [T]=TEENSY
    r=resistor 100/150ohm

    [P]=[A]═══[T]              GND                X Vin  - PREAMP V+    [T]═══[A]=[P]
                     GPS TX -> RX0 *              X  Analog GN D        [T]═══[A]=[P]
                     GPS RX -> TX1 *              .  3.3V MEMS MIC      [T]   [A]=[P]
                                 2 .              23 AUDIO -LRCLK       [T]=r=[A]=[P]
                    I2C reserved 3 *              22 AUDIO -TX          [T]=r=[A]=[P]
                    I2C reserved 4 *              21 <-TFT CS           [T]=r=[A]=[P]
                                 5 .              20 <-TFT DC           [T]=r=[A]=[P]
    [P]=[A]=r=[T]    AUDIO MEMCS 6                19 AUDIO - SCL        [T]=r=[A]=[P]
    [P]=[A]=r=[T]    AUDIO MOSI  7                18 AUDIO - SDA        [T]=r=[A]=[P]
                                 8 .           *  17 TFT-PWM option     [T]═══[A]=[P]
    [P]=[A]=r=[T]    AUDIO BCLK  9             *  16 A2
    [P]=[A]=r=[T]    AUDIO SDCS 10                15 AUDIO -VOL         [T]=r=[A]=[P]
    [P]=[A]=r=[T]    AUDIO MCLK 11                14 AUDIO -SCLK        [T]=r=[A]=[P]
    [P]=[A]=r=[T]    AUDIO MISO 12                13 AUDIO -RX          [T]=r=[A]=[P]
                              3.3V .               X  GND               [T]═══════[P]
                                24 .           .  A22 DAC1
                                25 .           .  A21 DAC0
                                26 .              39  TFT MISO          [T]=r═════[P]
    [P]═════r=[T]   TFT SCLK->  27                38  MICROPUSH_L       [T]=r═════[P]
    [P]═════r=[T]   TFT MOSI->  28                37  MICROPUSH_R       [T]=r═════[P]
    [P]═════r=[T]  ENC_L-BUTTON 29                36  ENC_R-BUTTON      [T]=r═════[P]
    [P]═════r=[T]  ENC_L A      30                35  ENC_R A           [T]=r═════[P]
    [P]═════r=[T]  ENC_L B      31                34  ENC_R B           [T]=r═════[P]
                   DS18B20  T   32 *            . 33


 // TEENSY 4.1 CURRENT PINSETUP (20210527)
     pins marked with X are default in use
     pins marked with * are used for optional functions
     pins marked with . are not (yet) in use

 [P]═[A]═══[T]               GND                   Vin PREAMP V+    [T]═══[A]=[P]
 [P]=[A]=r=[T]   * GPS TX -> RX0 *                 Analog GN D      [T]═══[A]=[P]
                               1 *              .  3.3V - MEMS MIC  [T]   [A]=[P]
                               2 .                 23 AUDIO -MCLK   [T]=r=[A]=[P]
                  I2C reserved 3 *              .  22 ADC-A8
                  I2C reserved 4 *                 21 AUDIO BCLK    [T]=r=[A]=[P]
                               5 .                 20 AUDIO LRCLK   [T]=r=[A]=[P]
                   AUDIO MEMCS 6 *                 19 AUDIO - SCL   [T]=r=[A]=[P]
 [P]=[A]=r=[T]     AUDIO DIN   7                   18 AUDIO - SDA   [T]=r=[A]=[P]
 [P]=[A]=r=[T]     AUDIO DOUT  8                .  17 ADC-A3
                               9 .              .  16 ADC-A2
                   AUDIO SDCS 10 *              *  15 AUDIO -VOL
                   AUDIO MOSI 11 *                 14 TFT-PWM       [T]═══[A]=[P]
                   AUDIO MISO 12 *              *  13 AUDIO -SCLK
                            3.3V .                 GND              [T]═══════[P]
                              24 .              .  41 ADC-A17
                              25 .                 40 TFT_DC        [T]═r═════[P]
 [P]═════r═[T]      TFT_MOSI  26                   39 TFT_MISO      [T]═r═════[P]
 [P]═════r═[T]      TFT_SCLK  27                   38 TFT_CS        [T]═r═════[P]
 [P]═════r═[T]         ENC_R  28                   37 PUSH R        [T]═r═════[P]
 [P]═════r═[T]      ENCPUSH_R 29                   36 ENC_R         [T]═r═════[P]
 [P]═════r═[T]         ENC_L  30                   35 ENC_L         [T]═r═════[P]
 [P]═════r═[T]      PUSH L    31                   34 ENCPUSH_L     [T]═r═════[P]
                    DS18B20   32 *              .  33

 */

#ifndef _BAT_DEFINES_H
#define _BAT_DEFINES_H

#include <TimeAltLib.h>

 //USE THIS FILE TO SETUP PERSONAL DEFINES
#define DEBUG       //default OFF

#define DEVELOPMENT //development version
#ifdef DEVELOPMENT
//these three settings allow indicators for recordings to SD to allow inspection of possible artefacts (timing etc)
//#define AGC_MARKER
//#define BUFFER_MARK //mark the end of each buffer
//#define DUMMYDATA //allows data to be altered before writing to SD to check for continuity
#endif

#define EXPERIMENTAL // will allow settings that could be less stable or not fully tested in certain setup/situations
#ifdef EXPERIMENTAL
#define PEAKBIN_SERIAL //will send a SERIAL message at the end each incoming ultrasonic call 
#define TEMP_TIMECORRECT //allows to correct the DS18B20 temperature for a warming Teensy (gradual correction over 20 minutes after startup)
//#define USE_TEFACTOR //optional heterodyne module after TimeExpansion to lower the frequency on the output(headphone) (RAM 140 bytes Flash 700 bytes)
#endif

// GLOBAL NEW FEATURES in V1_3
//#define USE_SDCONFIG //V1_3: allow to read/write the current config to SD. Allows transport and restoring of configuration for several machines.
#define USE_FULLMODE_ON_REC //V1_3: will use active TFT and allow TE mode during recording
#define UTC_OFFSET 1; // hours used to show local time when using GPS (offset from GMT/UTC)

#define DEEPSLEEP //allow the system to cycling between deepsleep(very low power) and wakeup when using AUTORECORD during the day to save batterypower
#ifdef DEEPSLEEP
#define DEEPSLEEP_TIMER 10 //seconds wakeup delay. For a USBPACK it is necessary to this short to prevent the USBpowerack switching OFF
// #define DEEPSLEEP_LED 5 //allows a led on PIN 5 to blink during sleep at each wakeup shortly
#endif

//HARDWARE SPECIFIC options for T3.6
#if defined(__MK66FX1M0__) //teensy 3.6
#define USE_PWMTFT 17      //use PWM controlled backlight (pin 17) (8bytes RAM 1kb Flash)
//#define USE_GPS // optional GPS on SERIALPINS 0,1
//#define USE_DS18B20 32 //optional DS18B20 temperature sensor on PIN32
#define MAX_REPLAY_SR 312000 //maximum samplerate to use during direct replay
#define VIN_ON //measure VIN only on T36
#define VIN_LOW 3000
#endif

//HARDWARE SPECIFIC options for T4.1
#if defined(__IMXRT1062__) //teensy 4.1
#define USE_PWMTFT 14      //use PWM controlled backlight (pin 14)
//#define USE_GPS  // optional GPS on SERIALPINS 0,1  
//#define USE_DS18B20 32 //optional DS18B20 temperature sensor on PIN32
#define MAX_REPLAY_SR 384000 //maximum samplerate to use during direct replay

#define USE_PSRAM //if active PSRAM will be used for prebuffer/recordingbuffer
#ifdef USE_PSRAM
extern "C" uint8_t external_psram_size;
uint8_t PSRAMsize = external_psram_size; //always test PSRAM at startup
#endif

#define SCROLL_HORIZONTAL  //scroll waterfall horizontal

#endif





#endif



