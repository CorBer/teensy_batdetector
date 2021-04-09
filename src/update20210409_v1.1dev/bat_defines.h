/* TEENSYBAT DETECTOR (for TEENSY 3.6) 
 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
 */

#ifndef _BAT_DEFINES_H
#define _BAT_DEFINES_H

//USE THIS FILE TO SETUP PERSONAL DEFINES
#define DEBUG //default OFF
#define PIOCOMPILER  //when compiling with PIO (platformIO)  
#define USE_SD //to use the SD-card for recording/replay
#define USE_HEX_FILENAMES //only relevant in combination with USE_SD, filename = HEX representation of time in seconds since 1jan1970
#define USE_HIPASS_FILTER // low frequent sounds can be filtered before further processing (RAM 300 bytes Flash 7Kb)
#define USE_TEFACTOR //heterodyne module after TimeExpansion to lower the frequency on the output(headphone) (RAM 140 bytes Flash 700 bytes)
//#define ADVANCEDMENU // will allow setting of INPUTSOURCE MIC/LINE_IN/ADC_IN

//#define USE_STAMPLOG //allows to log recording actions(start/end) onto a SD file called "TIME_LOG.log"

//HARDWARE SPECIFIC
#define USE_PWMTFT 17 //use PWM controlled backlight (pin 17) (8bytes RAM 1kb Flash)
//#define USE_ADC_IN // use A2 (pin16) for direct microphone input //default OFF
//#define USE_GPS // use a GPS with serial on pin 0,1 (430 bytes RAM 7kB Flash) //default OFF
//#define USE_DS18B20 32 //use a thermometer with wire-one on pin 32. (50 bytes RAM 4kB Flash) //default OFF

#endif