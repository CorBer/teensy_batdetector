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

#ifndef _BAT_ENCODER_H
#define _BAT_ENCODER_H

#include <Encoder.h>
#include <Bounce.h>

//try to avoid interrupts as they can (possibly ?) interfere during recording
#define ENCODER_DO_NOT_USE_INTERRUPTS


#if defined(__MK66FX1M0__) 
  //Teensy 3.6
  #define MICROPUSH_RIGHT  37
  #define MICROPUSH_LEFT  38
  #define encoderButton_RIGHT  36
  #define encoderButton_LEFT  29
  #define encoderRight 34,35
  #define encoderLeft 30,31
#endif

#if defined(__IMXRT1062__)
//Teensy 4.1
  #define MICROPUSH_RIGHT  37
  #define MICROPUSH_LEFT  31
  #define encoderButton_RIGHT  29
  #define encoderButton_LEFT  34
  #define encoderRight 36,28
  #define encoderLeft 30,35
#endif

Bounce micropushButton_R = Bounce(MICROPUSH_RIGHT, 50);
Bounce micropushButton_L = Bounce(MICROPUSH_LEFT, 50);

uint8_t ButtonL_state=0;
uint8_t lastButtonL_state=0;
uint8_t longpressL=false;
unsigned long ButtonL_down=0;

Bounce encoderButton_R = Bounce(encoderButton_RIGHT, 50);
Encoder EncRight(encoderRight);
int32_t EncRightPos=0;
int32_t EncRightnewPos=0;
int EncRightchange=0;

Bounce encoderButton_L = Bounce(encoderButton_LEFT, 50);
Encoder EncLeft(encoderLeft);
int32_t EncLeftPos=0;
int32_t EncLeftnewPos=0;

int EncLeftchange=0;

/************** LButton Mode SETTINGS *****************/
const uint8_t    MODE_REC = 0; //
const uint8_t    MODE_PLAY = 1; //
const uint8_t    MODE_AUTOREC=2; //
const uint8_t    MODE_DETECT = 5; // default right
const uint8_t    MODE_DISPLAY=200; //when not using SD

uint8_t LeftButton_Mode=MODE_REC; //DEFAULT RECORDING MODE
//uint8_t LeftButton_Next=MODE_PLAY;
uint8_t RightButton_Mode=MODE_DETECT;

//************************* ENCODER variables/constants
const int8_t enc_menu=0; //changing encoder sets menuchoice
const int8_t enc_value=1; //changing encoder sets value for a menuchoice

const int8_t enc_leftside=0; //encoder
const int8_t enc_rightside=1; //encoder
// ENCODER TURN 1 (clockwise). Use ENCODER_TURN -1 for counter-clockwise
static int8_t ENCODER_TURN= -1; //default CC
int8_t ENCODER_DIVIDER=4;

static int8_t enc_up=1*ENCODER_TURN; //encoder goes up
static int8_t enc_nc=0;
static int8_t enc_dn=-1*ENCODER_TURN; //encoder goes down

int EncLeft_menu_idx=0;
int EncRight_menu_idx=1;

int EncLeft_function=enc_menu;
int EncRight_function=enc_menu;

uint8_t last_RightMenuidx;
uint8_t last_RightMenufunc;
              

// **END************ LEFT AND RIGHT ENCODER DEFINITIONS


// *********************************************  SPECIAL FUNCTIONS
//adapted constrain function to allow cycling values initial functioncopied from wiring.h
#define cyclic_constrain_old(amt, low, high) ({ \
  typeof(amt) _amt = (amt); \
  typeof(low) _low = (low); \
  typeof(high) _high = (high); \
  (_amt < _low) ? _high : ((_amt > _high) ? _low : _amt); \
})

uint cyclic_constrain(int param, int change, int low, int high)
{
  int amt=int(param)+change; //read parameter and add change
  if (amt>high) //restrict paramter to values between low-high but allow rotation from high->low and low->high
    {amt=low;}
  else
  {
    if (amt<low)
       {amt=high;}
  }
    
  return amt;  
}
#include "bat_menu.h"
//set default menu for most detector_modes (volume on the left and gain on the right and in value mode)
void defaultMenuPosition()
{EncLeft_menu_idx=MENU_VOL;
 EncLeft_function=enc_value;
 EncRight_menu_idx=MENU_GAIN_MIC;
 EncRight_function=enc_value;
 if (detector_mode==detector_heterodyne) 
     { EncRight_menu_idx=MENU_FRQ; //set frequency on right_encoder in value-mode
     }
}

//read encoder and store change up/down
void getEncoderchanges()
{
  EncLeftnewPos = EncLeft.read();
  if (EncLeftnewPos%ENCODER_DIVIDER==0) 
      {EncLeftnewPos=EncLeftnewPos/ENCODER_DIVIDER;} 
   else 
       {EncLeftnewPos=EncLeftPos;} 
 
  if (EncLeftnewPos>EncLeftPos)
   { EncLeftchange=enc_up; }
   else
   if (EncLeftnewPos<EncLeftPos)
   { EncLeftchange=enc_dn; }
   else
   { EncLeftchange=enc_nc; }
  
EncLeftPos=EncLeftnewPos;
EncRightnewPos = EncRight.read();

if (EncRightnewPos%ENCODER_DIVIDER==0) 
      {EncRightnewPos=EncRightnewPos/ENCODER_DIVIDER;} 
   else 
       {EncRightnewPos=EncRightPos;} 

 if (EncRightnewPos>EncRightPos)
   { EncRightchange=enc_up; } // up
   else
   if (EncRightnewPos<EncRightPos)
    { EncRightchange=enc_dn; } // down
   else
    { EncRightchange=enc_nc; } //no change =0
  EncRightPos=EncRightnewPos;
}

//read microbuttons and encoder-buttons
void updateButtonStatus()
{
  encoderButton_L.update();
  encoderButton_R.update();
  micropushButton_L.update();
  micropushButton_R.update();
  lastButtonL_state=micropushButton_L.read(); 
}
//set alle buttons with internal pullups
void ButtonsEncoders_pullup()
{
  pinMode(encoderButton_RIGHT,INPUT_PULLUP);
  pinMode(encoderButton_LEFT,INPUT_PULLUP);

  pinMode(MICROPUSH_RIGHT,INPUT_PULLUP);
  pinMode(MICROPUSH_LEFT,INPUT_PULLUP);
  delay(100);
}

// **************************  ENCODERS

#endif