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
#ifndef _BAT_GPS_H
#define _BAT_GPS_H

#include "SparkFun_u-blox_GNSS_Arduino_Library.h"

#include "TimeAltLib.h"

 //local 
#include "bat_tft.h"
#include "bat_vars.h"

SFE_UBLOX_GNSS myGNSS;

UBX_NAV_PVT_data_t GPSdata;

#if defined(__MK66FX1M0__)
HardwareSerial GPSuart = HardwareSerial(0);
#endif

#if defined(__IMXRT1062__)
#define GPSuart Serial1
#endif

uint16_t Mrate;

// void SerialTime()
//  { 
//    Serial.printf("%02d:%02d:%02d",hour(),minute(),second());
//  }

void syncGPStime(time_t GPS_time)
  {
  dstactive = false;
  byte yy = year(GPS_time) % 100; // adjust your functions call
  byte mm = month(GPS_time);
  byte dd = day(GPS_time);
  //EU
  byte x1 = 31 - (yy + yy / 4 - 2) % 7; //last Sunday March
  byte x2 = 31 - (yy + yy / 4 + 2) % 7; // last Sunday October
  //US
  byte x = (yy + yy / 4 + 2) % 7;
  if (gps_dstzone == 0)
    {
    if ((mm > 3 && mm < 10) || (mm == 3 && dd >= x1) || (mm == 10 && dd < x2))
      {
      dstactive = true;
      }
    }
  if (gps_dstzone == 1)
    {
    if ((mm > 3 && mm < 11) || (mm == 3 && dd >= (14 - x)) || (mm == 11 && dd < (7 - x)))
      {
      dstactive = true;
      }
    }

  time_t setT = GPStime + gps_utcoff * 3600 + dstactive * 3600;
  Teensy3Clock.set(setT);
  setTime(setT);
  syncGPS_time = true;
  }

// ************************  start the GPS by trying to find the current baudrate. 
// Communication will be at 115K
// after connection set the default parameters for GPS

void initGPS()
  {
  uint32_t oldbaud = 115200;
  uint32_t newbaud = 115200;

  //all possible bauds for Ublox
  uint32_t bauds[7] = {9600, 19200, 38400, 57600, 115200, 230400, 460800};

  //test current baud
  boolean Sconnected = false;
  boolean skip = false;
  micropushButton_L.update();
  if ((micropushButton_L.fallingEdge()) or (micropushButton_L.read() == 0))
    {
    skip = true;
    }
  if (not skip)
    {
    D_PRINTXY("GNSS: trying oldbaud ", oldbaud);
    //uint32_t start = millis();
    GPSuart.begin(oldbaud);
    if (myGNSS.begin(GPSuart) == true)
      {
      Sconnected = true;
      }

    }
  D_PRINTXY("SKIP ", skip);
  int8_t br = 0;
  while ((!Sconnected) and (br < 7) and (not skip))
    {
    micropushButton_L.update();
    if ((micropushButton_L.fallingEdge()) or (micropushButton_L.read() == 0))
      {
      skip = true;
      }

    if (br < 7)
      {
      D_PRINTXY("GNSS: trying all baudrates ", bauds[br]);
      GPSuart.begin(bauds[br]);
      if (myGNSS.begin(GPSuart) == true)
        {
        Sconnected = true;
        oldbaud = bauds[br];
        }
      else
        {
        br++;
        }
      }
    }

  //we should be connected now
  if ((myGNSS.begin(GPSuart) == true) and (Sconnected))
    {
    D_PRINTXY("GNSS: switching to new baud", newbaud);
    myGNSS.setSerialRate(newbaud);
    delay(100);
    GPSuart.begin(newbaud);
    if (myGNSS.begin(GPSuart) == true)
      {
      GPSconnected = true;
      GPSbaudOK = true;
      }
    }

  if (GPSbaudOK)
    {
    //myGNSS.hardReset();
    myGNSS.warmReset();

    D_PRINT("Ublox Protocol version:");
    D_PRINT(myGNSS.getProtocolVersionHigh());
    D_PRINT(".");
    D_PRINTLN(myGNSS.getProtocolVersionLow());

    if (myGNSS.powerSaveMode(false))
      {
      D_PRINTLN("Power Save Mode disabled.");
      }

    // Make sure GPS is enabled (we must leave at least one major GNSS enabled!)
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);    // Disable SBAS
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO); // Disable Galileo
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);  // Disable BeiDou
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);    // Disable IMES
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);    // Disable QZSS
    myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
    myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS); // Disable GLONASS

    myGNSS.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
    time_t dt = now() - gps_utcoff * 3600 - dstactive * 3600;
    myGNSS.setUTC(dt); //update the RTC of the GPSmodule

  #ifdef DEBUG
    Serial.print("PMS ");
    myGNSS.getpayload(UBX_CLASS_CFG, UBX_CFG_PMS);
    Serial.print("PMS2 ");
    myGNSS.getpayload(UBX_CLASS_CFG, UBX_CFG_PM2);
    Serial.print("setUTC ");
    D_TIME();
    Serial.println();
  #endif

    //only look for satellites 10 degrees above the horizon and aim at a auto 2d/3d fix
    myGNSS.setminElev_Fix(10, 3); //1=2d fix 2=3d fix, 3=2d or 3d
    myGNSS.setDynamicModel(DYN_MODEL_PORTABLE); // alt until 12000m, max Hspeed 310m/s max Vspeed 50m/s
    myGNSS.setMeasurementRate(100); //time in ms
    myGNSS.setNavigationRate(1);    //this calculates a navigation solution every second (rate should be an EVEN number !!)
    myGNSS.powerSaveMode(0);

    myGNSS.saveConfiguration(); //necessary otherwise startup afterwards will not work

//0 full 1 balanced 2 interval 3 agressive 1hz 4 agressize 2hz 5 agressive 4hz
  #ifdef DEBUG
    Serial.print("PMS ");
    myGNSS.getpayload(UBX_CLASS_CFG, UBX_CFG_PMS);
    Serial.print("PMS2 ");
    myGNSS.getpayload(UBX_CLASS_CFG, UBX_CFG_PM2);
  #endif
    }
  else
    {
    D_PRINTLN("GPS not connected at 115200");
    }
  }


//************************** UPDATE GPS data
boolean readGPS()
  {
  if (myGNSS.getPVT(200)) //check if new data is available
    {
    gps_fix = myGNSS.packetUBXNAVPVT->data.fixType;

    if ((gps_fix > 0) and (gps_fix < 6))
      {
      D_PRINT(gps_fix);
      D_PRINT(" ");
      gps_SIV = myGNSS.packetUBXNAVPVT->data.numSV;

      GPStime = ((((((((uint32_t)myGNSS.packetUBXNAVPVT->data.year - 1970) * 365) + ((((uint32_t)myGNSS.packetUBXNAVPVT->data.year - 1970) + 3) / 4)) +
        DAYS_SINCE_MONTH[((uint32_t)myGNSS.packetUBXNAVPVT->data.year - 1970) & 3][(uint32_t)myGNSS.packetUBXNAVPVT->data.month] +
        ((uint32_t)myGNSS.packetUBXNAVPVT->data.day - 1)) * 24
        + (uint32_t)myGNSS.packetUBXNAVPVT->data.hour) * 60
        + (uint32_t)myGNSS.packetUBXNAVPVT->data.min) * 60
        + (uint32_t)myGNSS.packetUBXNAVPVT->data.sec);

      snprintf(gps_time, 20, "%04d%02d%02d %02d:%02d:%02d", year(GPStime), month(GPStime), day(GPStime), hour(GPStime), minute(GPStime), second(GPStime));

      GPSfix_time = GPStime + gps_utcoff * 3600 + dstactive * 3600; //this is needed to keep track of the time since the last fix

      if (syncGPS_time == false) //only do this once !
        {
        syncGPStime(GPStime);
        }

      gps_HACC = long(myGNSS.packetUBXNAVPVT->data.hAcc);
      gps_latitude = myGNSS.packetUBXNAVPVT->data.lat;
      gps_longitude = myGNSS.packetUBXNAVPVT->data.lon;
      if (gps_fix == 3)
        {
        gps_altitude = myGNSS.packetUBXNAVPVT->data.hMSL;
        }
      gps_head = myGNSS.packetUBXNAVPVT->data.headMot;
      gps_speed = myGNSS.packetUBXNAVPVT->data.gSpeed;
      }
    return true;
    }
  else
    {
    return false;
    }
  }

#endif
