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
int16_t FFT_bin [128];
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



uint16_t FFT_pixels[240];
//const uint16_t FFT_points = 1024;

//int barm [512];

uint16_t powerspectrumCounter=0;

float FFTpowerspectrum[128];
float powerspectrum_Max=0;
static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
		return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}

void updateFFTanalysis()
        {
       
        FFT_peakF_bin=0;
        int peak=512;
        if (AUTO_REC)
            {peak=512*AREC_S;
              }
          // there are 128 FFT different bins only 120 are shown on the graphs
        FFT_count++;
        int avgFFTbin=0;
        for (int i = spec_lo; i < spec_hi; i++) 
          {
            int val = myFFT.output[i]*10 + 10; //v1
            //only check for peaks above the lo_bin frequencies
            if (i>signal_LoF_bin)
            {
              avgFFTbin+=val;
              //detect if the peakfrequency is above the threshold and above the lo_bin setting
              if ((val>peak) )
                { peak=val;
                  FFT_peakF_bin=i;
                }
            }
            
            if (display_mode==waterfallgraph)
                {uint8_t pixpos=(i-spec_lo)*spec_width;
                FFT_pixels[pixpos] = color565(
                           min(200, val*2), //very low values will show as red
                          (val/3>255)? 255 : val/3, // up to 3*255 will be coloured a mix of red/green (yellow) 
                           val/9>255 ? 255:  val/9 ); // uo to 9*255 will be coloured a mix of red/green/blue (white)
                FFT_pixels[pixpos+1]=FFT_pixels[pixpos];
                }   
          }

        avgFFTbin=avgFFTbin/(spec_hi-signal_LoF_bin);
        // if (FFT_peakF_bin>0)
        // {
        //   D_PRINTLN(FFT_peakF_bin);
        // }
        if ((peak/avgFFTbin)<1.1) //very low peakvalue so probably a lot of noise, dont detect this as a peak
            { FFT_peakF_bin=0;
            }  
        
       

        powerSpectrum_Maxbin=0;
        sample_UltraSound= false;  
        // detected a peak in the selected frequencies
        if ((FFT_peakF_bin>signal_LoF_bin) and (FFT_peakF_bin<signal_HiF_bin))
          { //collect data for the powerspectrum
            sample_UltraSound=true;
            
            if ((display_mode==waterfallgraph) or (display_mode==spectrumgraph))
                {  for (int i = spec_lo; i < spec_hi; i++)
                  {   //add new samples
                      FFTpowerspectrum[i]+=myFFT.output[i];
                      //keep track of the maximum
                      if (FFTpowerspectrum[i]>powerspectrum_Max)
                        { powerspectrum_Max=FFTpowerspectrum[i];
                          powerSpectrum_Maxbin=i;
                        }
                  }
                //keep track of the no of samples with bat-activity
                powerspectrumCounter++;
                }
          }    
  
        }
#endif
