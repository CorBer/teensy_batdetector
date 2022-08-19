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

#ifndef _BAT_TFT_H
#define _BAT_TFT_H

#include "bat_vars.h"
#include "bat_sd.h"
#include "bat_menu.h"
#include "bat_fft.h"
#include "bat_image.h"

#include "bat_gps.h"


#define ILI9341 //currently only created to be used with ILI9341
#ifdef ILI9341

#include "ILI9341_t3n.h"
#include "ili9341_t3n_font_Arial.h"
#include "ili9341_t3n_font_ArialBold.h"

#define TFT_FONT Arial_16
#define MENU_FONT Arial_14

 //see https://www.pjrc.com/store/display_ili9341_touch.html
#if defined(__MK66FX1M0__)

#define BACKLIGHT_PIN 255 //backlight is controlled by hardware
#define TFT_DC 20
#define TFT_CS 21
#define TFT_RST 255 // 255 = unused. connect to 3.3V
#define TFT_MOSI 28
#define TFT_SCLK 27
#define TFT_MISO 39

#endif

#if defined(__IMXRT1062__)
#define BACKLIGHT_PIN 255 //backlight is controlled by hardware
#define TFT_DC 40
#define TFT_CS 38
#define TFT_RST 255 // 255 = unused. connect to 3.3V
#define TFT_MOSI 26
#define TFT_SCLK 27
#define TFT_MISO 39
#ifdef SCROLL_HORIZONTAL
boolean updateFramebuffer = false;
uint16_t bufferline = 0;
const uint16_t frameBuffer_height = 240;
//uses fast RAM instead of DMA !!
uint16_t frameBuffer[(frameBuffer_height + 10) * 240] = {0}; //block to store FFTpixels for the scrolling screen

#endif
#endif

//call extended library for ILI9341_t3
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

//#define Touch_CS    8
//XPT2046_Touchscreen ts = XPT2046_Touchscreen(Touch_CS);
//predefine menu background etc colors

//setup of the SCREEN BLOCKS for a 240wide and 320high TFT
//**********************
// SETTINGS AND TIME   *
//                     *
//      POWERGRAPH     *
//     SPECTRUMSCALE   *
//_____________________*<-----TOPOFFSET
//     LIVE GRAPH      *
//        .            *
//         .           *
//          .          *
//           .         *
// ____________________* <-----BOTTOMOFFSET
// PUSHBUTTON SETTING  *
// ENCODER SETTING     *
//***********************

//the powergraph is shown with a spectrumscale on the bottom and a graph above
// with the default settings the powergraph starts at y=90-45-5 and is 45 pixels hight
// the scale starts at y=90-5 and is 5 pixels high
#define TOP_OFFSET 70   //lower position of the graph ex 90
#define POWERGRAPH 40   //height of the graph         ex 45
#define SPECTRUMSCALE 6 //height of the scale  MINIMAL 6!!
#define BOTTOMSCALE 4   // pixels of bottomscale 

//split lowerhalf slightly unequal
#define BOTTOM_UPPERPART 40                   // split the BOTTOMOFFSET into 2 equal sections away from the bottom, this is the highest
#define BOTTOM_LOWERPART BOTTOM_UPPERPART / 2 // this is the lower section

#define SPECTRUM_HEIGHT ILI9341_TFTHEIGHT - TOP_OFFSET - BOTTOM_UPPERPART

//default menu colors

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################

unsigned int rainbow(byte value, byte mod)
  {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0;   // Red is the top 5 bits of a 16 bit colour value
  byte green = 0; // Green is the middle 6 bits
  byte blue = 0;  // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0)
    {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
    }
  if (quadrant == 1)
    {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
    }
  if (quadrant == 2)
    {
    blue = 0;
    green = 63;
    red = value % 32;
    }
  if (quadrant == 3)
    {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
    }

  if (mod != 2)
    {
    red = byte(red * 2 / 4);
    green = byte(green * 2 / 4);
    blue = byte(blue * 2 / 4);
    }

  return (red << 11) + (green << 5) + blue;
  }

//fast draw a stored BMP on the screen x and y need to be the same size as in the stored array
void draw_RGB_Bitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h)
  {
  for (int16_t j = 0; j < h; j++, y++)
    {
    tft.writeRect(0, y, w, 1, &bitmap[j * w]); //scanline copy to tft
    }
  }

//slowly draw a stored BMP on the screen x and y need to be the same size as in the stored array
// void draw_RGB_Bitmapold(int16_t x, int16_t y,
//   const uint16_t bitmap[], int16_t w, int16_t h) { 
//     for(int16_t j=0; j<h; j++, y++) { 
//         for(int16_t i=0; i<w; i++ ) { 
//             tft.drawPixel(x+i, y, pgm_read_word(&bitmap[j * w + i]));
//         }
//     }

// }

uint16_t ENC_MENU_COLOR = COLOR_YELLOW;
uint16_t HILIGHT_MENU_COLOR = COLOR_WHITE;
uint16_t BCK_MENU_COLOR = COLOR_DARKRED;

#define ENC_VALUE_COLOR COLOR_LIGHTGREY
// #define MENU_BCK_COLOR COLOR_DARKRED
#define MENU_BCK_COLOR_PART COLOR_DARKGREEN

//calculate needed pixels (x) for a characterstring
int calc_menu_dxoffset(const char* str)
  {
  String s = String(str);
  char charBuf[50];
  s.toCharArray(charBuf, 50);
  uint16_t dx = tft.strPixelLen(charBuf);
  return dx;
  }

//print charstring rightaligned on screen starting at y-position
void txt_rightAlign(const char* s, int16_t y)
  {
  uint16_t dx = calc_menu_dxoffset(s);
  tft.setCursor(ILI9341_TFTWIDTH - dx - 2, y);
  tft.print(s);
  }

//switch settings_page
void show_next_settings_page(int change)
  {
  settings_page_nr = cyclic_constrain(settings_page_nr, change, 0, settings_menu_pages - 1);
  tft.fillRect(0, 45, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT - 45 - 40, COLOR_BLACK);
  }

//draw the scale below the powerspectrum and above the waterfall/spectrum
FLASHMEM
void drawScale()
  {
  if (display_mode != settings_page)
    {
    if ((display_mode == spectrumgraph) or (display_mode == waterfallgraph) or (display_mode == h_waterfallgraph))
      {
      tft.fillRect(0, TOP_OFFSET - (SPECTRUMSCALE + 1), ILI9341_TFTWIDTH, SPECTRUMSCALE + 1, COLOR_BLACK);

    #ifdef BOTTOMSCALE
      if (display_mode == h_waterfallgraph)
        {
        tft.fillRect(ILI9341_TFTWIDTH - BOTTOMSCALE, TOP_OFFSET, BOTTOMSCALE, ILI9341_TFTHEIGHT - TOP_OFFSET - BOTTOM_UPPERPART, COLOR_BLACK);
        }
      else //spectrum or normal waterfall
        {
        tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - BOTTOMSCALE, ILI9341_TFTWIDTH, BOTTOMSCALE, COLOR_BLACK);
        }
    #endif    

      }

    float x_factor = 0;
    int maxScale = 0;

    if ((!playActive) or (playDirect)) //adapt to samplerate
      {
      x_factor = 10000 / (0.5 * (SR_real / FFT_points));
      maxScale = int(SR_real / 20000);
      }
    else //playing at lower speed use recorded_SR for the scale
      {
      x_factor = 10000 / (0.5 * (recorded_SR / FFT_points));
      maxScale = int(recorded_SR / 20000);
      }

    if ((HI_pass > HI_PASS_OFF))
      {
      if (!recorderActive)
        {
        tft.fillRect(0, TOP_OFFSET - SPECTRUMSCALE + 3, uint8_t(HI_pass * x_factor * 0.1), 3, COLOR_DARKGRAY);
        tft.setFont(MENU_FONT);
        tft.setCursor(10, TOP_OFFSET - SPECTRUMSCALE - POWERGRAPH + 1);
        tft.setTextColor(COLOR_BLACK);
        tft.print("hi");
        tft.setCursor(9, TOP_OFFSET - SPECTRUMSCALE - POWERGRAPH);
        tft.setTextColor(ENC_MENU_COLOR);
        tft.print("hi");
        }

      }

    for (int i = 1; i < maxScale; i++) //every 10k a tick
      {

      tft.drawFastVLine(i * x_factor - 2, TOP_OFFSET - SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
      if (i % 5 == 0)
        {
        tft.drawFastVLine(i * x_factor - 1, TOP_OFFSET - SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
        tft.drawFastVLine(i * x_factor - 3, TOP_OFFSET - SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
        }

    #ifdef BOTTOMSCALE
      if (display_mode == h_waterfallgraph)
        {
        if ((i * x_factor) < (ILI9341_TFTHEIGHT - TOP_OFFSET - BOTTOM_UPPERPART))
          {
          tft.drawFastHLine(ILI9341_TFTWIDTH - BOTTOMSCALE, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - i * x_factor, BOTTOMSCALE, ENC_MENU_COLOR);
          //tft.drawFastHLine(ILI9341_TFTWIDTH-BOTTOMSCALE, ILI9341_TFTHEIGHT-BOTTOM_UPPERPART-i*x_factor+1, BOTTOMSCALE, ENC_MENU_COLOR);
          if (i % 5 == 0) //hilight every 5th marker
            {
            tft.drawFastHLine(ILI9341_TFTWIDTH - BOTTOMSCALE, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - i * x_factor + 1, BOTTOMSCALE, COLOR_WHITE);
            tft.drawFastHLine(ILI9341_TFTWIDTH - BOTTOMSCALE, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - i * x_factor - 1, BOTTOMSCALE, COLOR_WHITE);
            }
          }
        }
      else //spectrum or normal waterfall
        {
        tft.drawFastVLine(i * x_factor - 2, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - BOTTOMSCALE, BOTTOMSCALE, ENC_MENU_COLOR);
        if (i % 5 == 0)
          {
          tft.drawFastVLine(i * x_factor - 1, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - BOTTOMSCALE, BOTTOMSCALE, ENC_MENU_COLOR);
          tft.drawFastVLine(i * x_factor - 3, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - BOTTOMSCALE, BOTTOMSCALE, ENC_MENU_COLOR);

          }
        }

    #endif

      }
    }
  }

//draw markers on the scale
void drawScaleMarker() //draws a filled triangle on the scale at frequency where ultrasound trigger starts
  {
  if (not(LeftButton_Mode == MODE_PLAY) or (playDirect))
    {
    if (display_mode != settings_page)
      {
      int curF = signal_LoF_bin;
      tft.fillTriangle(curF - 3, TOP_OFFSET - 1, curF + 3, TOP_OFFSET - 1, curF, TOP_OFFSET - 9, COLOR_WHITE);
      //float f=SR_real/FFT_points;
      curF = int(osc_frequency / SR_FFTratio);
      if ((curF < ILI9341_TFTWIDTH - 4) and ((detector_mode == detector_Auto_heterodyne) or (detector_mode == detector_heterodyne)))
        {
        tft.fillCircle(curF, TOP_OFFSET - SPECTRUMSCALE + 1, 2, COLOR_YELLOW);
        }
      }
    }
  }

//draw a small bar-graph on the header
void headergraph(uint8_t x, uint8_t height, uint8_t mxheight)
  {
  for (uint8_t i = 0; i < height; i++)
    {
    if (i % 2 > 0)
      {
      tft.drawFastHLine(x, mxheight - i, 10, ENC_MENU_COLOR);
      }
    }
  }

//show the time and update GPS/DS18B20 (if used)
void showTime()
  {
  char tstr[9];
  seconds2time(getRTC_TSR());
  tft.setFont(MENU_FONT);
  snprintf(tstr, 9, "%02d:%02d ", tm_hour, tm_min);

  uint8_t dx = calc_menu_dxoffset(tstr);

  tft.fillRect(ILI9341_TFTWIDTH - dx - 3, 0, dx + 3, MENU_FONT.cap_height + 3, BCK_MENU_COLOR);
  tft.setCursor(ILI9341_TFTWIDTH - dx + 1, 1);
  tft.setTextColor(ENC_MENU_COLOR);
  tft.print(tstr);

  tft.fillRect(ILI9341_TFTWIDTH - 2, tm_sec / 4, 2, 2, COLOR_WHITE);

#ifdef USE_GPS

  if (GPSbaudOK)
    {
    tft.setFont(TFT_FONT);
    //first clear the headerblock
    //uint headheight=TFT_FONT.cap_height+2;
    //uint dx=tft.strPixelLen((char *)"23:55"); //estimate leftmost position of time display
    dx = dx + 17;
    tft.fillRect(ILI9341_TFTWIDTH - dx, 0, 16, TFT_FONT.cap_height + 2, BCK_MENU_COLOR);
    uint8_t dxl = ILI9341_TFTWIDTH - dx;
    //byte fix;
    //fix=gps_fix;
    //based on headheight of 16 pixels draw a GPS droplet with a hole
    no_gps = (now() - GPSfix_time) / 60; // time since the latest GPSfix in minutes

    if (no_gps > 5) //no connection
      {
      tft.fillCircle(dxl + 5, 7, 5, COLOR_ORANGE);
      tft.fillTriangle(dxl + 0, 8, dxl + 10, 8, dxl + 5, 16, COLOR_ORANGE);
      }

    else //connected
      {
      uint16_t dropletcol = COLOR_GREEN;
      if (no_gps == 0)
        {
        dropletcol = COLOR_LIGHTBLUE;
        }

      tft.fillCircle(dxl + 5, 7, 5, dropletcol);
      tft.fillTriangle(dxl + 0, 8, dxl + 10, 8, dxl + 5, 16, dropletcol);
      if (no_gps == 0)
        {
        tft.setFont(Arial_8);
        tft.setTextColor(COLOR_BLACK);
        tft.setCursor(dxl + 4, 4);
        if (gps_SIV < 10)
          {
          tft.print(gps_SIV);
          }
        else
          {
          tft.print("+");
          }

        tft.setTextColor(ENC_MENU_COLOR);
        tft.setFont(TFT_FONT);
        }
      }

    if (no_gps > 0)
      {
      byte r = no_gps; //in minutes
      if (r > 4)
        {
        r = 4;
        }
      tft.fillCircle(dxl + 5, 7, r, COLOR_BLACK);
      }
    if (no_gps > 10) // no connection for 10 minutes or at startup
      {
      tft.fillCircle(dxl + 5, 7, 5, COLOR_TOMATO);
      tft.fillTriangle(dxl + 0, 8, dxl + 10, 8, dxl + 5, 16, COLOR_TOMATO);
      tft.setFont(MENU_FONT);
      snprintf(tstr, 2, "x");
      tft.setCursor(dxl + 1, 0);
      tft.setTextColor(COLOR_YELLOW);
      tft.print(tstr);
      tft.setTextColor(ENC_MENU_COLOR);
      tft.setFont(TFT_FONT);
      //tft.drawChar(dxl,2,'X',COLOR_WHITE,COLOR_TOMATO,2);
      }
    }
  // else //no GPS connected over serial
  // { tft.setFont(TFT_FONT);
  //   dx=dx+17;
  //   uint8_t dxl=ILI9341_TFTWIDTH-dx;
  //   tft.setCursor(dxl,0);
  //   tft.setTextColor(COLOR_WHITE);
  //   tft.print(tstr);
  //   tft.setTextColor(ENC_MENU_COLOR);
  // }

#endif

#ifdef USE_DS18B20
  tft.setFont(Arial_12_Bold);
  //bat_tempC=-1.5;
  float timecorrect = 1.0;

#ifdef TEMP_TIMECORRECT
  uint32_t temp_time = millis();
  if (temp_time < 20 * 60000)
    {
    timecorrect = float(temp_time) / float(20 * 60000);
    //Serial.println(timecorrect);
    }
#endif  

  if (bat_tempC != -127)
    {
    snprintf(tstr, 5, "%.1f ", bat_tempC + T_corrector * timecorrect);
    }
  else
    {
    tft.setTextColor(COLOR_WHITE);
    snprintf(tstr, 3, "NC ");
    tft.setTextColor(ENC_MENU_COLOR);
    }

  uint t_dx = calc_menu_dxoffset("99.9");
  dx += t_dx;
  tft.setCursor(ILI9341_TFTWIDTH - dx - 4, 4);
  tft.fillRect(ILI9341_TFTWIDTH - dx - 4, 0, t_dx - 1, MENU_FONT.cap_height + 2, BCK_MENU_COLOR);

  tft.print(tstr);
#endif
  }

//update the time and check if an update is appropriate
FLASHMEM
void updateTime()
  {
  if ((timeupdate > 10000))
    {
    showTime();
    timeupdate = 0;
    }
  }

//show all information in the header (full update)
FLASHMEM
void showHeader()
  {
  tft.setFont(TFT_FONT);
  //first clear the headerblock
  uint8_t headheight = TFT_FONT.cap_height + 2;
  tft.fillRect(0, 0, ILI9341_TFTWIDTH, headheight, BCK_MENU_COLOR);
  //position cursor and display important settings
  tft.setCursor(0, 1);
  tft.setTextColor(ENC_MENU_COLOR);
  tft.print("V");
  uint8_t x = tft.getCursorX();
  headergraph(x + 2, uint8_t(volume * 20 / 90), headheight);

  if (AGC_mode == 0)
    {
    tft.setCursor(x + 15, 1);
    tft.print("G");
    x = tft.getCursorX();
    headergraph(x + 2, uint8_t(mic_gain * 20 / 39), headheight);
    }
  else
    {
    tft.setCursor(x + 15, 4);
    tft.setFont(Arial_13_Bold);
    tft.print("AG");
    x = tft.getCursorX();
    tft.print("C");
    tft.setFont(TFT_FONT);
    }

  if (AudioMemorywarning)
    {
    tft.fillRect(0, 0, ILI9341_TFTWIDTH, headheight, COLOR_BLUE);
    tft.setCursor(0, 1);
    tft.setTextColor(ENC_MENU_COLOR);
    tft.print("!AUDIOMEM!");
    }
  else //(!AudioMemorywarning)
    {
    tft.setCursor(x + 20, 1);

    if (LeftButton_Mode == MODE_PLAY)
      {
      if ((recorded_SR != 0) and (playActive)) //only show replay SR when playing
        {
        tft.print("R");
        tft.print(uint16_t(recorded_SR / 1000));
        // tft.print("k");
        }
      }
    else
      {
      if (mv1 != 9999)
        {
      #ifdef VIN_ON  //monitor VIN 
        if (mv1 < VIN_LOW)
          {
          tft.fillRect(0, 0, ILI9341_TFTWIDTH, headheight, COLOR_BLUE);
          tft.setCursor(0, 1);
          tft.setTextColor(ENC_MENU_COLOR);
          tft.print("VIN LOW ");
          }
        tft.print(mv1); //show VIN if measurements are ON
      #endif
        }
      else
        {
        tft.print(SRtext);
        tft.print('k');
        }

      }
    }

  showTime(); //always update the time at the end since this operation clears the complete header
  }

//if no SDcard is mounted show this

void showNOSD()
  {
  if ((!SD_ACTIVE))
    {
    char tstr[9];
    tft.setFont(Arial_13);
    snprintf(tstr, 9, "no SD");
    //tft.setCursor(ILI9341_TFTWIDTH-tft.strPixelLen(tstr),TOP_OFFSET-TFT_FONT.cap_height-SPECTRUMSCALE);
    tft.setTextColor(HILIGHT_MENU_COLOR);
    txt_rightAlign(tstr, TOP_OFFSET - TFT_FONT.cap_height - SPECTRUMSCALE);
    tft.print(tstr);
    }
  else
    {
      
    #if SD_FAT_VERSION!=20005
        #pragma message ("SDFAT library not 2.0.5")
    #endif

    if (SD_FAT_VERSION != 20005)
      {
      
      char tstr[12];
      tft.setFont(Arial_13);
      snprintf(tstr, 12, "WRONG SDFAT");
      tft.setCursor(0, 20);
      tft.setTextColor(HILIGHT_MENU_COLOR);
      tft.print(tstr);
      }
    }
  }

uint8_t menugap = 0;
//show a menu line with predefined texts on a position

FLASHMEM
void showMenuLine(const char* menu_txt, const char* tstr, uint8_t line, uint8_t update_page, bool reverse)
  {
  int h = SET_MENU_FONT.cap_height + 3;
  tft.setCursor(2, TOP_OFFSET - POWERGRAPH + line * h + menugap);

  if (reverse == true) // this menuline has the cursor, show in reverse
    {
    tft.setTextColor(COLOR_BLACK);
    tft.fillRect(0, TOP_OFFSET - POWERGRAPH + line * h + menugap, ILI9341_TFTWIDTH, h - 2, HILIGHT_MENU_COLOR);
    }
  else //normal menu_entry
    {
    tft.setTextColor(ENC_MENU_COLOR);
    tft.fillRect(0, TOP_OFFSET - POWERGRAPH + line * h + menugap, ILI9341_TFTWIDTH, h - 2, COLOR_BLACK);
    }

  if (menu_txt != Settings0Menu[0].menu_txt) //check if we are updating a page_header or a normal menu entry
    {                                          //normal menu
    tft.print(menu_txt);                     //show stored menutext
    txt_rightAlign(tstr, tft.getCursorY());
    }
  else // header, so dont show the settings_menu.menu_txt part but only the pagename
    {
        {
        uint16_t t_col = ENC_MENU_COLOR;
        if (line != set_menu_pos[update_page])
          {
          t_col = BCK_MENU_COLOR;
          }
        tft.fillRect(0, TOP_OFFSET - POWERGRAPH - SPECTRUMSCALE + line * h, ILI9341_TFTWIDTH, h + 4, t_col); //use default backgroundcolor for the header
        tft.setCursor(0, TOP_OFFSET - POWERGRAPH - SPECTRUMSCALE + 2 + line * h);
        tft.print(tstr); //show menutext
        }
    }
  }

//update a menuline if the corresponding values have changed
void update_menu_value(uint8_t menu_id, const char* menu_txt, uint8_t update_page, uint8_t line)
  {
  const uint8_t tlen = 20;
  char tstr[tlen];
  snprintf(tstr, tlen, getMenuTxtVal(menu_id, update_page)); //build up the text for value based on the variable
  //now show the menu entry at the line and reversed if necessary
  bool reverse = (line == set_menu_pos[update_page]);
  if (menu_txt[0] == '>')
    {
    menugap += 5; //add 6 pixel gap
    menu_txt = &menu_txt[1];
    }
  //allow up to 2 > marks
  if (menu_txt[0] == '>')
    {
    menugap += 5; //add 6 pixel gap
    menu_txt = &menu_txt[1];
    }

  showMenuLine(menu_txt, tstr, line, update_page, reverse);
  }

//show settings pages
FLASHMEM
void showSettings()
  {
  tft.setTextColor(ENC_MENU_COLOR); //default color
  tft.setFont(SET_MENU_FONT);       //default font allows 12 lines of settings
  //for each of the settings pages build the menu in order of the settings page array
  menugap = 0;
  //update the directories before showing them
  if ((settings_page_nr == setpage_SDCARD) and (SD_ACTIVE))
    {
    if (lastsettings_page_nr != 4)
      {
      D_PRINTLN("CHECKDIRS from SETTINGSPAGE");

      countDirs_inroot();
      }
    }

  for (int i = 0; i < settings_MenuOptions[settings_page_nr]; i++)
    { //push the menu item, the pagenr and the screenline we are updating
    if (settings_page_nr == 0)
      {
      update_menu_value(Settings0Menu[i].menu_id, Settings0Menu[i].menu_txt, settings_page_nr, i); //update line in menu
      }
    if (settings_page_nr == 1)
      {
      update_menu_value(Settings1Menu[i].menu_id, Settings1Menu[i].menu_txt, settings_page_nr, i); //update line in menu
      }
    if (settings_page_nr == 2)
      {
      update_menu_value(Settings2Menu[i].menu_id, Settings2Menu[i].menu_txt, settings_page_nr, i); //update line in menu
      }
    if (settings_page_nr == 3)
      {
      update_menu_value(Settings3Menu[i].menu_id, Settings3Menu[i].menu_txt, settings_page_nr, i); //update line in menu
      }

  #ifdef SHOW_SDMENU
    if (settings_page_nr == setpage_SDCARD) //dont show the full menu when no card is mounted
      {
      if (SD_ACTIVE)
        {
        update_menu_value(SettingsSDCARDMenu[i].menu_id, SettingsSDCARDMenu[i].menu_txt, settings_page_nr, i);
        }
      else
        {
        if (i == 0)
          {
          update_menu_value(SettingsSDCARDMenu[i].menu_id, SettingsSDCARDMenu[i].menu_txt, settings_page_nr, i);
          }
        } //update line in menu
    }
  #endif

  #ifdef USE_GPS
    if (settings_page_nr == setpage_GPS)
      {
      update_menu_value(SettingsGPSMenu[i].menu_id, SettingsGPSMenu[i].menu_txt, settings_page_nr, i); //update line in menu
  }
  #endif
    }

#ifdef USE_GPS
  if (lastsettings_page_nr == setpage_GPS) //we leave the GPS page
    {
    if (settings_page_nr != setpage_GPS)
      {
      I2SON(); // switch ON I2S
      }
    }
  //we enter the GPS page
  if (settings_page_nr == setpage_GPS)
    {
    if (lastsettings_page_nr != setpage_GPS)
      {
      I2SOFF(); //switch off I2S
      }
    }
#endif
  //keep track
  lastsettings_page_nr = settings_page_nr;
  }

#include "bat_audio.h" //myFFT structure

void spectrum()
  {                                                                      // spectrum analyser code by rheslip - modified
  uint16_t OFFSET = ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TOP_OFFSET; //default 320 - 40 - 90 = 190
  //int16_t FFT_sbin [128];
  // limit until bin 119 to prevent overflow
  uint16_t barnew = 0;
  uint16_t bar;
  uint16_t barm = 0;
  //uint8_t g_x;
  uint8_t spectrumline;
  uint8_t spectrumline_new;
  uint16_t barg[120];
  uint16_t barg_new[120];

  for (int16_t x = 2; x < 119; x++)
    {
    //FFT_sbin[x] = int(myFFT.output[x]);//-FFTavg[x]*0.9;
    //int barnew = (FFT_sbin[x]) ;
    barnew = myFFT.output[x];
    // this is a very simple first order IIR filter to smooth the reaction of the peak values of the bars
    bar = (2 * barnew + 8 * barm) / 10;
    barm = bar;
    barg[x] = constrain(bar, 0, OFFSET - 1);
    barg_new[x] = constrain(barnew, 0, OFFSET - 1);
    }

  for (int16_t x = 2; x < 119; x++)
    {
    spectrumline = barg[x];
    spectrumline_new = barg_new[x];
    uint8_t len = 30;
    int16_t start = spectrumline_new - len;
    if (start < 0)
      {
      start = 0;
      }
    tft.drawFastVLine(x * 2, TOP_OFFSET, start, COLOR_ORANGE);
    tft.drawFastVLine(x * 2, TOP_OFFSET + start, len, COLOR_YELLOW);                                 // X, 90, length bar, colour
    tft.drawFastVLine(x * 2, TOP_OFFSET + spectrumline_new, OFFSET - spectrumline_new, COLOR_BLACK); // X, 90, length bar, colour

    if (spectrumline > 4)
      {
      tft.drawFastVLine(x * 2, TOP_OFFSET + spectrumline - 5, 5, COLOR_WHITE); //draw the moving average peak-hold
      }
    }
  }


void showPowerSpectrum()
  {
  uint8_t old_LoF_bin = signal_LoF_bin;
  powerspectrumCounter = 0;

  float bin2frequency = 0;

  if ((playActive) and (!playDirect))
    { //adapt to the recorded samplerate
    bin2frequency = (recorded_SR / (FFT_points * 2)) * 0.001;
    signal_LoF_bin = 0;
    }
  else
    {
    bin2frequency = SR_FFTratio * 0.001;
    }

  float halfbin = bin2frequency / 2; //correct for the bin-middle
  //clear powerspectrumbox
  tft.fillRect(0, TOP_OFFSET - POWERGRAPH - SPECTRUMSCALE, ILI9341_TFTWIDTH, POWERGRAPH + 1, COLOR_BLACK);

  // keep a minimum maximumvalue to the powerspectrum
  float powerSpectrum25 = (powerspectrum_Max + 1) * 0.25f;
  float powerSpectrum50 = (powerspectrum_Max + 1) * 0.5f;

  //prevent div by zero errors !!
  //find the nearest frequencies below 10% of the maximum to show on screen as values

  //draw spectrumgraph
  uint16_t spectrumcolor = COLOR_RED;
  //boolean peakFound=false;
  uint8_t lopeak = 0;
  uint8_t hipeak = 0;

  uint8_t i = powerSpectrum_Maxbin + 1;
  while ((FFTpowerspectrum[i] >= powerSpectrum50) and (i < 239))
    {
    i++;
    }
  //uint8_t hi_i=i;

  while ((FFTpowerspectrum[i] >= powerSpectrum25) and (i < 239))
    {
    i++;
    }
  hipeak = i;

  i = powerSpectrum_Maxbin - 1;
  while ((FFTpowerspectrum[i] >= powerSpectrum50) and (i > 0))
    {
    i--;
    }
  //uint8_t lo_i=i;

  while ((FFTpowerspectrum[i] >= powerSpectrum25) and (i > 0))
    {
    i--;
    }
  lopeak = i;
  if (powerspectrum_Max > 0)
    {
    for (uint8_t i = 5; i < spec_hi; i++)
      {
      int ypos = FFTpowerspectrum[i] * 1.0f / powerspectrum_Max * 1.0f * POWERGRAPH;
      if (ypos > POWERGRAPH)
        {
        ypos = POWERGRAPH - 2;
        }
      spectrumcolor = COLOR_RED;
      if ((i >= lopeak) and (i <= hipeak)) //25% range
        {
        spectrumcolor = COLOR_ORANGE;
        if (i == powerSpectrum_Maxbin)
          {
          spectrumcolor = COLOR_WHITE; //peak values
          }
        }
      tft.drawFastVLine((i - spec_lo), TOP_OFFSET - ypos - SPECTRUMSCALE, ypos, spectrumcolor);
      }
    }


  if (powerSpectrum_Maxbin != 0) //enough samples collected
    {



    for (uint8_t i = 0; i < spec_hi; i++)
      {
      FFTpowerspectrum[i] = 0;
      }

    uint32_t TE_F = int(powerSpectrum_Maxbin * bin2frequency + halfbin);

    if ((TE_F / TE_speed) > 5) //above 5K (so above 60Khz)
      {
      TE_speed = TE_F / 5;
      granular1.setTESpeed(TE_speed);
      }
    else //restore
      {
      granular1.setTESpeed(last_TE_speed);
      TE_speed = last_TE_speed;
      }


    char tstr[20];
    tft.setFont(Arial_13);

    snprintf(tstr, 12, "%3d %3d %3d ", int(lopeak * bin2frequency + halfbin), int(powerSpectrum_Maxbin * bin2frequency + halfbin), int(hipeak * bin2frequency + halfbin));

    if (myInput != 2)
      {
      tft.setTextColor(COLOR_BLACK);
      txt_rightAlign(tstr, TOP_OFFSET - 44);
      tft.setTextColor(COLOR_WHITE);
      txt_rightAlign(tstr, TOP_OFFSET - 45);
      }


    }

  powerspectrum_Max = powerspectrum_Max * 0.5; //lower the max after a graphupdate
  showNOSD();
  if (playActive)
    {
    signal_LoF_bin = old_LoF_bin;
    }
  }

void deepSleepStartScreen()
  {
  tft.fillScreen(COLOR_DARKRED);
  tft.setCursor(0, 0);
  tft.setFont(Arial_18);
  tft.println("GOING TO");
  tft.println("");
  tft.println("DEEP SLEEP");
  tft.println("IN");
  tft.println("10 SECONDS");

  D_TIME();
  D_PRINTLN("ACTIVEDEEPSLEEP ON");
  delay(5000);
  }

void showresettoDefault(uint8_t mode)
  {
  tft.setRotation(0);
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  tft.setCursor(0, 50);
  tft.setFont(Arial_20);
  tft.println("     RESET ");
  tft.println(" ");
  tft.println("  TO  DEFAULTS ");
  if (mode == 1)
    {
    tft.println("   from SD  ");

    }
  }

void StartupScreenimage()
  {
  tft.setRotation(0);
  tft.fillScreen(COLOR_BLACK);
  draw_RGB_Bitmap(0, 0, batPvH, 240, 153); //image by paul van Hoof https://www.paulvanhoof.nl/
  tft.setCursor(0, 0);
  tft.setFont(Arial_20);
  char tstr[9];
  seconds2time(getRTC_TSR());
  snprintf(tstr, 9, "%02d:%02d:%02d", tm_hour, tm_min, tm_sec);
  txt_rightAlign(tstr, 0);
  }

#define FDATED  (char const[]) {  __DATE__[4], __DATE__[5],'\0' }
#define FDATEM  (char const[]) {  __DATE__[0], __DATE__[1] , __DATE__[2],'\0'}
#define FDATEY  (char const[]) {  __DATE__[7], __DATE__[8] , __DATE__[9],  __DATE__[10],'\0'}

void StartupScreen()
  {
  D_PRINTLN_F(D_BOLDGREEN, "STARTUPSCREEN START");
  //char tstr[9];
  //tft.print(tstr);
  tft.setCursor(0, 150);
  tft.setTextColor(COLOR_YELLOW);
  tft.println("Teensy Batdetector");
  tft.setTextColor(COLOR_WHITE);
  tft.setFont(Arial_14);
  tft.setCursor(0, 175);
  tft.print(versionStr);
#if defined(__MK66FX1M0__)
  tft.print(" T3.6@");
  tft.print(F_CPU / 1000000);
  tft.println("Mhz");

#endif
#if defined(__IMXRT1062__)
  tft.print("T4.1 @");
  tft.print(F_CPU / 1000000);
  tft.println("Mhz");
#endif

  tft.setCursor(0, 195);
  tft.print(FDATEY); //show compiled date/time
  tft.print("-");
  tft.print(FDATEM); //show compiled date/time
  tft.print("-");
  tft.print(FDATED); //show compiled date/time
  tft.print(" ");
  tft.println(String(__TIME__));
  OS_compiler="User";
  #ifdef COMPILEROS
   OS_compiler=STR(COMPILEROS);
   #pragma message(STR(COMPILEROS))
  #endif

  tft.print(OS_compiler); //show compilerversion
  tft.println(" compiled");
  D_PRINTXY("Compiled by:",OS_compiler);
  
  tft.setCursor(0, 235);

  tft.printf("TeensyDuino: 1.%3d\n", TEENSYDUINO); //teensyduino version
#ifdef USE_PSRAM
  if (PSRAMsize > 0)
    {
    tft.printf("PSRAM :%dMB", PSRAMsize);
  }
#endif

  D_PRINTLN_F(D_BOLDGREEN, "STARTUPSCREEN END");

  }

FLASHMEM
void showEncoders()
  {
  tft.setFont(MENU_FONT);
  // **************************** DISPLAY ENCODER FUNCTIONS/VALUE
  tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART + 2); //display of encoder functions

  // set the colors according to the function of the encoders when NOT in the settings_page
  if ((display_mode != settings_page) and (AUTO_REC == false))
    {
    /****************** LEFT SIDE ENCODER ***********/

    uint16_t f_col = COLOR_BLACK;
    uint16_t b_col = HILIGHT_MENU_COLOR;

    if (EncLeft_function != enc_value)
      {
      f_col = ENC_MENU_COLOR;
      b_col = COLOR_BLACK;
      if (EncLeftchange != 0)
        {
        f_col = HILIGHT_MENU_COLOR;
        }
      }

    uint8_t mxLstr = 30;
    char ltstr[mxLstr];

    //playing a file
    if (((LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_PLAY)) and (EncLeft_function == enc_value) and (!playActive))
      // when play selected only show filename
      {
      //test if filename is HEX_ENCODED datetime
      //if (!root_active)
      tft.fillRect(0, TOP_OFFSET - POWERGRAPH - SPECTRUMSCALE, ILI9341_TFTWIDTH, POWERGRAPH + SPECTRUMSCALE, COLOR_BLACK);
      readFileInfo_byindex(fileindex[fileselect]);

      uint8_t peak_i = 0;
      uint8_t hi_i = 0;
      uint8_t lo_i = 0;

      if (wavPSpectrum)
        { //tft.fillRect(0,TOP_OFFSET-POWERGRAPH-SPECTRUMSCALE,ILI9341_TFTWIDTH,POWERGRAPH+1, COLOR_BLACK);
        uint8_t dip = 255;
        uint8_t peakpos = 0;
        uint8_t peak = 0;

        for (uint8_t i = 0; i < 120; i++)
          {
          if (wavPowerSpectrum[i] > peak)
            {
            peak = wavPowerSpectrum[i];
            peakpos = i * 2;
            peak_i = i;
            }
          }
        if (peak > 0) //only if we have data display
          {
          //find hi
          uint8_t i = peak_i;
          //USE 0.5 since this is a non-squared spectrum !!!
          while (wavPowerSpectrum[i] > (peak * 0.5f) and (i < 120))
            {
            i++;
            }
          hi_i = i;
          i = peak_i;
          while (wavPowerSpectrum[i] > (peak * 0.5f) and (i > 0))
            {
            i--;
            }
          lo_i = i;

          //search for the lowest value in the higher frequencies
          if (peak_i > 70) // at least scan the to 50 bins for the dip
            {
            peak_i = 70;
            }

          for (uint8_t i = peak_i; i < 120; i++)
            if (wavPowerSpectrum[i] < peak)
              {
              dip = wavPowerSpectrum[i];
              }

          //square the values to achieve a powerspectrum
          float pspecfactor = POWERGRAPH / ((peak * peak) - (dip * dip) + 0.00001); //deltavalues per pixels
          for (uint8_t i = 0; i < 120; i++)
            {
            float powerS1 = ((wavPowerSpectrum[i] * wavPowerSpectrum[i]) - dip * dip) * pspecfactor;         //square
            float powerS2 = ((wavPowerSpectrum[i + 1] * wavPowerSpectrum[i + 1]) - dip * dip) * pspecfactor; //square
            uint16_t graphcol = COLOR_RED;
            if ((i > lo_i) and (i < hi_i))
              {
              graphcol = COLOR_ORANGE;
              }
            uint8_t wavPowerSpec = uint8_t(powerS1);
            tft.drawFastVLine(i * 2, TOP_OFFSET - SPECTRUMSCALE / 2 - wavPowerSpec, wavPowerSpec, graphcol);
            wavPowerSpec = uint8_t((powerS1 + powerS2) * 0.5);
            tft.drawFastVLine(i * 2 + 1, TOP_OFFSET - SPECTRUMSCALE / 2 - wavPowerSpec, wavPowerSpec, graphcol);
            }

          uint8_t maxScale = int(recorded_SR / 20000);
          float x_factor = 10000 / (0.5 * (recorded_SR / FFT_points));
          for (int i = 1; i < maxScale; i++)
            {
            tft.drawFastVLine(i * x_factor, TOP_OFFSET - SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
            tft.drawFastVLine(i * x_factor + 1, TOP_OFFSET - SPECTRUMSCALE, SPECTRUMSCALE, ENC_MENU_COLOR);
            }

          char tstr[20];
          tft.setFont(Arial_13);
          tft.setTextColor(COLOR_WHITE);
          float bin2frequency = (recorded_SR / (FFT_points * 2)) * 0.001;
          peak_i = int(peakpos * bin2frequency + bin2frequency / 2);
          lo_i = int(lo_i * 2 * bin2frequency + bin2frequency / 2);
          hi_i = int(hi_i * 2 * bin2frequency + bin2frequency / 2);



          snprintf(tstr, 20, "%3d %3d %3d", lo_i, peak_i, hi_i);
          tft.setTextColor(COLOR_BLACK);
          txt_rightAlign(tstr, TOP_OFFSET - 44);

          tft.setTextColor(COLOR_WHITE);
          txt_rightAlign(tstr, TOP_OFFSET - 45);
          }

        } //end WAVSPECTRUM

      tft.setScroll(0);
      tft.setFont(MENU_FONT);

      //clear area large enough for dirname/filename/filedetails
      tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TFT_FONT.cap_height * 5, ILI9341_TFTWIDTH, TFT_FONT.cap_height * 5, COLOR_BLACK);
      unsigned long seconds = strtol(strlwr(filename), NULL, 16);
      tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TFT_FONT.cap_height * 2 - 5);
      tft.setTextColor(HILIGHT_MENU_COLOR);

      if (seconds > 1577836800) //1 jan 2020 HEXCODED FILENAME
        {
        seconds2time(seconds);
        snprintf(ltstr, 30, "%04d%02d%02d %02d%02d%02d", tm_year + 1970, tm_mon, tm_mday, tm_hour, tm_min, tm_sec);
        b_col = COLOR_YELLOW; //change the color of the lowerleft indicator
        f_col = COLOR_BLACK;
        tft.print(ltstr);
        }
      else
        {
        if (root_active) //print a dir
          {
          tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TFT_FONT.cap_height * 4 - 15); //move cursor up to print dirname
          tft.print("[");
          }
        tft.print(filename);
        if (root_active)
          {
          tft.print("]");
          }

        tft.println();

        if (!root_active)
          {
          //2nd line of fileinfo
          // uint16_t xp;
          // uint16_t yp;
          // xp=tft.getCursorX();
          // yp=tft.getCursorY();
          tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TFT_FONT.cap_height * 4 - 15); //move cursor up to print dirname
          tft.print('[');
          tft.print(active_dir);
          tft.println(']');
          // tft.setCursor(xp,yp);
          tft.print("len: ");
          tft.print(file_duration);
          tft.print("ms  Srate:");
          tft.print(int(recorded_SR / 1000));
          tft.print("K");
          tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART - TFT_FONT.cap_height * 4 - 15);


          }
        else
          {
          tft.print("files:");
          tft.print(FilesinDirIndex(dirindex[fileselect]));
          }

        //reposition the cursor
        tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART + 2);

        if (root_active)
          {
          snprintf(ltstr, mxLstr, "MENU ^");
          }
        else
          {
          snprintf(ltstr, mxLstr, "ROOTDIR ^");
          }

        } //END non-hex filename
      }

    else //depending on current setting show controls

      {
      if (!playActive) //show the normal menu
        {
        snprintf(ltstr, mxLstr, "%s", LeftBaseMenu[EncLeft_menu_idx].menu_txt);
        }
      else
        {
        snprintf(ltstr, mxLstr, "%s %2d", "Play-vol", volume);
        }
      }

    //LEFTENCODER ON REC SETTING
    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_REC)
      {
      if (EncLeft_function == enc_value)
        {
        snprintf(ltstr, mxLstr, "MENU ^");
        }
      else //MENUMODE
        {
        snprintf(ltstr, mxLstr, "REC(WAV)");
        }
      }

    //ADD values to menu readouts
    char ltstrcopy[mxLstr];
    snprintf(ltstrcopy, mxLstr, ltstr);
    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_GAIN_MIC)
      {
      snprintf(ltstr, mxLstr, "%s %.1f", ltstrcopy, real_gain);
      }

    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_FRQ)
      {
      if (detector_mode == detector_heterodyne)
        {
        snprintf(ltstr, mxLstr, "%s %d", ltstrcopy, osc_frequency / 1000);
        }
      else
        {
        snprintf(ltstr, mxLstr, "%s AUTO", ltstrcopy);
        f_col = ENC_MENU_COLOR;
        b_col = COLOR_BLACK;
        EncLeft_function = enc_menu;
        }
      }
    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_VOL)
      {
      snprintf(ltstr, mxLstr, "%s %d", ltstrcopy, volume);
      }

    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_DISPLAY)
      {
      snprintf(ltstr, mxLstr, "%s disp", setDisplay[display_mode]);
      }

    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_SR)
      {
      if (LeftButton_Mode == MODE_PLAY)
        {
        if (play_SR > 1)
          {
          snprintf(ltstr, mxLstr, "%s 1/%d", ltstrcopy, play_SR);
          }
        else
          {
          snprintf(ltstr, mxLstr, "%s %s", ltstrcopy, "Direct");
          }
        }
      else
        {
        snprintf(ltstr, mxLstr, "%s %s", ltstrcopy, SR[oper_SR].txt);
        }
      }

    if (LeftBaseMenu[EncLeft_menu_idx].menu_id == MENU_HIP)
      {
      if (HI_pass > HI_PASS_OFF)
        {
        snprintf(ltstr, mxLstr, "%s(%d) %d", ltstrcopy, HI_pass_stages, HI_pass);
        }
      else
        {
        snprintf(ltstr, mxLstr, "%s OFF", ltstrcopy);
        }
      }

    tft.setTextColor(f_col);
    tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART, ILI9341_TFTWIDTH / 2 - 2, BOTTOM_LOWERPART, b_col);
    tft.print(ltstr);

    /****************** RIGHT SIDE ENCODER ***********/
    f_col = COLOR_BLACK;
    b_col = HILIGHT_MENU_COLOR;

    if (EncRight_function != enc_value)
      {
      f_col = ENC_MENU_COLOR;
      b_col = COLOR_BLACK;
      if (EncRightchange != 0)
        {
        f_col = HILIGHT_MENU_COLOR;
        }
      }

    uint8_t mxRstr = 12;
    char rtstr[mxRstr];
    if (RightBaseMenu[EncRight_menu_idx].menu_id == MENU_GAIN_MIC)
      {
      snprintf(rtstr, mxRstr, "%s %.1f", RightBaseMenu[EncRight_menu_idx].menu_txt, real_gain);
      }

    if (RightBaseMenu[EncRight_menu_idx].menu_id == MENU_FRQ)
      {
      if (detector_mode == detector_heterodyne)
        {
        snprintf(rtstr, mxRstr, "%s %d", RightBaseMenu[EncRight_menu_idx].menu_txt, int(osc_frequency / 1000));
        }
      else
        {
        snprintf(rtstr, mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, "AUTO");
        f_col = ENC_MENU_COLOR;
        b_col = COLOR_BLACK;
        EncRight_function = enc_menu;
        }
      }

    if (RightBaseMenu[EncRight_menu_idx].menu_id == MENU_VOL)
      {
      snprintf(rtstr, mxRstr, "%s %d", RightBaseMenu[EncRight_menu_idx].menu_txt, volume);
      }

    if (RightBaseMenu[EncRight_menu_idx].menu_id == MENU_SR)
      {
      if (LeftButton_Mode == MODE_PLAY)
        {
        if ((play_SR > 1) and (play_SR < MAX_play_SR))
          {
          snprintf(rtstr, mxRstr, "%s 1/%d", RightBaseMenu[EncRight_menu_idx].menu_txt, play_SR);
          }
        else
          {
          if (recorded_SR < MAX_REPLAY_SR)
            {
            snprintf(rtstr, mxRstr, "Direct");
            }
          else
            {
            snprintf(rtstr, mxRstr, "1/20");
            }
          }
        }
      else
        {
        snprintf(rtstr, mxRstr, "%s %s", RightBaseMenu[EncRight_menu_idx].menu_txt, SR[oper_SR].txt);
        }
      }

    tft.setTextColor(f_col);
    tft.fillRect(ILI9341_TFTWIDTH / 2, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART, ILI9341_TFTWIDTH / 2, BOTTOM_LOWERPART, b_col);
    txt_rightAlign(rtstr, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART + 2);

    } // END of regular SETTINGS
  }

FLASHMEM
void showPushButtons()
  { //clear the line

  tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART); //position of button functions
  tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART, ILI9341_TFTWIDTH, BOTTOM_LOWERPART, BCK_MENU_COLOR);

  tft.setTextColor(ENC_MENU_COLOR);
  //on the left
  if (LeftButton_Mode == MODE_DISPLAY)
    {
    tft.print(setDisplay[display_mode]);
    }

  if (LeftButton_Mode == MODE_REC)
    {
    if (SD_ACTIVE)
      {
      if (recorderActive == false)
        {
        tft.print("Start REC");
        }
      else
        {
        tft.print("Stop REC");
        }
      }
    else
      {
      tft.print("NO REC");
      }
    }

  if (LeftButton_Mode == MODE_PLAY)
    {
    if (!root_active)
      {
      if (playActive == false)
        {
        tft.print("PLAY");
        }
      else
        {
        tft.print("STOP");
        }
      }

    else //we are selecting directories
      {
      tft.print("SELECTDIR");
      }
    }

  //on the right side of the pushbutton menu
  if (not recorderActive)
    {
    tft.setTextColor(ENC_MENU_COLOR);

    if ((AUTO_REC == false)) //dont show rightside menu choice when playing a file
      {
      char tstr[12];
      if (RightButton_Mode == MODE_DETECT)
        {
        snprintf(tstr, 12, "mode %s", DT_short[detector_mode]);
      #ifdef USE_TEFACTOR
        if ((detector_mode == detector_Auto_TE) and (detune_factor != 100))
          {
          snprintf(tstr, 12, "mode TE-HT");
        }
      #endif
        if ((LeftButton_Mode == MODE_PLAY) and (play_SR != MAX_play_SR))
          {
          snprintf(tstr, 12, "replay");
          }
        if ((LeftButton_Mode == MODE_PLAY) and (play_SR == MAX_play_SR))
          {
          if (recorded_SR > MAX_REPLAY_SR)
            {
            snprintf(tstr, 12, " ");
            }
          }
      }

      uint16_t sx = tft.strPixelLen(tstr);

      tft.setCursor(ILI9341_TFTWIDTH - sx, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 1);
      tft.print(tstr);
    }
  }
  }

void showSettingsButtons()
  {
  //clear lines
  tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART, ILI9341_TFTWIDTH, BOTTOM_LOWERPART, BCK_MENU_COLOR);
  tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART, ILI9341_TFTWIDTH, BOTTOM_LOWERPART, COLOR_BLACK);

  tft.setTextColor(HILIGHT_MENU_COLOR); //show special sidemenu indicator for time/date menus
  if ((set_menu_id[settings_page_nr] == SET_MENU_TIME) or (set_menu_id[settings_page_nr] == SET_MENU_DATE))
    {
    txt_rightAlign("sidemenu [ ]", ILI9341_TFTHEIGHT - BOTTOM_UPPERPART);
    }

  // if (SD_ACTIVE)
  // {if ((set_menu_id[settings_page_nr]==SET_MENU_SD_PLAYFOLDER) or (set_menu_id[settings_page_nr]==SET_MENU_SD_RECFOLDER))
  //   { txt_rightAlign("[confirm]",ILI9341_TFTHEIGHT-BOTTOM_UPPERPART);

  //   }
  // }
  //text on the bottom
  tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_LOWERPART);
  tft.print("MENU ^");

  txt_rightAlign("change/Save", ILI9341_TFTHEIGHT - BOTTOM_LOWERPART);
  }
void showpushtoStop()
  {
  tft.setCursor(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART); //position of button functions
  tft.fillRect(0, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART, ILI9341_TFTWIDTH, BOTTOM_LOWERPART, BCK_MENU_COLOR);
  tft.setFont(MENU_FONT);
  tft.print("<< PUSH TO STOP");
  }

void showSaved()
  {
  tft.setTextColor(COLOR_BLACK);
  tft.fillRect(150, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 2, ILI9341_TFTWIDTH - 150, 16, COLOR_WHITE);
  tft.fillCircle(150, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 10, 7, COLOR_WHITE);
  tft.setCursor(150, ILI9341_TFTHEIGHT - BOTTOM_UPPERPART + 3);

  tft.print(" Saved");
  tft.setTextColor(COLOR_WHITE);
  }

void startRecordScreen()
  {
  tft.setScroll(0); //stop scrolling
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(ENC_MENU_COLOR);
  tft.setFont(Arial_18);
  tft.setCursor(0, 0);

  if (AUTO_REC)
    {
    tft.println("AUTO RECORD");
    tft.setFont(Arial_13);
    tft.print(" # ");
    tft.print(autocounter);
    showpushtoStop();
    }
  else
    {
    tft.println("RECORDING");
    tft.setFont(Arial_13);
    tft.print(guano_filename); //dont show directory
    }

  if (!AUTO_REC)
    {
    showPushButtons();
    }
  tft.setFont(MENU_FONT);
  }

void updateAUTORECstatus()
  {
  tft.setFont(TFT_FONT);
  tft.setTextColor(ENC_MENU_COLOR);
  showpushtoStop();
  tft.fillRect(0, ILI9341_TFTHEIGHT - TFT_FONT.cap_height, ILI9341_TFTWIDTH, TFT_FONT.cap_height, COLOR_BLACK);
  tft.setCursor(0, ILI9341_TFTHEIGHT - TFT_FONT.cap_height);
  //ms            //blocks in ms
  if (recording_stopped < (AREC_P * AREC_5SEC)) //show the time left to wait until the next recording can start
    {
    uint8_t p = int((AREC_P * AREC_5SEC - recording_stopped) / 1000);
    tft.print("PAUSE (s) ");
    tft.print(p);
    }
  else
    {
    tft.print("WAIT for signal ");
    tft.print("rec#");
    tft.print(autocounter + 1);
    }
  }

void initTFT()
  {
  tft.setRotation(0);
  tft.setScroll(0);
  tft.setTextColor(COLOR_WHITE);
  tft.fillScreen(COLOR_BLACK);
  tft.setCursor(0, 0);

  tft.setScrollMargins(TOP_OFFSET, BOTTOM_UPPERPART + BOTTOMSCALE);
  tft.setTextWrap(false);
  setcolourmap(colorscheme);
#ifdef SCROLL_HORIZONTAL
  memset(frameBuffer, 0, sizeof(frameBuffer));
#endif
  }

#endif //ifdef ILI9341

#endif