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

#ifndef _BAT_TFT_H
#define _BAT_TFT_H

#include "bat_vars.h"
#include "bat_sd.h"
#include "bat_menu.h"
#include "bat_fft.h"

#define ILI9341 //currently only created to be used with ILI9341
#ifdef ILI9341
  #include "ILI9341_t3.h"
  #include "font_Arial.h"
  #define TFT_FONT Arial_16 
  #define MENU_FONT Arial_14
  
 //see https://www.pjrc.com/store/display_ili9341_touch.html 

  #define BACKLIGHT_PIN 255 //backlight is controlled by hardware 
  #define TFT_DC      20
  #define TFT_CS      21
  #define TFT_RST     255  // 255 = unused. connect to 3.3V
  #define TFT_MOSI    28
  #define TFT_SCLK    27
  #define TFT_MISO    39

  ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

  //#define Touch_CS    8
  //XPT2046_Touchscreen ts = XPT2046_Touchscreen(Touch_CS);
  //predefine menu background etc colors

//setup of the SCREEN BLOCKS for a 240wide and 320high TFT
  //**********************
  // SETTINGS AND TIME   *
  //                     *
  // POWERGRAPH          * <-----TOPOFFSET 
  //                     *
  //       GRAPH         * 
  //                     *
  // PUSHBUTTON SETTING  * <-----BOTTOMOFFSET
  // ENCODER SETTING     *
 //***********************

  //the powergraph is show with a spectrumscale on the bottom and a graph above
  // with the default settings the powergraph starts at y=90-45-5 and is 45 pixels hight
  // the scale starts at y=90-5 and is 5 pixels high
  #define TOP_OFFSET 70 //lower position of the graph ex 90
  #define POWERGRAPH 40 //height of the graph         ex 45
  #define SPECTRUMSCALE 8 //height of the scale       ex 5
  
  #define BOTTOM_OFFSET 40      // split the BOTTOMOFFSET into 2 sections, this is the highest 
  #define BOTTOM_OFFSET_PART BOTTOM_OFFSET/2 // this is the lowest

  //#define LINE2 BOTTOM_OFFSET  // allows switch of menuline positions
  //#define LINE1 BOTTOM_OFFSET_PART
    
  //default menu colors

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value, byte mod)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red
  
  byte red = 0;         // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;        // Green is the middle 6 bits
  byte blue = 0;         // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
          blue = 31;
          green = 2 * (value % 32);
          red = 0;
  }
  if (quadrant == 1) {
          blue = 31 - (value % 32);
          green = 63;
          red = 0;
  }
  if (quadrant == 2) {
          blue = 0;
          green = 63;
          red = value % 32;
  }
  if (quadrant == 3) {
          blue = 0;
          green = 63 - 2 * (value % 32);
          red = 31;
  }
 
  
  if (mod!=2)
   {
    red=byte(red*2/4);
    green=byte(green*2/4);
    blue=byte(blue*2/4);
   }

  return (red << 11) + (green << 5) + blue;

}

uint16_t ENC_MENU_COLOR= COLOR_YELLOW;
uint16_t HILIGHT_MENU_COLOR= COLOR_WHITE;
uint16_t BCK_MENU_COLOR= COLOR_DARKRED;

#define ENC_VALUE_COLOR COLOR_LIGHTGREY
 // #define MENU_BCK_COLOR COLOR_DARKRED
#define MENU_BCK_COLOR_PART COLOR_DARKGREEN

int calc_menu_dxoffset(const char* str)  // to position the menu on the right screenedge
{

  String s=String(str); 
  char charBuf[50];
  s.toCharArray(charBuf, 50) ;
  uint16_t dx=tft.strPixelLen(charBuf);
  return dx; 

}

void txt_rightAlign(const char *s, int16_t y)
{
      uint16_t dx=calc_menu_dxoffset(s); 
      tft.setCursor(ILI9341_TFTWIDTH-dx,y);
      tft.print(s);
}

void show_next_settings_page(int change)
{
  settings_page_nr+=change;
  settings_page_nr=cyclic_constrain(settings_page_nr,0,0,settings_menu_pages-1);
  tft.fillRect(0,45,ILI9341_TFTWIDTH,ILI9341_TFTHEIGHT-45-40,COLOR_BLACK);
}

void drawScale()
{
     if (display_mode!=settings_page)
      {   
        if (display_mode==spectrumgraph)
           { tft.fillRect(0,TOP_OFFSET-(SPECTRUMSCALE+1),ILI9341_TFTWIDTH,SPECTRUMSCALE+1,COLOR_BLACK);
           }
        if ((display_mode==waterfallgraph)) 
          { tft.fillRect(0,TOP_OFFSET-(SPECTRUMSCALE+1),ILI9341_TFTWIDTH,SPECTRUMSCALE+1,COLOR_BLACK);
          }   

        float x_factor=10000/(0.5*(SR_real/FFT_points));
        int maxScale=int(SR_real/20000);
        #ifdef USE_HIPASS_FILTER
        if (HI_pass>HI_PASS_OFF)
           { tft.fillRect(0,TOP_OFFSET-SPECTRUMSCALE+1,uint(HI_pass*x_factor*0.1),4,COLOR_DARKGRAY);
             tft.setFont(MENU_FONT); 
             tft.setCursor(0,TOP_OFFSET-SPECTRUMSCALE-MENU_FONT.cap_height);
             tft.print("*hi");
           }
        #endif   
        for (int i=1; i<maxScale; i++)
        { tft.drawFastVLine(i*x_factor, TOP_OFFSET-SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
          tft.drawFastVLine(i*x_factor+1, TOP_OFFSET-SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
          }
      }    
       
}

void drawScaleMarker() //draws a filled circle on the scale
{
    if (display_mode!=settings_page)
     { 
      //float f=SR_real/FFT_points; 
      int curF=int(osc_frequency/SR_FFTratio);
       
      if (curF<ILI9341_TFTWIDTH-4)
          {//tft.fillRect(curF-1,TOP_OFFSET-10,3,10,HILIGHT_MENU_COLOR);
           //tft.fillCircle(curF,TOP_OFFSET-5,3,HILIGHT_MENU_COLOR);
           tft.fillTriangle(curF-3,TOP_OFFSET-SPECTRUMSCALE,curF+3,TOP_OFFSET-SPECTRUMSCALE,curF,TOP_OFFSET-1,HILIGHT_MENU_COLOR);
          //  if (display_mode==spectrumgraph) //with text
          //   { char tstr[9];
              // snprintf(tstr,9, "%02d", int(osc_frequency/1000));
              // tft.setTextColor(HILIGHT_MENU_COLOR);
              // tft.setCursor(curF-10,TOP_OFFSET-20);  
              // tft.print(tstr);
          //   }
          }
     }    
     int curF=signal_LoF_bin;
     tft.fillTriangle(curF-3,TOP_OFFSET-SPECTRUMSCALE,curF+3,TOP_OFFSET-SPECTRUMSCALE,curF,TOP_OFFSET-SPECTRUMSCALE-7, COLOR_WHITE);

}

void headerbar(uint x, uint height, uint mxheight)
{
  for (uint i=0; i<height; i++) 
  {if (i%2>0)
         {tft.drawFastHLine(x,mxheight-i,10,ENC_MENU_COLOR);
         }
  }

}

void showTime()
{     char tstr[9];
      
      struct tm tx = seconds2time(RTC_TSR);
      tft.setFont(MENU_FONT);
      snprintf(tstr,9, "%02d:%02d", tx.tm_hour, tx.tm_min);
      uint dx=calc_menu_dxoffset(tstr);
      tft.fillRect(ILI9341_TFTWIDTH-dx,0,dx,MENU_FONT.cap_height+2,BCK_MENU_COLOR);
      tft.setCursor(ILI9341_TFTWIDTH-dx,1);
      tft.setTextColor(ENC_MENU_COLOR);
      tft.print(tstr);
    
}     

void updateTime()
{
  if ((timeupdate>10000))
   { showTime();
     timeupdate=0;
    }
}

void showHeader()
{   tft.setFont(TFT_FONT);
  //first clear the headerblock
    uint headheight=TFT_FONT.cap_height+2;
    tft.fillRect(0,0,ILI9341_TFTWIDTH,headheight,BCK_MENU_COLOR);
    //position cursor and display important settings
    tft.setCursor(0,1);
    tft.setTextColor(ENC_MENU_COLOR);   
    tft.print("V"); 
    uint x=tft.getCursorX();
    headerbar(x+2,uint(volume*20/90),headheight);
    tft.setCursor(x+15,1);
    tft.print("G"); 
    x=tft.getCursorX();
    headerbar(x+2,uint(mic_gain*20/64),headheight);
    tft.setCursor(x+40,1);
    tft.print(SRtext);
    tft.print('k');
    
    showTime();
    // if ((detector_mode==detector_heterodyne) and (display_mode==waterfallgraph))
    //     {tft.print(" f");  tft.print(int(osc_frequency/1000));
    //     }
    // else
    // {
    //   if (HI_pass>HI_PASS_OFF)
    //     {tft.print(" hi");
    //      tft.print(HI_pass);}   
    // }
        
       

}

void showNOSD()
{ if ((!SD_ACTIVE))
    {char tstr[9];
        tft.setFont(Arial_13);
        snprintf(tstr,9, "NO SD");
        tft.setCursor(ILI9341_TFTWIDTH-tft.strPixelLen(tstr),20);
        tft.setTextColor(HILIGHT_MENU_COLOR);
        tft.print(tstr);
    }
}


uint8_t menugap=0;
void  showMenuLine(const char* menu_txt, const char* tstr, uint8_t line, uint8_t update_page, bool reverse)
{
  int h = SET_MENU_FONT.cap_height+ 3;
  tft.setCursor(0,TOP_OFFSET-POWERGRAPH+line*h+menugap);
  
  if (reverse==true) // this menuline has the cursor, show reverse
            { tft.setTextColor(COLOR_BLACK);
              tft.fillRect(0,TOP_OFFSET-POWERGRAPH+line*h+menugap,ILI9341_TFTWIDTH,h-2,HILIGHT_MENU_COLOR);
            }
          else //normal menu_entry
          { tft.setTextColor(ENC_MENU_COLOR);
            tft.fillRect(0,TOP_OFFSET-POWERGRAPH+line*h+menugap,ILI9341_TFTWIDTH,h-2,COLOR_BLACK);
          }
        
  if (menu_txt!=Settings0Menu[0].menu_txt) //check if we are updating the page_header or a normal menu entry  
    { //normal menu
     tft.print(menu_txt); //show stored menutext
     txt_rightAlign(tstr, tft.getCursorY());
    
    }
  else // header, so dont show the settings_menu.menu_txt part but only the pagename
    {
      { uint16_t t_col=ENC_MENU_COLOR;
        if (line!=set_menu_pos[update_page])
          { t_col=BCK_MENU_COLOR;
          }
        tft.fillRect(0,TOP_OFFSET-POWERGRAPH-SPECTRUMSCALE+line*h,ILI9341_TFTWIDTH,h+4,t_col);//use default backgroundcolor for the header
        tft.setCursor(0,TOP_OFFSET-POWERGRAPH-SPECTRUMSCALE+2+line*h);
        tft.print(tstr); //show menutext
        
      }
    }
}

void update_menu_value(uint8_t menu_id, const char* menu_txt, uint8_t update_page, uint8_t line)
{ 
  const uint8_t tlen=11;
  char tstr[tlen];
  snprintf(tstr,tlen, getMenuTxtVal(menu_id,update_page)); //build up the text for value based on the variable
  //now show the menu entry at the line and reversed if necessary  
  bool reverse=(line==set_menu_pos[update_page]);
  if (menu_txt[0]=='>')
     { menugap+=6; //add 6 pixel gap
       menu_txt = &menu_txt[1];  
      }
  showMenuLine(menu_txt, tstr, line, update_page, reverse);

}

void showSettings()
{
        tft.setTextColor(ENC_MENU_COLOR); //default color
        tft.setFont(SET_MENU_FONT); //default font allows 12 lines of settings
        //for each of the settings pages build the menu in order of the settings page array
        menugap=0;
        for (int i=0; i<settings_MenuOptions[settings_page_nr]; i++)
              { //push the menu item, the pagenr and the screenline we are updating
                if (settings_page_nr==0)
                   {update_menu_value(Settings0Menu[i].menu_id,Settings0Menu[i].menu_txt,settings_page_nr,i); //update line in menu
                   }
                 if (settings_page_nr==1)
                   {update_menu_value(Settings1Menu[i].menu_id,Settings1Menu[i].menu_txt,settings_page_nr,i); //update line in menu
                   }  
                 if (settings_page_nr==2)
                   {update_menu_value(Settings2Menu[i].menu_id,Settings2Menu[i].menu_txt,settings_page_nr,i); //update line in menu
                   }
              }
        
}

#include "bat_audio.h" //myFFT structure

void spectrum() 
{ // spectrum analyser code by rheslip - modified

  uint16_t OFFSET =ILI9341_TFTHEIGHT-BOTTOM_OFFSET-TOP_OFFSET;  //default 320 - 40 - 90 = 190
  //int16_t FFT_sbin [128];
  // limit until bin 119 to prevent overflow
  uint16_t barnew=0;
  uint16_t bar;
  uint16_t barm=0;
  //uint8_t g_x;
  uint8_t spectrumline;
  uint8_t spectrumline_new;
  uint16_t barg[120];
  uint16_t barg_new[120];
  
  for (int16_t x = 2; x < 119; x++) {
     //FFT_sbin[x] = int(myFFT.output[x]);//-FFTavg[x]*0.9;
     //int barnew = (FFT_sbin[x]) ; 
     barnew=myFFT.output[x];
     // this is a very simple first order IIR filter to smooth the reaction of the peak values of the bars
     bar = (2 * barnew + 8 * barm)/10;
     barm=bar;
     barg[x]=constrain(bar,0,OFFSET-1);
     barg_new[x]=constrain(barnew,0,OFFSET-1);
     
  }
   
   for (int16_t x = 2; x < 119; x++) {
     spectrumline=barg[x];
     spectrumline_new=barg_new[x];
     uint8_t len=30;
     int16_t start=spectrumline_new-len;
     if (start<0) { start=0;}
     tft.drawFastVLine(x*2,TOP_OFFSET,start,COLOR_ORANGE);
     tft.drawFastVLine(x*2,TOP_OFFSET+start,len,COLOR_YELLOW); // X, 90, length bar, colour 
     tft.drawFastVLine(x*2,TOP_OFFSET+spectrumline_new,OFFSET-spectrumline_new, COLOR_BLACK); // X, 90, length bar, colour
  
      if (spectrumline>4) 
         {tft.drawFastVLine(x*2,TOP_OFFSET+spectrumline-5,5, COLOR_WHITE); //draw the moving average peak-hold
         }
        
  }
}

void showPowerSpectrum()
{
        powerspectrumCounter=0;
         float bin2frequency=SR_FFTratio*0.001;
         //clear powerspectrumbox
         tft.fillRect(0,TOP_OFFSET-POWERGRAPH-SPECTRUMSCALE,ILI9341_TFTWIDTH,POWERGRAPH, COLOR_BLACK);
         
         // keep a minimum maximumvalue to the powerspectrum
         float powerSpectrum25=(powerspectrum_Max+1)*0.25;

         //prevent div by zero errors !!
         //find the nearest frequencies below 10% of the maximum to show on screen as values
         
          //draw spectrumgraph
          uint16_t spectrumcolor=COLOR_RED;
          boolean peakFound=false;
          uint lopeak=0;
          uint hipeak=0;
          for (uint i=signal_LoF_bin; i<spec_hi; i++)
            { int ypos=FFTpowerspectrum[i]/powerspectrum_Max*POWERGRAPH;
              if (FFTpowerspectrum[i]>powerSpectrum25)
                 { if (lopeak==0)
                         {lopeak=i;}
                  spectrumcolor=COLOR_ORANGE; //range (>25%of peakvalue) values
                  peakFound=false;
                  hipeak=i;
                  if (i==powerSpectrum_Maxbin)
                     {spectrumcolor=COLOR_WHITE; //peak values
                      peakFound=true;
                      }
                 }
              else
                 {spectrumcolor=COLOR_RED; //normal values outside range/peak
                   }   

              tft.drawFastVLine((i-spec_lo),TOP_OFFSET-ypos-SPECTRUMSCALE,ypos,spectrumcolor);
                            
              //reset powerspectrum for next samples
              FFTpowerspectrum[i]=0;
            }
          
          if (powerSpectrum_Maxbin!=0)
          { 
            char tstr[12];
            tft.setFont(Arial_13);
            snprintf(tstr,12, "%3d %3d %3d", int(lopeak*bin2frequency),int(powerSpectrum_Maxbin*bin2frequency),int(hipeak*bin2frequency));
            txt_rightAlign(tstr,TOP_OFFSET-SPECTRUMSCALE-Arial_13.cap_height-2);

          }
          powerspectrum_Max=powerspectrum_Max*0.5; //lower the max after a graphupdate
          showNOSD();

}




void showresettoDefault()
{
       tft.setRotation( 0 );
       tft.setTextColor(COLOR_WHITE);
       tft.fillScreen(COLOR_BLACK);
       tft.setCursor(0,50);
       tft.setFont(Arial_20);
       tft.print("     RESETTING ");
       tft.print("");
       tft.print("    TO   DEFAULTS ");
       delay(5000);
}
void StartupScreen()
  {
  tft.setRotation( 0 );
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  
  tft.setCursor(80,50);
  tft.setFont(Arial_20);
  char tstr[9];
  struct tm tx = seconds2time(RTC_TSR);
  snprintf(tstr,9, "%02d:%02d:%02d", tx.tm_hour, tx.tm_min, tx.tm_sec);
  tft.print(tstr);
  tft.setCursor(0,90);
  tft.println("Teensy Batdetector");
  
  tft.setCursor(0,120);
  tft.println(versionStr);
  tft.println();
  tft.setFont(TFT_FONT);
  snprintf(tstr,9, "%9lX",chipNum[2]);  
  tft.print("Teensy SN:");
  tft.println(tstr);
  #ifdef USE_EEPROM
  tft.print("EEprom  v:");
  snprintf(tstr,9,"%s",versionEE);
  //tft.println(String(versionEE));
  tft.println(tstr);
  tft.print("  Saved #:");
  tft.println(EEsaved_count);
  if (use_presets==1)
  {tft.println("  Settings:USER");
  }
  else
  {tft.println("  Settings:DEFAULT");
    }
      
  #endif
}

void showEncoders()
{
     tft.setFont(MENU_FONT);
     // **************************** DISPLAY ENCODER FUNCTIONS/VALUE
     tft.setCursor(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART+2); //display of encoder functions
       
     // set the colors according to the function of the encoders when NOT in the settings_page
     if ((display_mode!=settings_page) and (AUTO_REC==false))
      {
      /****************** LEFT SIDE ENCODER ***********/        

       uint16_t f_col=COLOR_BLACK;
       uint16_t b_col=HILIGHT_MENU_COLOR;

       if (EncLeft_function!=enc_value)
        { f_col=ENC_MENU_COLOR;
          b_col=COLOR_BLACK;
          if (EncLeftchange!=0)
           {f_col=HILIGHT_MENU_COLOR;
             } 
         }
       
       uint mxLstr=12;
       char ltstr[mxLstr];
    
       //playing a file
       if (((LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_PLAY)) and (EncLeft_function==enc_value)) // when play selected only show filename
           { 
             //test if filename is HEX_ENCODED datetime
             unsigned long seconds= strtol(strlwr(filelist[fileselect]), NULL, 16);
             if (seconds> 1577836800) //1 jan 2020
               {
                 struct tm tx = seconds2time(seconds);
                 D_PRINTXY(" secs",seconds);
                 snprintf(ltstr,mxLstr,"%02d%02d %02d%02d%02d", tx.tm_mon, tx.tm_mday, tx.tm_hour, tx.tm_min, tx.tm_sec); 
                 D_PRINTXY("%s", ltstr);
                 b_col=COLOR_YELLOW;
                 f_col=COLOR_BLACK;
                }                      
             else
             {
               snprintf(ltstr,mxLstr,"%s",strlwr(filelist[fileselect])); 
             }
           
             
            }
          else 
          { snprintf(ltstr,mxLstr,"%s",LeftBaseMenu[EncLeft_menu_idx].menu_txt); 
            
          }

      //LEFTENCODER ON REC SETTING
       if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_REC) 
        {  if (EncLeft_function==enc_value)  
              {snprintf(ltstr,mxLstr,"BACK ^");
              }
         else //MENUMODE
            {if (FFORMAT==FFORMAT_RAW)
              {snprintf(ltstr,mxLstr,"REC(RAW)");
                }
            else
               {snprintf(ltstr,mxLstr,"REC(WAV)");
                 }
             } 
        }


       if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_GAIN_MIC)
          { snprintf(ltstr,mxLstr,"%s %d",ltstr,mic_gain);
          } 

       if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_FRQ)
          { if (detector_mode==detector_heterodyne)
            { snprintf(ltstr,mxLstr,"%s %d",ltstr,osc_frequency/1000);
            }
            else
            {
              snprintf(ltstr,mxLstr,"%s AUTO",ltstr);
              f_col=ENC_MENU_COLOR;
              b_col=COLOR_BLACK;
              EncLeft_function=enc_menu;
              
            }
            
            
           } 
       if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_VOL)
          { snprintf(ltstr,mxLstr,"%s %d",ltstr,volume);
          }


       if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_SR)
          { if (LeftButton_Mode==MODE_PLAY)
                {if (play_SR<MAX_play_SR)
                  {snprintf(ltstr,mxLstr,"%s %s",ltstr, SR[play_SR].txt);
                  }
                  else
                  {
                   snprintf(ltstr,mxLstr,"%s %s",ltstr, "DIRECT");
                  }
                  
                }
             else
                {snprintf(ltstr,mxLstr,"%s %s",ltstr, SR[oper_SR].txt);
                 }
          }    
       #ifdef USE_HIPASS_FILTER
         if (LeftBaseMenu[EncLeft_menu_idx].menu_id==MENU_HIP)
            { if (HI_pass>HI_PASS_OFF)
                {snprintf(ltstr,mxLstr,"%s %d",ltstr, HI_pass);
                }
              else
              {
                snprintf(ltstr,mxLstr,"%s OFF",ltstr);
              }
             
            }
       #endif   
       tft.setTextColor(f_col);
       tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART,ILI9341_TFTWIDTH/2-2, BOTTOM_OFFSET_PART,b_col);
       tft.print(ltstr);
         

/****************** RIGHT SIDE ENCODER ***********/
       f_col=COLOR_BLACK;
       b_col=HILIGHT_MENU_COLOR;

       if (EncRight_function!=enc_value)
        { f_col=ENC_MENU_COLOR;
          b_col=COLOR_BLACK;
          if (EncRightchange!=0)
           {f_col=HILIGHT_MENU_COLOR;
             } 
         }

      
       uint mxRstr=12;       
       char rtstr[mxRstr];
       if (RightBaseMenu[EncRight_menu_idx].menu_id==MENU_GAIN_MIC)
          {  snprintf(rtstr,mxRstr, "%s %d", RightBaseMenu[EncRight_menu_idx].menu_txt, mic_gain);
          } 

       if (RightBaseMenu[EncRight_menu_idx].menu_id==MENU_FRQ)
          { if (detector_mode==detector_heterodyne)
              {snprintf(rtstr,mxRstr, "%s %d", RightBaseMenu[EncRight_menu_idx].menu_txt, int(osc_frequency/1000)); 
              }
            else
            {
              snprintf(rtstr,mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, "AUTO");
              f_col=ENC_MENU_COLOR;
              b_col=COLOR_BLACK;
              EncRight_function=enc_menu;
            }
              
          } 

       if (RightBaseMenu[EncRight_menu_idx].menu_id==MENU_VOL)
          {  snprintf(rtstr,mxRstr, "%s %d", RightBaseMenu[EncRight_menu_idx].menu_txt, volume);
          }

       if (RightBaseMenu[EncRight_menu_idx].menu_id==MENU_SR)
          { if (LeftButton_Mode==MODE_PLAY)
                {if (play_SR<MAX_play_SR)
                  {snprintf(rtstr,mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, SR[play_SR].txt);
                  }
                  else
                  {
                   snprintf(rtstr,mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, "DIRECT");
                  }
                  
                  
                }
             else
                { snprintf(rtstr,mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, SR[oper_SR].txt);
                 } 
          
          }  
       tft.setTextColor(f_col);
       tft.fillRect(ILI9341_TFTWIDTH/2,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART,ILI9341_TFTWIDTH/2, BOTTOM_OFFSET_PART, b_col);      
       txt_rightAlign(rtstr,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART+2);
       
       
       } // END of regular SETTINGS
}

void showPushButtons()
{   //clear the line
    tft.setCursor(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET); //position of button functions
    tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET,ILI9341_TFTWIDTH,BOTTOM_OFFSET_PART,BCK_MENU_COLOR);

    tft.setTextColor(ENC_MENU_COLOR);

    //on the left 
    if (LeftButton_Mode==MODE_DISPLAY)
        { tft.print(setDisplay[display_mode]);
        }  

    if (LeftButton_Mode==MODE_REC)
        {    
           if (recorderActive==false)  
            {   tft.print("Start REC");
                }
            else
            {   tft.print("Stop REC");
            }
           
       }

    if (LeftButton_Mode==MODE_PLAY)
        { if (playActive==false) 
            { tft.print("PLAY");
            }
            else
            { tft.print("STOP");
            }
            }
    

    //on the right
    if (not recorderActive)
    {
    tft.setTextColor(ENC_MENU_COLOR);
    if ((playActive==false) and (AUTO_REC==false))//dont show rightside menu choice when playing a file to allow the display of a filename
     {//char* s=(char*)"MODE";
      char tstr[12];
      //currently v0.81 only DETECT 
      if (RightButton_Mode==MODE_DETECT) 
           snprintf(tstr,12,"mode %s", DT_short[detector_mode]); 
             
      uint16_t sx=tft.strPixelLen(tstr);
      tft.setCursor(ILI9341_TFTWIDTH-sx,ILI9341_TFTHEIGHT-BOTTOM_OFFSET);
      tft.print(tstr);
     }  
    }
}

void showSettingsButtons()
{
    //clear lines
      tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET,ILI9341_TFTWIDTH,BOTTOM_OFFSET_PART,BCK_MENU_COLOR);
      tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART,ILI9341_TFTWIDTH,BOTTOM_OFFSET_PART,COLOR_BLACK);
    
      tft.setTextColor(HILIGHT_MENU_COLOR); //show special sidemenu indicator for time/date menus    
      if ((set_menu_id[settings_page_nr]==SET_MENU_TIME) or (set_menu_id[settings_page_nr]==SET_MENU_DATE))
        { txt_rightAlign("sidemenu [ ]",ILI9341_TFTHEIGHT-BOTTOM_OFFSET);
        
        }
    //text on the bottom 
    tft.setCursor(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART);
    tft.print("BACK ^");
    #ifdef USE_EEPROM
      txt_rightAlign("change/Save",ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART);
    #else
      txt_rightAlign("change",ILI9341_TFTHEIGHT-BOTTOM_OFFSET_PART);
    #endif  
}
void showpushtoStop()
{
    tft.setCursor(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET); //position of button functions
    tft.fillRect(0,ILI9341_TFTHEIGHT-BOTTOM_OFFSET,ILI9341_TFTWIDTH,BOTTOM_OFFSET_PART,BCK_MENU_COLOR);
    tft.setFont(MENU_FONT);
    tft.print("<< PUSH TO STOP");
}

void showSaved()
{
           tft.setTextColor(COLOR_BLACK);
           tft.fillRect(150, ILI9341_TFTHEIGHT-BOTTOM_OFFSET-20,ILI9341_TFTWIDTH-150,18,COLOR_WHITE);
           tft.setCursor(150,ILI9341_TFTHEIGHT-BOTTOM_OFFSET-18);
           tft.print("Saved");
           tft.setTextColor(COLOR_WHITE);
           
}

void startRecordScreen()
{
  tft.setScroll(0); //stop scrolling
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(ENC_MENU_COLOR);
  tft.setFont(Arial_20);
  tft.setCursor(0,100);

  if (AUTO_REC)
   {tft.println("AUTO RECORD");
    tft.print(" # "); tft.print(autocounter);
    showpushtoStop();
    }
  else
   {tft.println("RECORDING");
    tft.setFont(MENU_FONT);
    tft.print(filename); 
   }

  
  if (!AUTO_REC)
     {showPushButtons();
      }
  
}

void updateAUTORECstatus()
{
  tft.setFont(TFT_FONT);
  tft.setTextColor(ENC_MENU_COLOR);
  showpushtoStop();
  tft.fillRect(0,ILI9341_TFTHEIGHT-TFT_FONT.cap_height,ILI9341_TFTWIDTH,TFT_FONT.cap_height,COLOR_BLACK); 
  tft.setCursor(0,ILI9341_TFTHEIGHT-TFT_FONT.cap_height);
          //ms            //blocks in ms
  if (recording_stopped<(AREC_P*AREC_BLOCK)) //show the time left to wait until the next recording can start
    { uint8_t p=int( (AREC_P*AREC_BLOCK-recording_stopped)/1000);
      tft.print("PAUZE (s) ");
      tft.print(p);
    }
  else
  {
      tft.print("WAITING FOR SIGNAL"); 
  }
}

void initTFT()
{
  tft.setRotation(0);
  tft.setScroll(0);
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  tft.setCursor(0,0);
  tft.setScrollarea(TOP_OFFSET,BOTTOM_OFFSET);

}


#endif  //ifdef ILI9341

#endif