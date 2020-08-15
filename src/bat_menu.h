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

#ifndef _BAT_MENU_H
#define _BAT_MENU_H

#include "bat_vars.h"
#include "bat_sd.h"


//this structure will be used to store the menu
struct Menu_Descriptor
{   uint8_t menu_id; //menu_id unique value
    const char* menu_txt; //text for the display
    bool active;
   
};
//MAIN DEFINITIONS 

//constants refering to a menu_id (the first 4 are STATIC positions == IDs)
const uint8_t  MENU_VOL=   0; //volume
const uint8_t  MENU_GAIN_MIC=  1; //mic_gain
const uint8_t  MENU_FRQ=  2; //frequency
const uint8_t  MENU_SR=    3; //sample rate
#ifdef USE_HIPASS_FILTER
  const uint8_t  MENU_HIP=    4; //sample rate
#endif

//settings depending on SDcard
const uint8_t MENU_REC=   100; //record
const uint8_t MENU_PLAY=  101; //play
const uint8_t MENU_PLAYD=   102; //play at original rate
const uint8_t MENU_AUTOREC=  103; //automated recording
//additional settings
const uint8_t  MENU_SETTINGS= 201; // SETTINGS menu

const uint8_t MENU_MAX= 10; //maximum number of entries

const Menu_Descriptor BaseMenu [MENU_MAX]
{ 
//available on both encoders  
   {MENU_VOL,      "Volume", true},
   {MENU_GAIN_MIC, "Gain", true},
   {MENU_FRQ,      "Freq.", true},
   {MENU_SR,       "SRate", true},
 #ifdef USE_HIPASS_FILTER 
   {MENU_HIP,"HIpass",true},
 #endif
//only available on left encoder   
   {MENU_PLAY,     "PLAY", true},
   {MENU_REC,      "REC", true},
   {MENU_AUTOREC,  "AutoREC", true},
#ifdef ADVANCEDMENU   
   {MENU_PLAYD,    "PLAYD", true},
#else
   {MENU_PLAYD,    "PLAYD", false},
#endif
   {MENU_SETTINGS, "SETTINGS", true},
};

uint8_t LeftMenuOptions=MENU_MAX; //will show all menu options
uint8_t RightMenuOptions=4; //allow up to SRate

//storage for specific menus 
Menu_Descriptor LeftBaseMenu [MENU_MAX] 
{  };
Menu_Descriptor RightBaseMenu [4] //only top 4 menu options are available on the right-encoder
{} ;

/*************************************************************SETTINGS MENU defines *********/
//int SET_MENU_pos=0;
#define SET_MENU_FONT Arial_13
//basic settings
#define SET_MENU_PAGE 255
 
#define SET_MENU_DISPLAY 0
#define SET_MENU_DETECTOR 1
#define SET_MENU_SR 2 //Sample Rate at startup
#define SET_MENU_STARTUPMODE 3 //EEprom or default settings
#define SET_MENU_HIPASS  4 //
#define SET_MENU_GAIN 5//
#define SET_MENU_VOL 6//

 //sample rates
//settings during recording/playback
#define SET_MENU_REC_DETECT 10 //detector mode during recording, HT or A-HT (will use FFT)
#define SET_MENU_SR_REC 11 //Sample Rate
#define SET_MENU_SR_PLY 12 //Sample Rate
#define SET_MENU_FFORMAT 13 //fileformat

#define SET_MENU_TE_LOW   100 //TE_LOW
#define SET_MENU_TE_SPD   101 //TE_SPD
#define SET_MENU_TE_GAP   102 //TE_GAP
#define SET_MENU_FD_DIV   105 //FD



//autorecording
#define SET_MENU_AREC_SR_REC 150 //Sample Rate
#define SET_MENU_AREC_DURATION 151 //duration in seconds
#define SET_MENU_AREC_PAUSE 152 //pauze in seconds
#define SET_MENU_AREC_F 153 //lowest frequency
#define SET_MENU_AREC_S 154 //lowest signalstrength
#define SET_MENU_AREC_BREAK 155 //break the recording after a silence for X seconds

#define SET_MENU_TIME  201
#define SET_MENU_DATE  202
#define SET_MENU_ENC   203 //encoder CW CCW
#define SET_MENU_COLORS 210 //allow menu-color to be changed
#define SET_MENU_HICOLORS 211 //allow menu-color to be changed
#define SET_MENU_BCKCOLORS 212 //allow menu-color to be changed

//advanced menuparts >240
#define SET_MENU_VAG 241
#define SET_MENU_INPUT 242
#define SET_MENU_ADC 243
#define SET_MENU_FFT_N 244
#define SET_MENU_LR_DELAY  245 //LR_DELAY

#define SET_MENU_SKIP 254 // skip half a line ?? TODO !!

int8_t settings_page_nr=0; //active settings page
const uint8_t settings_menu_pages=3;//0,1,2
//topstructure for settingspages
const char* settings_page_name[settings_menu_pages]={"DEFAULTS","AUTORECORD","SETUP"};
uint8_t set_menu_id[settings_menu_pages]={SET_MENU_PAGE,SET_MENU_PAGE,SET_MENU_PAGE}; 
uint8_t settings_MenuOptions[settings_menu_pages]={0,0,0};
int8_t set_menu_pos[settings_menu_pages]={0,0,0}; //startup positions of settings_menus

int8_t timemenu_pos=0; // hours or minutes (0,1)
//gapindicator = > //menu will jump a few pixels
const Menu_Descriptor Settings0Menu []
{  {SET_MENU_PAGE,        "SETTINGSpage", true}, //needs to be the 1st option of a settings menu
   {SET_MENU_DETECTOR,    "Detector", true},
   {SET_MENU_DISPLAY,     "Display", true},
   {SET_MENU_VOL,         "Volume",true},
   {SET_MENU_GAIN,        "Gain",true},
   
   {SET_MENU_HIPASS,      "HighPass", true},
   
   {SET_MENU_REC_DETECT,  "Record Detect", true},

   {SET_MENU_SR,          ">SampleRate", true},
   {SET_MENU_SR_REC,      "SampleRate Rec", true},
   {SET_MENU_SR_PLY,      "SampleRate Play", true},

   {SET_MENU_TE_SPD,      "TE speed", true},
   {SET_MENU_TE_GAP,      ">TE gap (ms)", true},
   {SET_MENU_TE_LOW,      "TE low freq.", true},
   {SET_MENU_FD_DIV,      "FD divider", true},
   
    
         
};

const Menu_Descriptor Settings1Menu []=
{
 {SET_MENU_PAGE,"SETTINGSpage", true},
 {SET_MENU_AREC_SR_REC, "Recorder SampleRate",true},
 {SET_MENU_AREC_DURATION,"max Duration(s)",true},
 {SET_MENU_AREC_BREAK,"max Silence(s)",true},
 {SET_MENU_AREC_PAUSE,"min Pause(s)",true},
 {SET_MENU_AREC_F,"Low Freqency",true},
 {SET_MENU_AREC_S,"Low signal strength",true}
};

/****SETTINGS MENU2*********/
const Menu_Descriptor Settings2Menu []=
  { {SET_MENU_PAGE,"SETTINGSpage", true},
    {SET_MENU_TIME,"TIME HHMM", true},
    {SET_MENU_DATE,"YYYYMMDD", true},
      
    {SET_MENU_ENC, "Encoder", true},
     
    {SET_MENU_STARTUPMODE, "Use EEprom", true},
    {SET_MENU_FFORMAT, "Fileformat", true},
  
    {SET_MENU_COLORS,">Menu Color", true},
    {SET_MENU_HICOLORS,"HilightColor", true},
    {SET_MENU_BCKCOLORS,"Back-Color", true},
   
     #ifdef ADVANCEDMENU   
          {SET_MENU_INPUT,       ">INPUT", true},
          {SET_MENU_FFT_N,       "FFT_N", true},
          {SET_MENU_LR_DELAY,      "LR delay", true},
        #ifdef ADAPTED_SGTL_LIB
          {SET_MENU_VAG,         "VAG", true},
        #endif 
        #ifdef USE_ADC_IN  
            {SET_MENU_ADC,       "use ADC_IN", true},
        #endif
          
     #endif

  };

void build_menu_structures()
{
menu_id=BaseMenu[0].menu_id; //always set to default position of
uint menucount=0;
//leftside menu
for (uint8_t i=0; i<LeftMenuOptions; i++)
{ 
  if (SD_ACTIVE==true)
  { if (BaseMenu[i].active==true)
     {LeftBaseMenu[menucount]=BaseMenu[i];
      menucount++;}
  }
  else
  {if ((BaseMenu[i].menu_id!=MENU_REC) and (BaseMenu[i].menu_id!=MENU_PLAY) and (BaseMenu[i].menu_id!=MENU_PLAYD) and (BaseMenu[i].menu_id!=MENU_AUTOREC) 
         ) 
       { if (BaseMenu[i].active==true)
         {LeftBaseMenu[menucount]=BaseMenu[i];
          menucount++;
         }
       }
  }
  
}

LeftMenuOptions=menucount;
menu_id=BaseMenu[0].menu_id;
D_PRINT("LEFTBASEMENU ") D_PRINTLN(menucount)

//rightBase will stop before MENU_REC
menucount=0;
for (uint8_t i=0; i<RightMenuOptions; i++)
{ 
  RightBaseMenu[menucount]=BaseMenu[i];
   menucount++;
  
}
RightMenuOptions=menucount;
D_PRINT("RIGHTBASEMENU ") D_PRINTLN(menucount)
/**** BUILD SETTINGS MENU 0 1 2 *************************/

settings_MenuOptions[0]=sizeof (Settings0Menu) / sizeof (Settings0Menu[0]);
settings_MenuOptions[1]=sizeof (Settings1Menu) / sizeof (Settings1Menu[0]);
settings_MenuOptions[2]=sizeof (Settings2Menu) / sizeof (Settings2Menu[0]);

D_PRINTXY("SETMENU0 ",settings_MenuOptions[0]);
D_PRINTXY("SETMENU1 ",settings_MenuOptions[1]);
D_PRINTXY("SETMENU2 ",settings_MenuOptions[2]);

//run over all settings_pages
for (uint8_t j=0; j<settings_menu_pages; j++)
{
menucount=0; //reset the counter

for (uint8_t i=0; i<settings_MenuOptions[j]; i++)
 { 
  if ((j==0) and (Settings0Menu[i].active==true))
     { menucount++;}
  if ((j==1) and (Settings1Menu[i].active==true))
     { menucount++;}   
  if ((j==2) and (Settings2Menu[i].active==true))
     { menucount++;}
 }  
 settings_MenuOptions[j]=menucount;
}
}

char * getMenuTxtVal(uint8_t menu_id, uint8_t update_page)
{  const uint8_t tlen=12;
   static char tstr[12];
   struct tm tx = seconds2time(RTC_TSR);
   switch (menu_id) {
      case SET_MENU_PAGE:
        snprintf(tstr,tlen, settings_page_name[update_page]);
      break; 
      case SET_MENU_TE_LOW:
        snprintf(tstr,tlen, "%02d kHz", TE_low);
        break;
      case SET_MENU_TE_GAP:
        snprintf(tstr,tlen, "%03d", TE_GAP);
        break;
      case SET_MENU_TE_SPD:
        snprintf(tstr,tlen, "1/%02d", TE_speed);
        break;
      case SET_MENU_FD_DIV:
        snprintf(tstr,tlen, "1/%02d", FD_divider);
      break;
      case SET_MENU_SR:
        snprintf(tstr,tlen,"%s",SR[oper_SR].txt);
      break;
      case SET_MENU_SR_REC:
        snprintf(tstr,tlen,"%s",SR[rec_SR].txt);
        break;

      case SET_MENU_SR_PLY:
        snprintf(tstr,tlen,"%s",SR[play_SR].txt);
        break;
      case SET_MENU_FFORMAT:
        if (FFORMAT==FFORMAT_RAW)
           {snprintf(tstr,tlen,"%s",".RAW");
           }
         else
         {
           snprintf(tstr,tlen,"%s",".WAV");
         }
        break;

      case SET_MENU_DISPLAY:
        snprintf(tstr,tlen,"%s",setDisplay[startup_display]);
        break;
      case SET_MENU_DETECTOR:
        snprintf(tstr,tlen,"%s",DT[startup_detector]);
        break; 
      case SET_MENU_REC_DETECT:
        snprintf(tstr,tlen,"%s",DT[record_detector]);
        break; 
        
      case SET_MENU_TIME:
        //struct tm tx = seconds2time(RTC_TSR);
        //show which part can be changed currently
        if (timemenu_pos==0)
            {snprintf(tstr,tlen, "[%02d]%02d", tx.tm_hour, tx.tm_min);
            }
        if (timemenu_pos>=1)
            {snprintf(tstr,tlen, "%02d[%02d]", tx.tm_hour, tx.tm_min);
            }
        break;
      case SET_MENU_DATE:
        //struct tm tx = seconds2time(RTC_TSR);
        tx = seconds2time(RTC_TSR);
        if (timemenu_pos==0)
            {snprintf(tstr,tlen, "[%04d]%02d%02d", tx.tm_year+1970, tx.tm_mon, tx.tm_mday);
            }
        if (timemenu_pos==1)
            {snprintf(tstr,tlen, "%04d[%02d]%02d", tx.tm_year+1970, tx.tm_mon, tx.tm_mday);
            }
        if (timemenu_pos==2)
            {snprintf(tstr,tlen, "%04d%02d[%02d]", tx.tm_year+1970, tx.tm_mon, tx.tm_mday);
            }
        break;

      case SET_MENU_STARTUPMODE:
      if (use_presets==0)
          {snprintf(tstr,tlen, "OFF");
            }
          else
          {snprintf(tstr,tlen, "ON");
              };
      break;

      case SET_MENU_GAIN:
         snprintf(tstr,tlen, "%03d", def_gain);
      break;
      
      case SET_MENU_VOL:
         snprintf(tstr,tlen, "%03d", def_vol);
      break;

      #ifdef USE_HIPASS_FILTER
      case SET_MENU_HIPASS:
       if (HI_pass>HI_PASS_OFF)
        {snprintf(tstr,tlen, "%02d kHz", HI_pass);
        }
        else
        {
          snprintf(tstr,tlen, "OFF");
        }
        
      break;
      #endif
      
      case SET_MENU_AREC_DURATION:
        snprintf(tstr,tlen, "%03d", AREC_D*5);
      break;
      case SET_MENU_AREC_PAUSE:
        snprintf(tstr,tlen, "%03d", AREC_P*5);
      break;
      case SET_MENU_AREC_F:
        snprintf(tstr,tlen, "%02d kHz", AREC_F);
      break;
      case SET_MENU_AREC_S:
        snprintf(tstr,tlen, "%02d", AREC_S);
      break;
      case SET_MENU_AREC_BREAK:
        snprintf(tstr,tlen, "%03d", AREC_B*5);
      break;
      
      case SET_MENU_AREC_SR_REC:
        snprintf(tstr,tlen,"%s",SR[rec_SR].txt);
      break;
      
      case SET_MENU_COLORS:
        snprintf(tstr,tlen, "%03d", cwheelpos[0] );
      break;
      case SET_MENU_HICOLORS:
        snprintf(tstr,tlen, "%03d",  cwheelpos[1]);
      break;
      case SET_MENU_BCKCOLORS:        
        snprintf(tstr,tlen, "%03d",  cwheelpos[2]);
      break;      

      case SET_MENU_ENC:
      if (ENCODER_TURN==1)
            { snprintf(tstr,tlen, "Clockw");
            }
          else
            {snprintf(tstr,tlen, "C-Clockw");
            }
      break;
  
      #ifdef USE_ADC_IN 
      case SET_MENU_ADC:
           if (ADC_ON)
                {  snprintf(tstr,tlen,"On ");
                }
            else   
             {  snprintf(tstr,tlen,"Off ");
                }
          break;    
      #endif

      #ifdef ADVANCEDMENU            
          case SET_MENU_LR_DELAY:
          snprintf(tstr,tlen, "%03d", LR_DELAY);
          break;

          case SET_MENU_INPUT: 
          if (myInput==AUDIO_INPUT_LINEIN)
              {snprintf(tstr,tlen, "LINE_IN");
                }
              else
              {snprintf(tstr,tlen, "MIC_IN");
                  };
          break;
          #ifdef ADAPTED_SGTL_LIB
          case SET_MENU_VAG:
              snprintf(tstr,tlen, "%04X", VAG);
          break;
          #endif
          case SET_MENU_FFT_N:
              snprintf(tstr,tlen, "%01d", FFT_N);
          break;    
                    
      #endif 

    default:
    // statements
    break;
  }


 return tstr;
}




#endif