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

#ifndef _BAT_CONFIG_H
#define _BAT_CONFIG_H

 // this setup allows storage of most of the settings that are also in EEprom into an SDfile (text).
 // this can be used for "long term storage" so that at startup a user can reset to "stored" settings

#include "SDConfig.h" // libraryfile

//necessary links to local files
#include "bat_vars.h"
#include "bat_audio.h"
#include "bat_encoder.h"
#include "bat_debug.h"

char configFile[] = "TB_V13.cfg"; // filename to be used for the storage. TB_V13-> Teensybat V1_3

//***************************************** read a stored configuration from the SD
boolean readConfiguration()
  {
  /*
   * Length of the longest line expected in the config file.
   * The larger this number, the more memory is used
   * to read the file.
   * You probably won't need to change this number.
   */
  int maxLineLength = 127;
  SDConfig cfg;
  // Open the configuration file.
  D_PRINTXY("trying to read ", configFile)
    if (!cfg.begin(configFile, maxLineLength))
      {
      Serial.print("Failed to open configuration file: ");
      Serial.println(configFile);
      return false;
      }

  // Read each setting from the file.

  while (cfg.readNextSetting())
    {
    // D_PRINTXY(cfg.getName(),cfg.getIntValue());

    if (cfg.nameIs("detector_mode"))
      {
      detector_mode = cfg.getIntValue();
      }
    else if (cfg.nameIs("record_detector"))
      {
      record_detector = cfg.getIntValue();
      }

    //else if (cfg.nameIs("display_mode")){display_mode=cfg.getIntValue(); }

    else if (cfg.nameIs("play_SR"))
      {
      play_SR = cfg.getIntValue();
      }
    else if (cfg.nameIs("Arec_SR"))
      {
      Arec_SR = cfg.getIntValue();
      }
    else if (cfg.nameIs("oper_SR"))
      {
      oper_SR = cfg.getIntValue();
      }

    else if (cfg.nameIs("mic_gain"))
      {
      mic_gain = cfg.getIntValue();
      }
    else if (cfg.nameIs("volume"))
      {
      volume = cfg.getIntValue();
      }
    else if (cfg.nameIs("osc_frequency"))
      {
      osc_frequency = cfg.getIntValue();
      } // for Heterodyne the default oscillator frequency in Khz

    else if (cfg.nameIs("HI_Pass"))
      {
      HI_pass = cfg.getIntValue();
      }
    else if (cfg.nameIs("HI_Pass_stage"))
      {
      HI_pass_stages = cfg.getIntValue();
      }

    else if (cfg.nameIs("TE_speed"))
      {
      TE_speed = cfg.getIntValue();
      }
    else if (cfg.nameIs("TE_low"))
      {
      TE_low = cfg.getIntValue();
      }
    else if (cfg.nameIs("TE_GAP"))
      {
      TE_GAP = cfg.getIntValue();
      }
    else if (cfg.nameIs("TE_peak"))
      {
      TE_peak = cfg.getIntValue();
      }
    else if (cfg.nameIs("FD_divider"))
      {
      FD_divider = cfg.getIntValue();
      }
    else if (cfg.nameIs("detune_factor"))
      {
      detune_factor = cfg.getIntValue();
      }

    else if (cfg.nameIs("AREC_D"))
      {
      AREC_D = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_P"))
      {
      AREC_P = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_B"))
      {
      AREC_B = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_F"))
      {
      AREC_F = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_S"))
      {
      AREC_S = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_G"))
      {
      AREC_G = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_AGC_MODE"))
      {
      AREC_AGC_MODE = cfg.getIntValue();
      }
    else if (cfg.nameIs("AREC_PREBUFFER"))
      {
      AREC_PREBUFFER = cfg.getIntValue();
      }

    else if (cfg.nameIs("AGC_mode"))
      {
      AGC_mode = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_maxpeak"))
      {
      AGC_maxpeak = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_minpeak"))
      {
      AGC_minpeak = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_attack"))
      {
      AGC_attack = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_decay"))
      {
      AGC_decay = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_maxGAIN"))
      {
      AGC_maxGAIN = cfg.getIntValue();
      }
    else if (cfg.nameIs("AGC_minGAIN"))
      {
      AGC_minGAIN = cfg.getIntValue();
      }

    else if (cfg.nameIs("use_presets"))
      {
      use_presets = cfg.getIntValue();
      }

    else if (cfg.nameIs("ENCODER_TURN"))
      {
      ENCODER_TURN = cfg.getIntValue();
      }
    else if (cfg.nameIs("ENCODER_DIVIDER"))
      {
      ENCODER_DIVIDER = cfg.getIntValue();
      }

    else if (cfg.nameIs("colorscheme"))
      {
      colorscheme = cfg.getIntValue();
      }
    else if (cfg.nameIs("COLOR_MENU"))
      {
      cwheelpos[0] = cfg.getIntValue();
      }
    else if (cfg.nameIs("COLOR_HILIGHTMENU"))
      {
      cwheelpos[1] = cfg.getIntValue();
      }
    else if (cfg.nameIs("COLOR_BACKMENU"))
      {
      cwheelpos[2] = cfg.getIntValue();
      }

    else if (cfg.nameIs("tft_backlight"))
      {
      tft_backlight = cfg.getIntValue();
      }
    else if (cfg.nameIs("tft_sleep"))
      {
      tft_sleep = cfg.getIntValue();
      }

    else if (cfg.nameIs("gps_utcoff"))
      {
      gps_utcoff = cfg.getIntValue();
      }
  #ifdef USE_DS18B20
    else if (cfg.nameIs("T_correct"))
      {
      T_corrector = cfg.getIntValue() / 100;

      }
  #endif
    else
      {
      // report unrecognized names.
      Serial.print("Unknown name in config: ");
      Serial.println(cfg.getName());
      }
    }
  // clean up
  cfg.end();
  return true;
  }

//*****************************************write a configuration to the SD
boolean writeConfiguration()
  {
  if (SD_ACTIVE)
    {
    File config = SD.open(configFile, FILE_WRITE_BEGIN);

    config.printf("detector_mode= %d\n", detector_mode);
    config.printf("record_detector= %d\n", record_detector);
    //config.printf("display_mode= %d\n", display_mode);

    config.printf("play_SR= %d\n", play_SR);
    config.printf("Arec_SR= %d\n", Arec_SR);
    config.printf("oper_SR= %d\n", oper_SR);

    config.printf("mic_gain= %d\n", mic_gain);
    config.printf("volume= %d\n", volume);
    config.printf("osc_frequency= %d\n", osc_frequency); // for Heterodyne the default oscillator frequency in Khz

    config.printf("HI_Pass= %d\n", HI_pass);
    config.printf("HI_Pass_stage= %d\n", HI_pass_stages);

    config.printf("TE_speed= %d\n", TE_speed);
    config.printf("TE_low= %d\n", TE_low);
    config.printf("TE_GAP= %d\n", TE_GAP);
    config.printf("TE_peak= %d\n", TE_peak);
    config.printf("FD_divider= %d\n", FD_divider);
    config.printf("detune_factor= %d\n", detune_factor);

    config.printf("AREC_D= %d\n", AREC_D);
    config.printf("AREC_P= %d\n", AREC_P);
    config.printf("AREC_B= %d\n", AREC_B);
    config.printf("AREC_F= %d\n", AREC_F);
    config.printf("AREC_S= %d\n", AREC_S);
    config.printf("AREC_G= %d\n", AREC_G);
    config.printf("AREC_AGC_MODE= %d\n", AREC_AGC_MODE);
    config.printf("AREC_PREBUFFER= %d\n", AREC_PREBUFFER);

    config.printf("AGC_mode= %d\n", AGC_mode);
    config.printf("AGC_maxpeak= %d\n", AGC_maxpeak);
    config.printf("AGC_minpeak= %d\n", AGC_minpeak);
    config.printf("AGC_attack= %d\n", AGC_attack);
    config.printf("AGC_decay= %d\n", AGC_decay);
    config.printf("AGC_maxGAIN= %d\n", AGC_maxGAIN);
    config.printf("AGC_minGAIN= %d\n", AGC_minGAIN);

    config.printf("use_presets= %d\n", use_presets);

    config.printf("ENCODER_TURN= %d\n", ENCODER_TURN);
    config.printf("ENCODER_DIVIDER= %d\n", ENCODER_DIVIDER);

    config.printf("colorscheme= %d\n", colorscheme);
    config.printf("COLOR_MENU= %d\n", cwheelpos[0]);
    config.printf("COLOR_HILIGHTMENU= %d\n", cwheelpos[1]);
    config.printf("COLOR_BACKMENU= %d\n", cwheelpos[2]);

    config.printf("tft_backlight= %d\n", tft_backlight);
    config.printf("tft_sleep= %d\n", tft_sleep);

    config.printf("gps_utcoff= %d\n", gps_utcoff);
  #ifdef USE_DS18B20
    config.printf("T_correct= %d\n", int(T_corrector * 100));

  #endif
    config.flush();
    config.close();

    return true;
    }
  else
    {
    return false;
    }
  }

#endif
