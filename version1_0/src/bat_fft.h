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

#ifndef _BAT_FFT_H
#define _BAT_FFT_H

#include "bat_vars.h"
#include "bat_tft.h"

long FFT_count=0;
int idx_t = 0;
int idx = 0;
int64_t sum;
float32_t mean;
float FFT_bin [240][4];
//TEST
int16_t FFT_max1 = 0;
uint32_t FFT_max_bin1 = 0;
int16_t FFT_mean1 = 0;
int16_t FFT_max2 = 0;
uint32_t FFT_max_bin2 = 0;
int16_t FFT_mean2 = 0;
//int16_t FFT_threshold = 0;
int16_t FFT_bat [3]; // max of 3 frequencies are being displayed
int16_t index_FFT;
int l_limit;
int u_limit;
int index_l_limit;
int index_u_limit;

boolean fftSpectrumAvailable=false;

uint16_t FFT_pixels[240];

uint16_t powerspectrumCounter=0;

float FFTpowerspectrum[240];
float powerspectrum_Max=0;
static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
		return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}


void updateFFTanalysis()
{
  
  float peak=512;
   
  if (AUTO_REC)
      {peak=512*AREC_S;
          }

  FFT_count++;

  float avgFFTbin=0;
  float val=0;
  float weight=1;
  uint8_t bincount=0;
  FFT_peakF_bin=0;
  
  //scan the FFT for a peak and also build the sum for the average signal above the lower-threshold
  for (uint i = 2; i < 120; i++) 
    { weight=1;//+(i/120); //add extra weight to higher frequencies to prefer detecting a peak higher up
      FFT_bin[i*2][0]=myFFT.output[i]*10+10;
      val = FFT_bin[i*2][0]; //only recent data
      if (i*2>signal_LoF_bin) //above the low threshold
          { avgFFTbin+=val*weight;
            bincount++;
            //detect if the peak at peakfrequency is above the peak-threshold 
            if ((val*weight>peak) )
              { peak=val*weight;
                FFT_peakF_bin=i*2; //record peakbin 
              }
          }
    }
  //if a strong enough peak was detected
  if (FFT_peakF_bin>0)  
   { avgFFTbin=avgFFTbin/(bincount+0.001); //calculate average signalvalue above the threshold
     if ((peak/avgFFTbin)<2) //very low peakvalue vs average signal so probably mainly noise, dont detect this as a peak
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
            FFT_bin[i*2+1][0]=(FFT_bin[i*2][0]+FFT_bin[i*2+2][0])*0.5; //lineair interpolate halfbins
      
      for (int i = spec_lo; i < spec_hi; i++)  
                {  uint8_t pixpos=(i-spec_lo);
                 
                   val=(FFT_bin[i][0]);
                   FFT_bin[i][1]=FFT_bin[i][0]; //store for the next run
                   
                   FFT_pixels[pixpos] = color565(
                             min(200, val/2), //very low values will show as red
                            (val/5)>255? 255 : val/5, // up to 3*255 will be coloured a mix of red/green (yellow) 
                            (val/12)>255 ? 255:  val/12 ); // uo to 9*255 will be coloured a mix of red/green/blue (white)

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
                    FFTpowerspectrum[i]+=FFT_bin[i][0];
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
  

}

#endif