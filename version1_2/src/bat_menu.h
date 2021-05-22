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
const uint8_t  MENU_DISPLAY=4; // display


  const uint8_t  MENU_HIP=    5; //hipass filter


//settings depending on SDcard
const uint8_t MENU_REC=   100; //record
const uint8_t MENU_PLAY=  101; //play
const uint8_t MENU_AUTOREC=  103; //automated recording
//additional settings
const uint8_t  MENU_SETTINGS= 255; // SETTINGS menu

const uint8_t MENU_MAX= 12; //maximum number of entries

const Menu_Descriptor BaseMenu [MENU_MAX]
{ 
//available on both encoders  
   {MENU_VOL,      "Volume", true},
   {MENU_GAIN_MIC, "Gain", true},
   {MENU_FRQ,      "Freq.", true},
   {MENU_SR,       "SRate", true},
   
//only available on left encoder   
   {MENU_HIP,"HIpass",true},
   {MENU_PLAY,     "PLAY", true},
   {MENU_AUTOREC,  "AutoREC", true},
   {MENU_DISPLAY,  "Display", true},
   {MENU_SETTINGS, "SETTINGS", true},

};

uint8_t LeftMenuOptions=MENU_MAX; //will show all menu options
const uint8_t RIGHT_MAX=4;
uint8_t RightMenuOptions=RIGHT_MAX; //allow up to SRate

//storage for specific menus 
Menu_Descriptor LeftBaseMenu [MENU_MAX] 
{};
Menu_Descriptor RightBaseMenu [RIGHT_MAX] //only top X menu options are available on the right-encoder
{} ;

/*************************************************************SETTINGS MENU defines *********/
//int SET_MENU_pos=0;
#define SET_MENU_FONT Arial_13
//basic settings
#define SET_MENU_PAGE 255
 
#define SET_MENU_DISPLAY 0
#define SET_MENU_DETECTOR 1
#define SET_MENU_SR 2 //Sample Rate at startup
//#define SET_MENU_STARTUPMODE 3 //EEprom or default settings
#define SET_MENU_HIPASS  4 //
#define SET_MENU_GAIN 5//
#define SET_MENU_VOL 6//
#define SET_MENU_DETUNE  7 //
#define SET_MENU_LOWDETECT  7 //
 //sample rates
//settings during recording/playback
#define SET_MENU_REC_DETECT 10 //detector mode during recording, HT or A-HT (will use FFT)
#define SET_MENU_SR_REC 11 //Sample Rate
#define SET_MENU_SR_PLY 12 //Sample Rate

//live time-expansion
#define SET_MENU_TE_LOW   100 //TE_LOW lowest trigger frequency
#define SET_MENU_TE_SPD   101 //TE_SPD replay speed
#define SET_MENU_TE_GAP   102 //TE_GAP gap-time (ms)
#define SET_MENU_TE_DETUNE   103 //FACTOR for HT after TE
#define SET_MENU_FD_DIV   105 //FD freq divider setting

//autorecording
#define SET_MENU_AREC_SR_REC 150 //Sample Rate
#define SET_MENU_AREC_DURATION 151 //duration in seconds
#define SET_MENU_AREC_PAUSE 152 //pause in seconds
#define SET_MENU_AREC_F 153 //lowest frequency
#define SET_MENU_AREC_S 154 //lowest signalstrength
#define SET_MENU_AREC_BREAK 155 //break the recording after a silence for X seconds

#define SET_MENU_TIME  201
#define SET_MENU_DATE  202
#define SET_MENU_ENC   203 //encoder CW CCW
//#define SET_MENU_colorscheme_gamma_preset 208
#define SET_MENU_COLORSCHEME 209
#define SET_MENU_COLORS 210 //allow menu-color to be changed
#define SET_MENU_HICOLORS 211 //allow menu-color to be changed
#define SET_MENU_BCKCOLORS 212 //allow menu-color to be changed
#define SET_MENU_BCKLIGHT 213 //backlight level
#define SET_MENU_TFTSLEEP 214 //sleep in autorecord

//SD INFO
#define SET_MENU_SD_SIZE 220
#define SET_MENU_SD_USED 221
#define SET_MENU_SD_WAV 222
#define SET_MENU_SD_FORMAT 223
#define SET_MENU_SD_RECFOLDER 224
#define SET_MENU_SD_PLAYFOLDER 225

//GPS INFO
#define SET_MENU_GPS_LAT 230
#define SET_MENU_GPS_LON 231
#define SET_MENU_GPS_ALT 232
#define SET_MENU_GPS_HDOP 233
#define SET_MENU_GPS_FIXAGE 234

#define SET_MENU_GPS_SENTENCE 235
#define SET_MENU_GPS_FAILS 236

//advanced menuparts >240
#define SET_MENU_VAG 241
#define SET_MENU_INPUT 242
#define SET_MENU_ADC 243

//#define SET_MENU_SKIP 254 // skip half a line ?? TODO !!
uint8_t lastsettings_page_nr=0;
uint8_t settings_page_nr=0; //active settings page
uint8_t settings_menu_pages=4;//0,1,2,3,4 //default available
const uint8_t menu_pages_max=6;
//topstructure for settingspagesy
const char* settings_page_name[menu_pages_max]={"STARTUP DEFAULTS","TIME-EXPANSION LIVE","AUTORECORD","SETUP","SD-CARD","GPS DATA"};
uint8_t set_menu_id[menu_pages_max]={SET_MENU_PAGE,SET_MENU_PAGE,SET_MENU_PAGE,SET_MENU_PAGE,SET_MENU_PAGE,SET_MENU_PAGE}; 
uint8_t settings_MenuOptions[menu_pages_max]={0,0,0,0,0,0};
int8_t set_menu_pos[menu_pages_max]={0,0,0,0,0,0}; //startup positions of settings_menus

int8_t timemenu_pos=0; // hours or minutes (0,1)
//gapindicator = > //menu will jump a few pixels

const Menu_Descriptor Settings0Menu []
{  {SET_MENU_PAGE,        "SETTINGSpage", true}, //needs to be the 1st option of any settings menu
   {SET_MENU_DETECTOR,    "Detectormode", true},
   {SET_MENU_DISPLAY,     "Display", true},
   {SET_MENU_VOL,         ">Volume",true},
   {SET_MENU_GAIN,        "Gain",true},
   
   {SET_MENU_HIPASS,      ">>HighPass", true},

   {SET_MENU_SR,          ">>SampleRate", true},
   {SET_MENU_SR_REC,      "SampleRate Rec", true},
   {SET_MENU_SR_PLY,      "SampleRate Play", true},

   {SET_MENU_REC_DETECT,  ">>Record Detect", true},
        
};

const Menu_Descriptor Settings1Menu []=
  {{SET_MENU_PAGE,"SETTINGSpage", true},
   {SET_MENU_TE_SPD,      "replay speed", true},
   {SET_MENU_TE_GAP,      "max signal duration(ms)", true},
   {SET_MENU_TE_LOW,      "Signal peak above", true},
   {SET_MENU_LOWDETECT, "Signal strength above",true},
   #ifdef USE_TEFACTOR
     {SET_MENU_TE_DETUNE,    "detune(%)", true},
   #endif
   {SET_MENU_FD_DIV,      ">>FD divider", true},
   
  
  };
//AREC settings
const Menu_Descriptor Settings2Menu []=
{
 {SET_MENU_PAGE,"SETTINGSpage", true},
 {SET_MENU_AREC_SR_REC, "Recording SampleRate",true},
 {SET_MENU_AREC_F,"Signal Peak above",true},
 {SET_MENU_AREC_S,"Signal Strength above",true},
 {SET_MENU_AREC_DURATION,">>max Duration(s) of rec",true},
 {SET_MENU_AREC_BREAK,"max Silence(s) in rec",true},
 {SET_MENU_AREC_PAUSE,"Pause(s) between rec",true},
 {SET_MENU_TFTSLEEP,">>Display during rec",true}, 
 
};

/****SETTINGS MENU2*********/
const Menu_Descriptor Settings3Menu []=
  { {SET_MENU_PAGE,"SETTINGSpage", true},
    {SET_MENU_TIME,"TIME HHMM", true},
    {SET_MENU_DATE,"YYYYMMDD", true},
    {SET_MENU_ENC, ">>Encoder", true},
   
    {SET_MENU_COLORS,">>menuColor", true},
    {SET_MENU_HICOLORS,"hilightColor", true},
    {SET_MENU_BCKCOLORS,"backColor", true},
    {SET_MENU_COLORSCHEME, ">>ColorScheme", true},
   // {SET_MENU_colorscheme_gamma_preset, ">>ColorGamma", true},
    
    #ifdef USE_PWMTFT
       {SET_MENU_BCKLIGHT,">>Backlight",true},
    #endif
    #ifdef ADVANCEDMENU   
          {SET_MENU_INPUT,       ">INPUT", true},
    #endif

  };


const Menu_Descriptor Settings5Menu []=
{       {SET_MENU_PAGE,"SETTINGSpage", true},
        {SET_MENU_SD_SIZE,"Size", true},
        {SET_MENU_SD_USED,"Used", true},
       
       // {SET_MENU_SD_RECFOLDER,">>REC folder", true},
        {SET_MENU_SD_PLAYFOLDER,">>PLAY folder", true},
        {SET_MENU_SD_WAV,"Wav-files", true},
       // {SET_MENU_SD_FORMAT,">>FORMAT", true},
        
          };


#ifdef USE_GPS
const Menu_Descriptor Settings4Menu []=
{ {SET_MENU_PAGE,"SETTINGSpage", true},
    #ifdef USE_GPS
        {SET_MENU_GPS_LAT,       ">GPS LAT", true},
        {SET_MENU_GPS_LON,       "GPS LON", true},
        {SET_MENU_GPS_ALT,       "GPS ALT", true},
        {SET_MENU_GPS_FIXAGE,     ">GPS fixage", true},
        {SET_MENU_GPS_HDOP,       "GPS HDOP", true},
        {SET_MENU_GPS_SENTENCE,   "GPS SENTENCE", true},
        {SET_MENU_GPS_FAILS,      "GPS FAILS", true},
     #endif
  };
#endif

void build_menu_structures()
{
menu_id=BaseMenu[0].menu_id; //always set to default position of
uint8_t menucount=0;
//leftside menu
for (uint8_t i=0; i<LeftMenuOptions; i++)
{ 
  if (SD_ACTIVE==true)
  { if (BaseMenu[i].active==true)
     {LeftBaseMenu[menucount]=BaseMenu[i];
      menucount++;}
  }
  else
  {if ((BaseMenu[i].menu_id!=MENU_REC) and (BaseMenu[i].menu_id!=MENU_PLAY) and (BaseMenu[i].menu_id!=MENU_AUTOREC) 
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
settings_MenuOptions[3]=sizeof (Settings3Menu) / sizeof (Settings3Menu[0]);
#ifdef DEBUG_DETAIL
D_PRINTXY("SETMENU0 ",settings_MenuOptions[0]);
D_PRINTXY("SETMENU1 ",settings_MenuOptions[1]);
D_PRINTXY("SETMENU2 ",settings_MenuOptions[2]);
D_PRINTXY("SETMENU3 ",settings_MenuOptions[3]);
#endif

uint8_t menu_count=3;
settings_menu_pages=menu_count+1;


 //this is dependent on a mounted SDcard
{  menu_count+=1;
  settings_MenuOptions[menu_count]=sizeof (Settings5Menu) / sizeof (Settings5Menu[0]);
  D_PRINTXY("SETMENU5 ",settings_MenuOptions[menu_count]);
  settings_menu_pages=menu_count+1; //increase number of pages
} 

#ifdef USE_GPS
  menu_count+=1;
  settings_MenuOptions[menu_count]=sizeof (Settings4Menu) / sizeof (Settings4Menu[0]);
  D_PRINTXY("SETMENU4 ",settings_MenuOptions[menu_count]);
  settings_menu_pages=menu_count+1; //increase number of pages

#endif

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
  if ((j==3) and (Settings3Menu[i].active==true))
     { menucount++;}   
  if ((j==4) and (Settings5Menu[i].active==true))
     { menucount++;}
     
  
  #ifdef USE_GPS   
  if ((j==5) and (Settings4Menu[i].active==true))
     { menucount++;} 
  #endif     
 }  

 settings_MenuOptions[j]=menucount;
}
}

char * getMenuTxtVal(uint8_t menu_id, uint8_t update_page)
{  const uint8_t tlen=20;
   static char tstr[tlen];
   
   seconds2time(getRTC_TSR());
   switch (menu_id) {
      case SET_MENU_PAGE:
        snprintf(tstr,tlen, settings_page_name[update_page]);
        if ((update_page==4) and (!SD_ACTIVE))
         { snprintf(tstr,tlen, "NO SDCARD MOUNTED");

         }
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
      case SET_MENU_LOWDETECT:
        snprintf(tstr,tlen, "%04d", TE_peak*25);
      break;
      
      #ifdef USE_TEFACTOR
      case SET_MENU_TE_DETUNE:
        if (detune_factor!=100)
            {snprintf(tstr,tlen, "%03d", detune_factor);
            }
            else
            { snprintf(tstr,tlen, "OFF");

            }
      break;
      #endif
      
      case SET_MENU_SR:
        snprintf(tstr,tlen,"%s",SR[oper_SR].txt);
      break;

      case SET_MENU_SR_REC:
        snprintf(tstr,tlen,"%s",SR[rec_SR].txt);
        break;

      case SET_MENU_SR_PLY:
        if (play_SR>1)
           {snprintf(tstr,tlen,"1/%d",play_SR);
           }
        else
        {
          {snprintf(tstr,tlen,"%s","Direct"); 
           }
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
        //struct tm tx = seconds2time(getRTC_TSR());
        //show which part can be changed currently
        if (timemenu_pos==0)
            {snprintf(tstr,tlen, "[%02d]%02d", tm_hour, tm_min);
            }
        if (timemenu_pos>=1)
            {snprintf(tstr,tlen, "%02d[%02d]", tm_hour, tm_min);
            }
        break;
      case SET_MENU_DATE:
        //struct tm tx = seconds2time(getRTC_TSR());
       seconds2time(getRTC_TSR());
        if (timemenu_pos==0)
            {snprintf(tstr,tlen, "[%04d]%02d%02d", tm_year+1970, tm_mon, tm_mday);
            }
        if (timemenu_pos==1)
            {snprintf(tstr,tlen, "%04d[%02d]%02d", tm_year+1970, tm_mon, tm_mday);
            }
        if (timemenu_pos==2)
            {snprintf(tstr,tlen, "%04d%02d[%02d]", tm_year+1970, tm_mon, tm_mday);
            }
        break;


      case SET_MENU_GAIN:
         snprintf(tstr,tlen, "%03d", def_gain);
      break;
      
      case SET_MENU_VOL:
         snprintf(tstr,tlen, "%03d", def_vol);
      break;

      
        case SET_MENU_HIPASS:
        if (HI_pass>HI_PASS_OFF)
          {snprintf(tstr,tlen, "%02d kHz", HI_pass);
          }
          else
          {
            snprintf(tstr,tlen, "OFF");
          }
          
        break;
      

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
        snprintf(tstr,tlen, "%04d", AREC_S*25);
      break;
      case SET_MENU_AREC_BREAK:
        snprintf(tstr,tlen, "%03d", AREC_B);
      break;
      
      case SET_MENU_AREC_SR_REC:
        snprintf(tstr,tlen,"%s",SR[rec_SR].txt);
      break;

      case SET_MENU_COLORSCHEME:
        strncpy(tstr, colorscheme_name, 12);
        //snprintf(tstr,tlen, "%s", tstr );
      break;
    //  case SET_MENU_colorscheme_gamma_preset:
    //     snprintf(tstr,tlen, "%03d", colorscheme_gamma_preset);
    //     //snprintf(tstr,tlen, "%s", tstr );
    //   break;
         
      case SET_MENU_COLORS:
        snprintf(tstr,tlen, "%03d", cwheelpos[0] );
      break;

    
      case SET_MENU_HICOLORS:
        snprintf(tstr,tlen, "%03d",  cwheelpos[1]);
      break;
      case SET_MENU_BCKCOLORS:        
        snprintf(tstr,tlen, "%03d",  cwheelpos[2]);
      break; 
      #ifdef USE_PWMTFT     
        case SET_MENU_BCKLIGHT:        
          snprintf(tstr,tlen, "%03d",  tft_backlight);
        break;   
      #endif 
       case SET_MENU_TFTSLEEP:        
          if (tft_sleep) //setting is DISPLAY, so if sleep is true, display is OFF
              { snprintf(tstr,tlen, "OFF");
              }
          else
              {
                snprintf(tstr,tlen, "ON");
              }
        break;   

      
      case SET_MENU_ENC:
      if (ENCODER_TURN==1)
            { snprintf(tstr,tlen, "Clockw");
            }
          else
            {snprintf(tstr,tlen, "C-Clockw");
            }
      break;

      

         case SET_MENU_SD_SIZE:
          snprintf(tstr,tlen, "%5luMb", SD_SIZE );
         break;
         case SET_MENU_SD_USED:
          snprintf(tstr,tlen, "%5luMb", SD_USED );
         break;
         
        case SET_MENU_SD_PLAYFOLDER:
          D_PRINTLN("PLAYFOLDER")
          dir.open("/");
          file.open(&dir,dirindex[playfolder], O_RDONLY);
          char dirname[80];
          file.getName(dirname,80);
          file.close();
          if (memcmp(active_dir,dirname,8)==0)
            {    snprintf(tstr,16,"[%s]",dirname);
            }
          else
            {    snprintf(tstr,16,"%s",dirname);
            }
          dir.close(); 
        
        break;

         case SET_MENU_SD_WAV:
            snprintf(tstr,tlen, "%4d", filecounter );
         break; 

         case SET_MENU_SD_RECFOLDER:
          D_PRINTLN("RECFOLDER")
          snprintf(tstr,tlen, "Disabled");
         break;

         case SET_MENU_SD_FORMAT:
          snprintf(tstr,tlen, "Disabled");
         break;
         
          
      

      
      #ifdef USE_GPS
        case SET_MENU_GPS_LAT: //degrees
              snprintf(tstr,tlen, "%.6f", float(gps_latitude)/1000000 );
          break;
        case SET_MENU_GPS_LON: //degrees
              snprintf(tstr,tlen, "%.6f", float(gps_longitude)/1000000 );
          break;
        case SET_MENU_GPS_ALT: //meters
              snprintf(tstr,tlen, "%.2f", float(gps_altitude)/100 );
          break;
        case SET_MENU_GPS_HDOP: //meters
              snprintf(tstr,tlen, "%.2f", float(gps_HDOP)/100 );
          break;
        case SET_MENU_GPS_FIXAGE://ms
              snprintf(tstr,tlen, "%.3f", float(gps_fixage)/1000 );
          break;
        
        case SET_MENU_GPS_FAILS:
              snprintf(tstr,tlen, "%d", gps_failed_cs);
          break;
        case SET_MENU_GPS_SENTENCE:
              snprintf(tstr,tlen, "%d", gps_sentences);
          break;

      #endif
           
      
      #ifdef ADVANCEDMENU            
          
          case SET_MENU_INPUT: 
          if (myInput==AUDIO_INPUT_LINEIN)
              {snprintf(tstr,tlen, "LINE_IN");
                }
           if (myInput==AUDIO_INPUT_MIC)
              {snprintf(tstr,tlen, "MIC_IN");
                  };
           if (myInput==AUDIO_INPUT_ADC)
              {snprintf(tstr,tlen, "ADC_IN");
                  };
                  
          break;
          
                    
      #endif 

    default:
    // statements
    break;
  }


 return tstr;
}

void menuAction(uint8_t menu_id, int8_t change )
//act on changes for a menu_setting
{
  D_PRINT(" MENU ACTION ")
  D_PRINTLN(menu_id)
  /******************************VOLUME  ***************/
      if (menu_id==MENU_VOL)
        { volume+=change;
          volume=constrain(volume,0,90); //not cyclic
          set_vol(volume);
        }
   /******************************MAIN SR   ***************/
      if ((menu_id==MENU_SR) and (LeftButton_Mode!=MODE_PLAY))  //selects a possible SR but only if we are not in the playing mode
        { D_PRINT(" CHANGE OPER_SR ")
  
          oper_SR+=change;
          oper_SR=constrain(oper_SR,SR_MIN,SR_MAX); //not cyclic
          set_SR(oper_SR);
        }
     /******************************MIC_GAIN  ***************/
      if (menu_id==MENU_GAIN_MIC)
        {
         mic_gain+=change;
         mic_gain=constrain(mic_gain,0,63);
         set_mic(mic_gain);
     
        }     
       if (menu_id==MENU_DISPLAY)
            { display_mode=cyclic_constrain(display_mode,1,no_graph,waterfallgraph);
            }

  /******************************FREQUENCY  ***************/
      if (menu_id==MENU_FRQ)
         { int delta=500;
          if (detector_mode==detector_heterodyne) //only allow manual change in heterodyne mode
           {uint32_t currentmillis=millis();
              //when turning the encoder faster make the changes larger
              if ((currentmillis-lastmillis)<500)
                  { delta=1000;}
              if ((currentmillis-lastmillis)<250)
                  { delta=2000;}
              if ((currentmillis-lastmillis)<100)
                  { delta=5000;}

              osc_frequency=osc_frequency+delta*change;
              // limit the frequency to 500hz steps
              osc_frequency=constrain(osc_frequency,7000,int(SR_real/2000)*1000-1000);
              last_osc_frequency=osc_frequency; //always backup the F setting
              set_freq_Oscillator (osc_frequency);
              lastmillis=millis();
           }
         }
     
          if (menu_id==MENU_HIP)
          {
                HI_pass+=change*2;
                HI_pass=constrain(HI_pass,HI_PASS_OFF,50); 
                //set_SR(oper_SR);
                setHiPass();
          
          }
      
    
    //  #ifdef USE_TEFACTOR
    //   if (menu_id==MENU_TEFACTOR)
    //     { 
    //       detune_factor=cyclic_constrain(detune_factor,change,50,100);
    //       if (detune_factor==100)
    //          set_OutputMixer(granularmixer);
    //       else
    //          set_OutputMixer(granularHTmixer);
                   
    //     }
    //  #endif          
}

void settingsMenuAction(uint8_t set_menu_id, int8_t change)
{ 
           D_PRINT(" SETMENU ACTION ")
           D_PRINTLN(set_menu_id) 
          if (set_menu_id==SET_MENU_FD_DIV)
            { //TE_low+=change;
              FD_divider=cyclic_constrain(FD_divider,change,4,16); //4,8,16,32,64
              granular1.setdivider(FD_divider);
             }

            if (set_menu_id==SET_MENU_LOWDETECT)
            {   
               TE_peak=cyclic_constrain(TE_peak,change,0,200); // peak is the user-set value. The user will see peak*25
                
             }
           
           #ifdef USE_TEFACTOR
           if (set_menu_id==SET_MENU_TE_DETUNE)
            { //TE_low+=change;
              detune_factor=cyclic_constrain(detune_factor,change,50,100); 
              D_PRINTXY("TE FACTOR", detune_factor)
              if (detune_factor==100)
                set_OutputMixer(granularmixer);
              else
                set_OutputMixer(granularHTmixer);
             }
               
           #endif   
          //time expansion duration
          if (set_menu_id==SET_MENU_TE_GAP)
            { //TE_low+=change;
              TE_GAP=cyclic_constrain(TE_GAP,change*5,15,200);
             }
              
          //time expansion lowest frequency
          if (set_menu_id==SET_MENU_TE_LOW)
            { //TE_low+=change;
              TE_low=cyclic_constrain(TE_low,change*2,0,35);
              signal_LoF_bin= int((TE_low*1000.0)/(SR_FFTratio));
              }
          //time expansion replay speed   
          if (set_menu_id==SET_MENU_TE_SPD)
            {  //TE_speed+=change;
               TE_speed=cyclic_constrain(TE_speed,change, 5,30);
               granular1.setTESpeed(TE_speed); // set TE speed
             }
          //operational sample rate   
          if (set_menu_id==SET_MENU_SR)
            { oper_SR+=change;
              oper_SR=constrain(oper_SR,SR_MIN,SR_MAX);
              set_SR(oper_SR);
             }   
           if (set_menu_id==SET_MENU_GAIN)
          {     def_gain+=change;
                def_gain=constrain(def_gain,0,63); 
                //set_SR(oper_SR);
                mic_gain=def_gain;
                set_mic(mic_gain);
            }   
            if (set_menu_id==SET_MENU_VOL)
          {     def_vol+=change;
                def_vol=constrain(def_vol,0,90); 
                //set_SR(oper_SR);
                volume=def_vol;
                set_vol(volume);
            }    
        
          if (set_menu_id==SET_MENU_HIPASS)
          {     HI_pass+=change*2;
                HI_pass=constrain(HI_pass,HI_PASS_OFF,50); 
                //set_SR(oper_SR);
                setHiPass();
            }
       
                      //default display mode   
          if (set_menu_id==SET_MENU_DISPLAY)
            { //startup_display+=change;
              startup_display=cyclic_constrain(startup_display,change, 0,2); //allowed settings
             }   
          //default detector mode
          if (set_menu_id==SET_MENU_DETECTOR)
            { //startup_detector+=change;
              startup_detector=cyclic_constrain(startup_detector,change,0,4); //allowed settings
             }      
          if (set_menu_id==SET_MENU_REC_DETECT)
            { //startup_detector+=change;
              record_detector=cyclic_constrain(record_detector,change,0,detector_Auto_heterodyne); //allowed settings
             }    
              
          //default sampleRate for replay
          if (set_menu_id==SET_MENU_SR_PLY)
            {  if (play_SR==1)
                 {play_SR=MAX_play_SR;}

              play_SR+=change;   
              play_SR=constrain(play_SR,5,MAX_play_SR);
              
              if (play_SR>=MAX_play_SR)
                 {play_SR=1;} //switch to DIRECTPLAY

            }
          //default sampleRate for recording  
          if (set_menu_id==SET_MENU_SR_REC)
            { //rec_SR+=change;
              rec_SR=cyclic_constrain(rec_SR,change, SR_44K,SR_MAX);
            }

          if (set_menu_id==SET_MENU_AREC_DURATION)
              {  
                 AREC_D=cyclic_constrain(AREC_D,change,1,60); //5-300sec
              }

            if (set_menu_id==SET_MENU_AREC_PAUSE)
              {  
                AREC_P=cyclic_constrain(AREC_P,change,0,60); //0-300sec
              }
            if (set_menu_id==SET_MENU_AREC_BREAK)
              {  
                AREC_B=cyclic_constrain(AREC_B,change,1,30); //1-30sec
              }
              
            //default sampleRate for recording  
            if (set_menu_id==SET_MENU_AREC_SR_REC)
            {
              rec_SR=cyclic_constrain(rec_SR,change,SR_44K,SR_MAX);
            }
            if (set_menu_id==SET_MENU_AREC_S)
              {  
                 AREC_S=cyclic_constrain(AREC_S,change, 1,200);
              }
            if (set_menu_id==SET_MENU_AREC_F)
              {  
                 AREC_F=cyclic_constrain(AREC_F,change*2,0,80); //20210428
              } 
            
            #ifdef USE_PWMTFT
             if (set_menu_id==SET_MENU_BCKLIGHT)
              {  tft_backlight=tft_backlight/5*5;
                 tft_backlight=cyclic_constrain(tft_backlight,change*5,0,255);
                 set_backlight(tft_backlight);
              } 
            #endif  


            if (set_menu_id==SET_MENU_TFTSLEEP)
               {
                 tft_sleep=!(tft_sleep);
               }
              

            //reserved for later use
            if (SD_ACTIVE) //disable when not active
            {
            //reserved for later use
            if (set_menu_id==SET_MENU_SD_FORMAT) //FORMAT AN SD
               {
                //
               }
            
             if (set_menu_id==SET_MENU_SD_PLAYFOLDER)
               {
                 playfolder=cyclic_constrain(playfolder,change,0,dircounter-1);
                 countFilesinDir_byindex(dirindex[playfolder]);
                          
               }
             //reserved for later use  
             if (set_menu_id==SET_MENU_SD_RECFOLDER)
               {
                 //
               }
            }

            if (set_menu_id==SET_MENU_COLORSCHEME)
               {
                 colorscheme=cyclic_constrain(colorscheme,change,0,max_colorscheme);
                 colorschemegamma_set=1000;
                 setcolourmap(colorscheme);
               }  

            // if (set_menu_id==SET_MENU_colorscheme_gamma_preset) //removed before V1_2 release !
            //    { 
            //      colorschemegamma_set=cyclic_constrain(colorschemegamma_set,change*100,100,5000);
            //      setcolourmap(colorscheme);
            //    } 

 //encoder direction
          if (set_menu_id==SET_MENU_ENC)
            { ENCODER_TURN=ENCODER_TURN*-1; //reverse
              enc_dn=ENCODER_TURN*-1;
              enc_up=ENCODER_TURN;
             } 



#ifdef ADVANCEDMENU
            if (set_menu_id==SET_MENU_INPUT)
              { 
                #ifdef USE_ADC_IN
                   myInput=cyclic_constrain(myInput,change,0,2);   
                #else
                   myInput=cyclic_constrain(myInput,change,0,1);   
                #endif  
                if ((myInput==AUDIO_INPUT_LINEIN) | (myInput==AUDIO_INPUT_MIC))
                     { set_InputMixer(in_mic);
                       AudioNoInterrupts();
                       sgtl5000.inputSelect(myInput);
                       set_mic(mic_gain);
                       AudioInterrupts();
                       
                       }
                if (myInput==AUDIO_INPUT_ADC)
                    { set_InputMixer(in_adc);
                      set_mic(mic_gain);
                    }


              }
#endif      

}

#endif