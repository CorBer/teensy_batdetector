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
float FFT_bin [250];
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


const uint16_t powerspectrumMaxcounts=50; //was 100 change 20210429
uint16_t powerspectrumCounter=0;

float FFTpowerspectrum[240];


float powerspectrum_Max=0;


void updateFFTanalysis()
{
    uint8_t old_LoF_bin=signal_LoF_bin;

  if ((playActive) and (!playDirect)) 
    { //adapt to the original recording samplerate
      signal_LoF_bin= int((TE_low*1000.0)/(recorded_SR/(FFT_points*2)));
    }

  //float peak=350;
  float peak=TE_peak*25; //

  if (AUTO_REC)
      {peak=25*AREC_S; 
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
      cumsumPowerspectrum[i]+=(myFFT.output[i]);
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
  current_peakPower=peak;
  
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
                   val=constrain(val,0,9000);
                   val=val/10;
                   FFT_pixels[pixpos]=colourmap[uint16_t(val)];                  
                  }

      if (current_peakPower>pulse_peakPower)
       {pulse_peakPower=current_peakPower; //keep track of the peakvalue during the pulse to estimate IPI
        }

      if ((current_peakPower<pulse_peakPower*0.25) and (found_peakdrop==false))  //25% level
         { callLength=time_since_StartDetection;
          
           found_peakdrop=true;
          //  for (int i = 0; i <5; i++) //yellow line indicating end of call
          //    { FFT_pixels[i]=color565(255,255,0); 
          //    }
          
            if (calls_detected<=1) //restarted
                  {total_callduration=0;
                   intercallduration=0;
                   total_peakpower=0;
                  }

            if (callLength<50) //only calls shorter than 50ms are to be kept
                 { calls_detected++;
                   total_callduration+=callLength;
                   total_peakpower+=pulse_peakPower; //use the peakpowers to estimate signalstrength
                   pulse_peakPower=0;
                 }
            
           
           
         }

      if ((time_since_EndDetection>10) and (time_since_StartDetection<5) and (showStart==false)) 
        {
          if (time_since_tick<1000)
            {intercallduration+=time_since_tick;
             ticks_detected++;
             }
          //for (int i = 0; i <10; i++)
          // {FFT_pixels[i]=color565(255,255,255);
          // }  //white line indicating start of call
            showStart=true;
           
           time_since_tick=0; //reset_tick
           
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
          { //only sum when the signal is not too strong
            if (current_peakPower<40000) // do not use to strong signals as they will have distorted peakvalues
            {
              for (int i = 0; i < spec_hi; i++)
                {   //add new samples
                    FFTpowerspectrum[i]+=FFT_bin[i]; //cumulative collection of samples
                    //keep track of the maximum
                    if ((FFTpowerspectrum[i]>powerspectrum_Max) and (i>signal_LoF_bin))
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
           else
             { 
               #ifdef DEBUG_DETAIL
                D_PRINTXY("peak ", current_peakPower)
               #endif

              };
          }

    }    

  if (playActive) 
    {signal_LoF_bin=old_LoF_bin;
    }

}

#endif