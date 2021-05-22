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

#ifndef _BAT_AUDIO_H
#define _BAT_AUDIO_H

#if defined(__MK66FX1M0__) 
   #include "Audio.h"
   #include "kinetis.h"
#endif

#if defined(__IMXRT1062__)
   #include "Audio.h"
#endif

#include "core_pins.h"
#include "bat_vars.h"

#define heterodynemixer 0
#define granularmixer 1
#define passivemixer 2
#define granularHTmixer 3

#define in_mic 0
#define in_player 1 
#define in_adc 2 

// *********************************** AUDIO SGTL5000 SETUP *******************************
// this audio comes from the codec by I2S2
AudioInputI2S                    i2s_in; // MIC input
AudioInputAnalog                 adc1(A2); //xy=197,73
AudioRecordQueue                 recorder;
AudioSynthWaveformSine           sine1; // local oscillator
//AudioFilterBiquad                BiQuad1;
AudioEffectMultiply              heterodyne_multiplier; // multiply = mix

AudioAnalyzeFFT256               myFFT; // for spectrum display

AudioBatPlaySdRaw                   player;

AudioBatEffectGranular              granular1;

AudioMixer4                      outputMixer; //selective output
AudioMixer4                      inputMixer; //selective input
AudioOutputI2S                   i2s_out; // headphone output

AudioConnection mic_toinput         (i2s_in, 0, inputMixer, 0); //microphone signal

AudioConnection player_toinput      (player, 0, inputMixer, 1); //player signal

AudioConnection ADC_toinput         (adc1, 0, inputMixer, 2); //adc signal
//AudioConnection sine1_toinput       (sine1,0, inputMixer, 2);   // to test!

//AudioConnection mic_torecorder      (inputMixer, 0, recorder, 0); //direct input signal after inputMixer gain !! No USE_HIPASS_FILTER !

//AUDIO FILTER ACTIVE signals first pass through a highpass biquad filter
AudioMixer4                          hiPassMixer;
AudioFilterBiquad                    biquad1;
AudioConnection input_tobiquad       (inputMixer,0, biquad1, 0); //output to biquad1
AudioConnection input_tohiPass       (inputMixer,0, hiPassMixer, 0); //input direct to hipass (bypass)
AudioConnection biquad_tohiPass      (biquad1,0, hiPassMixer, 1); //biquad filter to hipassmixer

AudioConnection HiPass_togranular  (hiPassMixer,0, granular1, 0);

AudioConnection granular_toout      (granular1,0, outputMixer,1); //granular directly to outputmixer 1

AudioConnection HiPass_toFFT         (hiPassMixer,0,  myFFT,0);
AudioConnection HiPass_toheterodyne1 (hiPassMixer,0, heterodyne_multiplier, 0); //heterodyne 1 signal
AudioConnection mic_torecorder       (hiPassMixer, 0, recorder, 0); 


#ifdef USE_TEFACTOR //when DETUNING the TE output add a heterodynemix setup
  
  AudioSynthWaveformSine          sine_detune;
  AudioEffectMultiply             HT_multiplier_detune;
  
  AudioConnection HiPass_tomult  (granular1,0, HT_multiplier_detune, 0);
  AudioConnection detune_tomult  (sine_detune,0, HT_multiplier_detune, 1);
  AudioConnection detunemult_toout  (HT_multiplier_detune, 0, outputMixer,3);

#endif

AudioConnection sineheterodyne1     (sine1, 0, heterodyne_multiplier, 1);//heterodyne 1 mixerfreq

AudioConnection heterodyne1_toout   (heterodyne_multiplier, 0, outputMixer, 0);  //heterodyne 1 output to outputmixer 0
AudioConnection player_toout        (inputMixer,0, outputMixer, 2);    //direct signal (use with player) to outputmixer 2

AudioConnection output_toheadphoneleft      (outputMixer, 0, i2s_out, 0); // output to headphone 
AudioConnection output_toheadphoneright     (outputMixer, 0, i2s_out, 1);

AudioControlSGTL5000        sgtl5000;

// #define heterodynemixer 0
// #define granularHTmixer 1
// #define passivemixer 2
// #define granularmixer 3

// #define in_mic 0
// #define in_player 1 
// #define in_adc 2 

#ifdef DEBUG
const char* inputs[3] =
  { "Mic",
    "SD_player",
    "ADC"
  };
const char* outputs[4] =
  { "heterodyne",
    "granular",
    "passive",
    "granularHT"
  };

#endif

int myInput = AUDIO_INPUT_MIC;

// *********************************SAMPLING ****************************

#define SR_MIN               0
#define SR_8K                0
#define SR_11K               1
#define SR_16K               2
#define SR_22K               3
#define SR_32K               4
#define SR_44K               5
#define SR_48K               6
#define SR_88K               7
#define SR_96K               8
#define SR_176K              9
#define SR_192K              10
#define SR_234K              11
#define SR_281K              12
#define SR_352K              13
#define SR_384K              14
#define SR_MAX               14

// ***************** SAMPLE RATE DESCRIPTION
typedef struct SR_Descriptor
{   const uint8_t SR_n;
    const char* txt; //text for the display
    const uint32_t osc_frequency;
    const u_int16_t MUL_Fs;
    const u_int16_t DIV_Fs;
    
} SR_Desc;

// SRtext and position for the FFT spectrum display scale
const SR_Descriptor SR [SR_MAX + 1] =
{   //SR_CODE  SR_TEXT, SR_FREQ, MUL_Fs, DIV_Fs}
    {  SR_8K,  "8" ,     8000,   46, 4043},
    {  SR_11K,  "11",   11025,   49, 3125},
    {  SR_16K,  "16",   16000,   73, 3208},
    {  SR_22K,  "22",   22050,   98, 3125},
    {  SR_32K,  "32",   32000,  183, 4021},
    {  SR_44K,  "44",   44100,  196, 3125}, //F: 43940.0 1 16
    {  SR_48K,  "48",   48000,  128, 1875}, //F: 47940.0 3 44
    {  SR_88K,  "88",   88200,  107,  853}, //F: 87910.0 1  8
    {  SR_96K,  "96",   96000,  219, 1604}, //F: 95880.0 3 22
    {  SR_176K,  "176", 176400,   1,    4},
    {  SR_192K,  "192", 192000, 219,  802}, //F: 191790.0 3 11
    {  SR_234K,  "234", 234000,   1,    3},
    {  SR_281K,  "281", 281000,   2,    5},
    {  SR_352K,  "352", 352800,   1,    2},
    {  SR_384K,  "384", 383500,   6,   11}
};

// default samplerates 
//change in V1.2
#define MAX_play_SR 21 //play will be allowed from 1/20 to 1/5 and 1:1 (direct)
int play_SR = 10; //default play SAMPLERATE 1/10

int oper_SR = SR_281K; //operational SAMPLERATE
int rec_SR = SR_281K; //recorder SAMPLERATE

uint32_t recorded_SR=0; //samplerate from a loaded recording
uint8_t knownSR=0;

float SR_real = SR[oper_SR].osc_frequency;
//int last_SR=oper_SR;
char SRtext[4]="...";

double allpass[]={1.0,0.0,0.0,0.0,0.0}; //biquad allpass

void set_InputMixer(uint8_t channel)
{    D_PRINTXY("inMixer from (gain=1)", inputs[channel]) 
     inputMixer.gain(0,0); //microphone off
     inputMixer.gain(1,0); //player off
     inputMixer.gain(2,0); //ADC off
     inputMixer.gain(channel,1); //activate channel
}

void set_OutputMixer(int channel)
 { D_PRINTXY("outMixer from ",outputs[channel]) 
  
   outputMixer.gain(heterodynemixer,0); //0
   outputMixer.gain(granularHTmixer,0); //1
   outputMixer.gain(passivemixer,0); //2
   outputMixer.gain(granularmixer,0); //3
   outputMixer.gain(channel,1);
   
}
// ***************************************************** OSCILLATOR
void  set_freq_Oscillator(int freq) {
    // audio lib thinks we are still in 44118sps sample rate therefore we have to scale the frequency of the local oscillator
    // in accordance with the REAL sample rate
    D_PRINTXY("set OSC", freq);
    freq_Oscillator = (freq) * (AUDIO_SAMPLE_RATE_EXACT / SR_real);
    //float F_LO2= (freq+5000) * (AUDIO_SR_EXACT / SR_real);
    // if we switch to LOWER samples rates, make sure the running LO frequency is allowed ( < 22k) ! 
    // If not, adjust consequently, so that LO freq never goes up 22k, also adjust the variable osc_frequency
    if(freq_Oscillator > 22000) {
      freq_Oscillator = 22000;
      osc_frequency = freq_Oscillator * (SR_real / AUDIO_SAMPLE_RATE_EXACT) + 9;
    }
    D_PRINTXY("set FreqOsc Sine", freq_Oscillator);
    AudioNoInterrupts();
    sine1.frequency(freq_Oscillator);
    sine1.amplitude(sine_amplitude); //sine ON
    AudioInterrupts();
    
} // END of function set_freq_Oscillator

void adjust_Heterodyne_Oscillator()
{   int frequency=int((FFT_peakF_bin*(SR_FFTratio)/500))*500+1000; //round to nearest 500hz and shift 1000hz up to make the signal audible
    if (abs(last_osc_frequency-frequency)>3000) //only adjust when necessary
      {
        osc_frequency=frequency;
        osc_frequency=constrain(osc_frequency,7000,int(SR_real/2000)*1000-1000);
        set_freq_Oscillator(osc_frequency*AHT_factor/100.0f);
        last_osc_frequency=osc_frequency;
      }
}

void setHiPass()
{
  if (HI_pass>HI_PASS_OFF)
            {
            D_PRINTXY("HI PASS",HI_pass);  
            biquad1.setHighpass(0,HI_pass*1000*(AUDIO_SAMPLE_RATE_EXACT /SR_real),0.7);
            //biquad1.setLowShelf(0,HI_pass*1000*(AUDIO_SAMPLE_RATE_EXACT /SR_real),-10,1);
            hiPassMixer.gain(0,0); //direct input off
            hiPassMixer.gain(1,1);  //biquad input in
            
            }
          else
          { D_PRINTLN("HI PASS OFF");
            biquad1.setCoefficients(0, allpass);
            
            hiPassMixer.gain(1,0); //biquad input off
            hiPassMixer.gain(0,1); //direct input on
            }
}


/***************************************************************/
//code by FRANK BOESING https://github.com/FrankBoesing/AudioTiming/

#if defined(__MK66FX1M0__) 
#define F_I2S ((((I2S0_MCR >> 24) & 0x03) == 3) ? F_PLL : F_CPU)
uint32_t I2S_dividers( float fsamp, uint32_t nbits, uint32_t tcr2_div )
{

  unsigned fract, divi;
  fract = divi = 1;
  float minfehler = 1e7;

  unsigned x = (nbits * ((tcr2_div + 1) * 2));
  unsigned b = F_I2S / x;

  for (unsigned i = 1; i < 256; i++) {

    unsigned d = round(b / fsamp * i);
    float freq = b * i / (float)d ;
    float fehler = fabs(fsamp - freq);

    if ( fehler < minfehler && d < 4096 ) {
      fract = i;
      divi = d;
      minfehler = fehler;
      //Serial.printf("%fHz<->%fHz(%d/%d) Fehler:%f\n", fsamp, freq, fract, divi, minfehler);
      if (fehler == 0.0f) break;
    }

  }

  return I2S_MDR_FRACT( (fract - 1) ) | I2S_MDR_DIVIDE( (divi - 1) );
}

/***************************************************************/
 
bool setI2S_freq(float fsamp)
{
  #ifdef DEBUG_DETAIL
  D_PRINTLN("setI2S");  
  #endif
  if ((SIM_SCGC6 & SIM_SCGC6_I2S) != SIM_SCGC6_I2S)
    return false; //I2S not enabled
  unsigned tcr5 = I2S0_TCR5;
  unsigned word0width = ((tcr5 >> 24) & 0x1f) + 1;
  unsigned wordnwidth = ((tcr5 >> 16) & 0x1f) + 1;
  unsigned framesize = ((I2S0_TCR4 >> 16) & 0x0f) + 1;
  unsigned nbits = word0width + wordnwidth * (framesize - 1 );
  unsigned tcr2div = I2S0_TCR2 & 0xff; //bitclockdiv
  uint32_t MDR = I2S_dividers(fsamp, nbits, tcr2div);
  if (MDR > 0) {
    while (I2S0_MCR & I2S_MCR_DUF) {
      ;
    }
    I2S0_MDR = MDR;
    return true;
  }
  #ifdef DEBUG_DETAIL
  D_PRINTLN("I2S NOT set");  
  #endif
  
  return false;

}
#endif


#if defined(__IMXRT1062__) //PRELIMINARY Teensy 4.1 UNTESTED CODE 

#include <utility/imxrt_hw.h>
bool setI2S_freq(int freq) {
  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  int n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
  int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  double C = ((double)freq * 256 * n1 * n2) / 24000000;
  int c0 = C;
  int c2 = 10000;
  int c1 = C * c2 - (c0 * c2);
  set_audioClock(c0, c1, c2, true);
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
       | CCM_CS1CDR_SAI1_CLK_PRED(n1-1) // &0x07
       | CCM_CS1CDR_SAI1_CLK_PODF(n2-1); // &0x3f
  return true;     
}
#endif

/***************************************************************/

//set the samplerate for replay (up to 312K can be used )
void set_SR_play(float playfreq)
{ D_PRINTXY("SET SR_PLAY PLAYFREQ",playfreq)

  if (playfreq>312000) //too high
    { D_PRINTLN_F(D_BOLDRED,"LIMIT PLAYFREQ");
      playfreq=playfreq/MAX_play_SR;
    }

  SR_real=playfreq;
  SR_FFTratio=(SR_real/(FFT_points*2));

  signal_LoF_bin= int((TE_low*1000.0)/(SR_FFTratio));
  if (AUTO_REC) 
      { signal_LoF_bin= int((AREC_F*1000.0)/(SR_FFTratio));
       }
  // highest frequencybin to detect as a batcall
  signal_HiF_bin= int((SR_real/2)/SR_FFTratio);
         
  //set FS directly from the preset SR MUL/DIVs 
  #ifdef DEBUG_DETAIL
    D_PRINTXY("playfreq",playfreq);
    D_PRINTXY("SampleRate",SR_real);
  #endif
  
  AudioNoInterrupts();  
  if ((setI2S_freq(playfreq)==true) )
   {  D_PRINTXY("I2Sset PLAYER SampleRate:",SR_real);
    }
  else
   { 
     D_PRINTLN("I2Sset PLAYER samplerate failed");
     //switch back to a known samplerate
     if (recorded_SR>0)
        {setI2S_freq(recorded_SR/10);
         D_PRINTXY("set PLAYSR",int(recorded_SR/10));
        }
     else //unknown recorded SR
        {setI2S_freq(19200); //set for 1/10 for 192K
         D_PRINTLN("force PLAYSR 19.2K"); 
        }
   }
    
  
  delay(200); // this delay seems to be very essential !
  
  set_freq_Oscillator (osc_frequency); // set oscillator for HT 
  
  myFFT.averageTogether(1); //keep this at all times at 1 (no oversampling)
  
  AudioInterrupts();
  delay(20); 

  
  setHiPass();
  
  
  if (!(playActive or recorderActive))
     {D_PRINTXY("SRplay DETECTOR SampleRate:",SR_real);
     }
  if (playActive)
  {
     D_PRINTXY("SRplay PLAYER SampleRate:",SR_real);
  }
  if (recorderActive)
   {
     D_PRINTXY("SRplay RECORDER SampleRate:",SR_real);
   }
  

}


// ***************************************************** SAMPLE RATE
//set SAMPLERATE
void  set_SR (int sr) {
  D_PRINTXY("setSR SR",sr)
  SR_real=SR[sr].osc_frequency;
  SR_FFTratio=(SR_real/(FFT_points*2));
      
  signal_LoF_bin= int((TE_low*1000.0)/(SR_FFTratio));
  if (AUTO_REC) 
      { signal_LoF_bin= int((AREC_F*1000.0)/(SR_FFTratio));
       }
  // highest frequencybin to detect as a batcall
  signal_HiF_bin= int((SR_real/2)/SR_FFTratio);
  snprintf(SRtext,4,SR[sr].txt);
 // SRtext=SR[sr].txt;
  AudioNoInterrupts();

  #if defined(__MK66FX1M0__) 
  //set FS directly from the preset SR MUL/DIVs 
    while (I2S0_MCR & I2S_MCR_DUF) ;
        I2S0_MDR = I2S_MDR_FRACT((SR[sr].MUL_Fs-1)) | I2S_MDR_DIVIDE((SR[sr].DIV_Fs-1));

  #endif
  #if defined(__IMXRT1062__) 
   setI2S_freq(SR_real);
  #endif  

  delay(200); // this delay seems to be very essential !
  set_freq_Oscillator (osc_frequency);
  myFFT.averageTogether(1);

  AudioInterrupts();
  delay(20); 

  
    setHiPass();
  
  
  if (!(playActive or recorderActive))
     {D_PRINTXY("setSR DETECTOR SampleRate:",SR_real);
     }
  if (playActive)
  {
     D_PRINTXY("setSR PLAYER SampleRate:",SR_real);
  }
  if (recorderActive)
   {
     D_PRINTXY("setSR RECORDER SampleRate:",SR_real);
   }
     
  
}

//SET VOLUME
void set_vol(int8_t volume)
   {      float V=volume*0.01;
          AudioNoInterrupts();
          sgtl5000.volume(V);
          AudioInterrupts();
   }

//SET MICROPHONE (OR LINEIN) GAIN
void  set_mic(int8_t gain) 
{
  D_PRINT("SET GAIN:");
  if (myInput==AUDIO_INPUT_MIC) // MIC input on SGTL5000 or ADC_IN 
    {
          D_PRINTLN("MIC");
          AudioNoInterrupts();
          sgtl5000.lineInLevel(0); //cutoff line_input
          
          //sgtl5000.micGain (gain);
          //shutdown linein
          D_PRINTXY("  gain:",gain)
         
          sgtl5000.micGain(gain);
          AudioInterrupts();
    } 
    if (myInput==AUDIO_INPUT_LINEIN)  
     { D_PRINTLN("LINE");
       AudioNoInterrupts();
       
       sgtl5000.micGain(0); //close mic
       
       //open linein
       sgtl5000.lineInLevel(gain/4);
       
       AudioInterrupts();
       D_PRINTXY("  gain:",gain/4)
     }
    #ifdef USE_ADC_IN
        if (myInput==AUDIO_INPUT_ADC)
          { float f_gain=gain;
            D_PRINTLN("ADC");
            D_PRINTXY("adc g:",gain);
            if (gain<=30)
              {f_gain=f_gain/30;
                }
              else
              {f_gain=1+(f_gain-30)/3;
                
              }
          
            inputMixer.gain(2,f_gain);
            D_PRINTXY("inMixer(ch:2) gain:",f_gain)  
                
          }
    #endif

    
   
} // end function set_mic_gain


#endif