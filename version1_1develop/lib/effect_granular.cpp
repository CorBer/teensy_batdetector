/*
 * Copyright (c) 2018 John-Michael Reed
 * bleeplabs.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "effect_granular.h"

void AudioEffectGranular::begin(int16_t *sample_bank_def, int16_t max_len_def)
{
	max_sample_len = max_len_def; //teensy V114 max_len_def is 16*1024;
	grain_mode = 0;
	read_head = 0;
	write_head = 0;
	prev_input = 0;
	playpack_rate = 65536;
	accumulator = 0;
	allow_len_change = true;
	sample_loaded = false;
	sample_bank = sample_bank_def;
	//int subsample=0;
}

void AudioEffectGranular::beginFreeze_int(int grain_samples)
{
	__disable_irq();
	grain_mode = 1;
	if (grain_samples < max_sample_len) {
		freeze_len = grain_samples;
	} else {
		freeze_len = grain_samples;
	}
	sample_loaded = false;
	write_en = false;
	sample_req = true;
	__enable_irq();
}

void AudioEffectGranular::beginPitchShift_int(int grain_samples)
{
	__disable_irq();
	grain_mode = 2;
	if (allow_len_change) {
		if (grain_samples < 100) grain_samples = 100;
		int maximum = (max_sample_len - 1) / 3;
		if (grain_samples > maximum) grain_samples = maximum;
		glitch_len = grain_samples;
	}
	sample_loaded = false;
	write_en = false;
	sample_req = true;
	__enable_irq();
}

void AudioEffectGranular::beginTimeExpansion_int(uint16_t grain_samples)
{  //teensy V114 max_sample_len and grain_samples are always 16*1024;
	__disable_irq();
	grain_mode = 3;
	if (allow_len_change) {
		if (grain_samples > max_sample_len) {
		grain_samples = max_sample_len;
	     } 
		glitch_len = grain_samples;
	}
	sample_loaded = false;
	write_en = false;
	sample_req = true;

	__enable_irq();
}

void AudioEffectGranular::beginDivider_int(int grain_samples)
{  //V114 teensy max_sample_len and grain_samples are always 16*1024;
	__disable_irq();
	divider_sample=0; //samplecounter
	read_head=0;
	grain_mode = 4;
	if (allow_len_change) {
		if (grain_samples > max_sample_len) {
		grain_samples = max_sample_len;
	     } 
		glitch_len = grain_samples;
	}
	sample_loaded = false;
	write_en = false;
	sample_req = true;
	__enable_irq();
}


void AudioEffectGranular::stop()
{
	grain_mode = 0;
	allow_len_change = true;
}

void AudioEffectGranular::update(void)
{
	audio_block_t *block;
	
	if (sample_bank == NULL) {
		block = receiveReadOnly(0);
		if (block) release(block);
		return;
	}

	block = receiveWritable(0);
	
	if (!block) return;

	if (grain_mode == 0) {
		// passthrough, no granular effect
		prev_input = block->data[AUDIO_BLOCK_SAMPLES-1];
	}
	else if (grain_mode == 1) {
		// Freeze - sample 1 grain, then repeatedly play it back
		for (int j = 0; j < AUDIO_BLOCK_SAMPLES; j++) {
			if (sample_req) {
				// only begin capture on zero cross
				int16_t current_input = block->data[j];
				if ((current_input < 0 && prev_input >= 0) ||
				  (current_input >= 0 && prev_input < 0)) {
					write_en = true;
					write_head = 0;
					read_head = 0;
					sample_req = false;
				} else {
					prev_input = current_input;
				}
			}
			if (write_en) {
				sample_bank[write_head++] = block->data[j];
				if (write_head >= freeze_len) {
					sample_loaded = true;
				}
				if (write_head >= max_sample_len) {
					write_en = false;
				}
			}
			if (sample_loaded) {
				if (playpack_rate >= 0) {
					accumulator += playpack_rate;
					read_head = accumulator >> 16;
				}
				if (read_head >= freeze_len) {
					accumulator = 0;
					read_head = 0;
				}
				block->data[j] = sample_bank[read_head];
			}
		}
	}
	else if (grain_mode == 2) {
		//GLITCH SHIFT
		// basic granular synth thingy
		// the shorter the sample the max_sample_len the more tonal it is.
		// Longer it has more definition.  It's a bit roboty either way which
		// is obv great and good enough for noise music.

		for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++) {
			// only start recording when the audio is crossing zero to minimize pops
			if (sample_req) {
				int16_t current_input = block->data[k];
				if ((current_input < 0 && prev_input >= 0) ||
				  (current_input >= 0 && prev_input < 0)) {
					write_en = true;
				} else {
					prev_input = current_input;
				}
			}

			if (write_en) {
				sample_req = false;
				allow_len_change = true; 
				if (write_head >= glitch_len) {  //wrap back to the beginning
					write_head = 0;
					sample_loaded = true;
					write_en = false;
					allow_len_change = false;
				}
				sample_bank[write_head] = block->data[k];  //store current data
				//decimate to half the samples
 			    write_head++;
			}

			if (sample_loaded) {
				//move it to the middle third of the bank.
				//3 "seperate" banks are used
				float fade_len = 20.00;
				int16_t m2 = fade_len;

				for (int m = 0; m < 2; m++) {
					// I'm off by one somewhere? why is there a tick at the
					// beginning of this only when it's combined with the
					// fade out???? ooor am i osbserving that incorrectly
					// either wait it works enough
					sample_bank[m + glitch_len] = 0;
				}

				for (int m = 2; m < glitch_len-m2; m++) {
					sample_bank[m + glitch_len] = sample_bank[m];
				}

				for (int m = glitch_len-m2; m < glitch_len; m++) {
					// fade out the end. You can just make fadet=0
					// but it's a little too daleky
					float fadet = sample_bank[m] * (m2 / fade_len);
					sample_bank[m + glitch_len] = (int16_t)fadet;
					m2--;
				}
				sample_loaded = false;
				prev_input = block->data[k];
				sample_req = true;
			}

			accumulator += playpack_rate;
			read_head = (accumulator >> 16);

			if (read_head >= glitch_len) {
				read_head -= glitch_len;
				accumulator = 0;

				for (int m = 0; m < glitch_len; m++) {
					sample_bank[m + (glitch_len*2)] = sample_bank[m+glitch_len];
					//  sample_bank[m + (glitch_len*2)] = (m%20)*1000;
				}
			}
			block->data[k] = sample_bank[read_head + (glitch_len*2)];
		}
	} 

	else if (grain_mode == 3) {
		//TIME EXPANSION ADDED for TEENSY BATDETECTOR 
		// process incoming block of samples
	
		for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++) 
		 { 
			// if a sample is requested position both read and writeheads at start
			if (sample_req) {
      		    write_en = true; //start collecting a sample by setting write_enabled
				write_head=0; //start at position 0
				read_head=0;
				accumulator=0; //new added 26012021 tracks 
  			}

            //active collecting of all incoming data if write_enabled
			if (write_en) {
				sample_req = false; // stop sample request as we are allready recording

				// collect until maximum glitch_len samples are available in the sample_bank
				if (write_head >= glitch_len) {
					write_head = glitch_len-1;
					//accumulator= 0;
					sample_loaded = true; //sample_bank is full 
					write_en = false; // stop sampling
				}

				play_sample=true; //replay is allowed since some data has been stored
				sample_bank[write_head]= block->data[k];
				write_head++; // next position to write a sample
			   
			}

            //sample bank is full and can be used to transfer back to the audio system
			if (sample_loaded) {
				
			}
            
			 
			if (play_sample) // move to next playing sample based on the TE_speed setting
			 { //accumulator += playpack_rate; // shift sampleposition  #old code 26012021
			   //read_head = (accumulator >> 16); //rightshift 16 == div by 65535 #old code 26012021
			   accumulator++;  //new code 26012021
			   if (accumulator==TE_speed)
			    {  read_head++; //new code 26012021
				   accumulator=0;
				}
			 }

            if (play_sample)
			    {block->data[k] = sample_bank[read_head]; //store old sample in the current output
				}
				else
				{block->data[k] = 0; //store empty samples when play should stop
				}
				
			 //read head has passed the write head, stop playing
			if (read_head >= write_head) {
				read_head = 0;
				accumulator = 0;
                sample_req = false; //we dont want a new sample
				sample_loaded = false;
				play_sample=false;
			}


            //block->data[k] = sample_bank[read_head]; //store old sample in the current output
			
			// if (old_read_head!=read_head) //we changed sample so start the FIIR 
			//   {last_sample=sample_bank[old_read_head];
			//    old_read_head=read_head;
			//   }
			   
			
		}
	} 
	else if (grain_mode == 4) {
		//FREQUENCY DIVIDER
		 
        uint8_t dividerset=Fdivider;
		
		//needs very little memory, only X=dividerset samples[X] 
		//128 samples come in and will be converted into 128/16=8 different soundsamples that are equal for 16 positions in a row
		//OLD SETUP
		// for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++) 
		//  {   
		// 	 sample_bank[divider_sample%dividerset]= block->data[k];  //store current readout in revolving arrayposition
		// 	 //eg at k=10 and divider_sample starting at 0
		// 	 //samplebank [10/16]=block->data[10]
		// 	 //samplebank [11/16]=block->data[11]
		// 	 //...
		// 	 //when k reaches 16 all data is lost !!
		// 	 //samplebank [16/16]=block->data[16]

	    //      block->data[k] = sample_bank[0]; //output is always based on the same arrayposition
		// 	 divider_sample++;
		// }
		//NEW
		//VERY CRUDE DATA REDUCTION 
		for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++) 
		 {   
           if (k%dividerset==0) // every Xth sample is used for the output
			  {   nxt_sample=block->data[k];
			   } //
		    block->data[k] = nxt_sample; 
		} 

        //OLD SETUP
		//roll over the sample_counter at a safe point (block_size*divider) so all blocks are based on equal datasets
		// if (divider_sample>=AUDIO_BLOCK_SAMPLES*dividerset)
		//  {  divider_sample=0;
		//  }
		  
	  }
	  
	transmit(block);
	release(block);
}




