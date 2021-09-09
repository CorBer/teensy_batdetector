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

#ifndef _BAT_FFT_H
#define _BAT_FFT_H

#include "bat_vars.h"
#include "bat_tft.h"
long last_FFT_count = 0;
long FFT_count = 0;

int idx_t = 0;
int idx = 0;
int64_t sum;
float32_t mean;
uint32_t FFT_bin[250];

int16_t FFT_max1 = 0;
uint32_t FFT_max_bin1 = 0;
//int16_t FFT_mean1 = 0;
int16_t FFT_max2 = 0;
uint32_t FFT_max_bin2 = 0;
//int16_t FFT_mean2 = 0;
int16_t FFT_bat[3]; // max of 3 frequencies are being displayed
int16_t index_FFT;
int l_limit;
int u_limit;
int index_l_limit;
int index_u_limit;

boolean fftSpectrumAvailable = false;

//colorvalues for the pixels of the waterfall graph go here
uint16_t FFT_pixels[240];

//powerspectrumMaxcount is used to update the powerspectrumgraph, the graph gets update when x powerspectra have been collected
const uint16_t powerspectrumMaxcounts = 500; //was 100 change 20210429
uint16_t powerspectrumCounter = 0;

//storage for the powerspectrum
uint32_t FFTpowerspectrum[240];

uint32_t powerspectrum_Max = 0;

//check if the incoming should has ultrasound in the requested spectrumrange and with a clear peak
void updateFFTanalysis()
  {
  uint8_t old_LoF_bin = signal_LoF_bin;

  if ((playActive) and (!playDirect))
    { //adapt to the original recording samplerate
    signal_LoF_bin = int((TE_low * 1000.0) / (recorded_SR / (FFT_points * 2)));
    }

  //float peak=350;
  uint32_t peak = TE_peak * 25;
  if (TE_peak == 0)
    {
    peak = (uint32_t)11.89f * exp(0.1035f * float(mic_gain)); //estimated peak
    //D_PRINTXY("Auto thresh ",peak);
    if (peak > 150)
      {
      peak = 150;
      }
    }

  if (AUTO_REC)
    {
    peak = 25 * AREC_S;
    }


  FFT_count++;
  float above_average = 1.5;
  uint32_t measured_peak = 0;
  uint32_t avgFFTbin = 0;
  uint32_t val = 0;
  uint8_t bincount = 1;

  FFT_peakF_bin = 0;
  //uint16_t i_val;
  //scan the FFT for a peak
  uint16_t FFToutput[128];
  memcpy(&FFToutput, &myFFT.output, 256);

  //takes 5.7 microseconds -> optimized to 2.6 microseconds by changing float to INT
  //210808_adapted and optimized code
  uint8_t idx;
  for (uint8_t i = 2; i < 120; i++)
    { //i_val=FFToutput[i]; //1.8microseconds
    idx = i * 2;
    FFT_bin[idx] = FFToutput[i] * 10 + 10;                          //0.4 microseconds
    cumsumPowerspectrum[i] = cumsumPowerspectrum[i] + FFToutput[i]; //0.4microseconds
    if (idx > signal_LoF_bin)                                       //search above the low threshold
      {                                                               //val = FFT_bin[idx]; //only recent data
      avgFFTbin += FFT_bin[idx];
      bincount++;
      //detect if the peak at peakfrequency is above the peak-threshold
      measured_peak = max(measured_peak, FFT_bin[idx]);
      //if (FFT_bin[idx]>measured_peak)
      //  {measured_peak= FFT_bin[idx];}
      if ((FFT_bin[idx] > peak))
        {
        peak = FFT_bin[idx];
        FFT_peakF_bin = idx; //record peakbin
        }
      }
    }

  current_peakPower = peak;

  if (AGC_mode > 0) //AGC is active
    {

    if (peak > AGC_maxpeak) //too strong signal
      {

      if (AGC_unchanged > AGC_decay * 100) //too long alleady;
        {
        mic_gain = constrain(mic_gain - 1, AGC_minGAIN, AGC_maxGAIN);
        AGC_unchanged = 0;
        set_mic(mic_gain);
        AGC_change = true;
        }
      }

    if (measured_peak < AGC_minpeak) //too weak signal increase gain if this is allready some time the case
      {
      if (AGC_unchanged > AGC_attack * 100) //too long alleady;
        {
        mic_gain = constrain(mic_gain + 1, AGC_minGAIN, AGC_maxGAIN);
        AGC_unchanged = 0; //micros without change
        set_mic(mic_gain);
        AGC_change = true;
        if (mic_gain < AGC_low)
          {
          AGC_low = mic_gain;
          }
        if (mic_gain > AGC_high)
          {
          AGC_high = mic_gain;
          }
        }
      }
    } //AGCMODE

  //if a strong enough peak was detected
  if (FFT_peakF_bin > 0)
    {
    avgFFTbin = (avgFFTbin - peak) / bincount; //calculate average signalvalue above the threshold
    if ((peak / avgFFTbin) < above_average)    //very low peakvalue vs average signal so probably mainly noise, dont detect this as a peak
      {
      FFT_peakF_bin = 0;
      }
    }
  //if a peak was found
  if (FFT_peakF_bin > 0)
    {
    fftSpectrumAvailable = true;
    //dampend the effect of normalized spectra
    }

  //create a new spectrum for the screen if a peak is available
  if (fftSpectrumAvailable)
    {
    //interpolate the bins
    for (int i = spec_lo; i < 118; i++)                               //3 microseconds
      FFT_bin[i * 2 + 1] = (FFT_bin[i * 2] + FFT_bin[i * 2 + 2]) / 2; //lineair interpolate halfbins

    for (int i = spec_lo; i < spec_hi; i++)
      {
      uint8_t pixpos = (i - spec_lo);
      val = FFT_bin[i];
      val = constrain(val, (uint)0, (uint)9000);
      val = val / 10;

      if (display_mode == waterfallgraph)
        {
        FFT_pixels[pixpos] = colourmap[uint16_t(val)];
        }
      if (display_mode == h_waterfallgraph)
        {
        FFT_pixels[spec_hi - pixpos] = colourmap[uint16_t(val)];
        }

      }
    //IPI estimator
    if (current_peakPower > pulse_peakPower)
      {
      pulse_peakPower = current_peakPower; //keep track of the peakvalue during the pulse to estimate IPI
      }

    if ((current_peakPower < pulse_peakPower * 0.25) and (found_peakdrop == false)) //25% level
      {
      callLength = time_since_StartDetection;

      found_peakdrop = true;

      if (calls_detected <= 1) //restarted
        {
        total_callduration = 0;
        intercallduration = 0;
        total_peakpower = 0;
        }

      if (callLength < 50) //only calls shorter than 50ms are to be kept
        {
        calls_detected++;
      #ifdef PEAKBIN_SERIAL
        D_PRINTXY("PEAKBIN ", int(FFT_peakF_bin * SR_FFTratio * 0.001f + SR_FFTratio * 0.0005f));
      #endif
        total_callduration += callLength;
        total_peakpower += pulse_peakPower; //use the peakpowers to estimate signalstrength
        pulse_peakPower = 0;
        }
      }

    if ((time_since_EndDetection > 10) and (time_since_StartDetection < 5) and (showStart == false))
      {
      if (time_since_tick < 1000)
        {
        intercallduration += time_since_tick;
        ticks_detected++;
        }
      showStart = true;

      time_since_tick = 0; //reset_tick
      }
    }

  powerSpectrum_Maxbin = 0;
  sample_UltraSound = false;

  // detected a peak in the selected frequencies between hi and lo
  if ((FFT_peakF_bin > signal_LoF_bin))
    { //collect data for the powerspectrum
    sample_UltraSound = true;
    powerspectrum_Max = 0;
    if ((display_mode == waterfallgraph) or (display_mode == h_waterfallgraph) or (display_mode == spectrumgraph))
      {                                //only sum when the signal is not too strong
      if (current_peakPower < 40000) // do not use to strong signals as they will have distorted peakvalues
        {
        for (int i = 0; i < spec_hi; i++)
          {                                    //add new samples
          FFTpowerspectrum[i] += FFT_bin[i]; //cumulative collection of samples
          //keep track of the maximum
          if ((FFTpowerspectrum[i] > powerspectrum_Max) and (i > signal_LoF_bin))
            {
            powerspectrum_Max = FFTpowerspectrum[i];
            powerSpectrum_Maxbin = i;
            }
          }

        if (powerSpectrum_Maxbin > signal_LoF_bin) //follow powerspectrum
          {
          FFT_peakF_bin = powerSpectrum_Maxbin;
          }

        //keep track of the no of samples with bat-activity
        powerspectrumCounter++;
        }
      }
    }

  if (playActive)
    {
    signal_LoF_bin = old_LoF_bin;
    }
  }

#endif