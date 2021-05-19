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

#ifndef _BAT_FFT_H
#define _BAT_FFT_H

#include "bat_vars.h"
#include "bat_tft.h"

long FFT_count=0;

int idx_t = 0;
int idx = 0;
int64_t sum;
float32_t mean;
float FFT_bin [240];
int16_t FFT_max1 = 0;
uint32_t FFT_max_bin1 = 0;
int16_t FFT_mean1 = 0;
int16_t FFT_max2 = 0;
uint32_t FFT_max_bin2 = 0;
int16_t FFT_mean2 = 0;
int16_t FFT_bat [3]; // max of 3 frequencies are being displayed
int16_t index_FFT;
int l_limit;
int u_limit;
int index_l_limit;
int index_u_limit;

boolean fftSpectrumAvailable=false;

uint16_t FFT_pixels[240];
const uint16_t powerspectrumMaxcounts=100;
uint16_t powerspectrumCounter=0;

float FFTpowerspectrum[240];
float powerspectrum_Max=0;

static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
		return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}

unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red
  value=value/2;
  byte red = 0;         // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;        // Green is the middle 6 bits
  byte blue = 0;         // Blue is the bottom 5 bits

  byte quadrant = value / 32;
  byte modquad= value %32;

  if (quadrant == 0) {
          blue = 31;
          green = 2 * modquad;
          red = 0;
  }
  if (quadrant == 1) {
          blue = 31 - modquad;
          green = 63;
          red = 0;
  }
  if (quadrant == 2) {
          blue = 0;
          green = 63;
          red = modquad;
  }
  if (quadrant == 3) {
          blue = 0;
          green = 63 - 2 * (modquad);
          red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}


void updateFFTanalysis()
{
  uint8_t old_LoF_bin=signal_LoF_bin;

  if ((playActive) and (!playDirect)) 
    { //adapt to the original recording samplerate
      signal_LoF_bin= int((TE_low*1000.0)/(recorded_SR/(FFT_points*2)));
    }

  //float peak=350;
  float peak=FFT_peak;

  if (AUTO_REC)
      {peak=peak*AREC_S;
          }
  
  FFT_count++;
  float above_average=2;
 
  float avgFFTbin=0;
  float val=0;
  uint8_t bincount=1;
  FFT_peakF_bin=0;
  
  //scan the FFT for a peak and also build the sum for the average signal above the lower-threshold
  for (uint8_t i = 2; i < 120; i++) 
    { 
         FFT_bin[i*2]=(myFFT.output[i])*10+10;
     
      val = FFT_bin[i*2]; //only recent data
      if (i*2>signal_LoF_bin) //above the low threshold
          { avgFFTbin+=val;
            bincount++;
            //detect if the peak at peakfrequency is above the peak-threshold 
            if ((val>peak) )
              { peak=val;
                FFT_peakF_bin=i*2; //record peakbin 
              }
          }
    }
    
  //if a strong enough peak was detected
  if (FFT_peakF_bin>0)  
   { avgFFTbin=(avgFFTbin-peak)/bincount; //calculate average signalvalue above the threshold
     if ((peak/avgFFTbin)<above_average) //very low peakvalue vs average signal so probably mainly noise, dont detect this as a peak
                { FFT_peakF_bin=0;
                }  
   }  
  //if a peak was found 
  if (FFT_peakF_bin>0)
     {fftSpectrumAvailable=true;
       //dampend the effect of normalized spectra
      
      }
   
  //create a new spectrum for the screen if a peak is available
  if (fftSpectrumAvailable)
    { 
      //interpolate the bins 
      for (int i = spec_lo; i < 118; i++)   
            FFT_bin[i*2+1]=(FFT_bin[i*2]+FFT_bin[i*2+2])*0.5; //lineair interpolate halfbins
      
      for (int i = spec_lo; i < spec_hi; i++)  
                {  uint8_t pixpos=(i-spec_lo);
                 
                   val=FFT_bin[i];
                                       
                   FFT_pixels[pixpos] = 
                             color565(
                              min(250, val/3), //very low values will show as red
                             (val/10)>255  ? 255 : val/10, // greens //was 3
                             (val/40)>255 ? 255 : val/40 ); // blues //was 12

                  }
                
    }

  powerSpectrum_Maxbin=0;
  sample_UltraSound= false;  
// detected a peak in the selected frequencies between hi and lo
  if ((FFT_peakF_bin>signal_LoF_bin))
    { //collect data for the powerspectrum
      sample_UltraSound=true;
      powerspectrum_Max=0;
      if ((display_mode==waterfallgraph) or (display_mode==spectrumgraph))
          { //only sum above the 
            for (int i = signal_LoF_bin; i < spec_hi; i++)
                {   //add new samples
                    FFTpowerspectrum[i]+=FFT_bin[i];
                    //keep track of the maximum
                    if (FFTpowerspectrum[i]>powerspectrum_Max)
                      { powerspectrum_Max=FFTpowerspectrum[i];
                        powerSpectrum_Maxbin=i;
                      }
                }
             
             if (powerSpectrum_Maxbin>signal_LoF_bin) //follow powerspectrum
                {FFT_peakF_bin=powerSpectrum_Maxbin;   
                }

          //keep track of the no of samples with bat-activity
          powerspectrumCounter++;
          }
    }    

  if (playActive) 
    {signal_LoF_bin=old_LoF_bin;
    }

}

#endif