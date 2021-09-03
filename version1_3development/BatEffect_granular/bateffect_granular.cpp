/* //ORIGINAL effect_granular by
 *
 * Copyright (c) 2018 John-Michael Reed
 * bleeplabs.com
 *
 * Adapted for TeensyBat, copyright (c) 2021 Cor Berrevoets registax@gmail.com
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
 * ****************************************************************************
 *
 */

#include <Arduino.h>
#include "bateffect_granular.h"

void AudioBatEffectGranular::begin(int16_t* sample_bank_def, int16_t max_len_def)
	{
	max_sample_len = max_len_def; //teensy V114 max_len_def is 16*1024;
	grain_mode = 3; //default TE
	read_head = 0;
	write_head = 0;
	prev_input = 0;
	//playpack_rate = 65536;
	accumulator = 0;
	allow_len_change = true;
	sample_loaded = false;
	sample_bank = sample_bank_def; //pointer towards the samplebank
	//int subsample=0;
	}

void AudioBatEffectGranular::beginTimeExpansion_int(uint16_t grain_samples)
	{  //teensy V114 max_sample_len and grain_samples are always 16*1024;
	__disable_irq();
	grain_mode = 3;
	if (allow_len_change)
		{
		if (grain_samples > max_sample_len)
			{
			grain_samples = max_sample_len;
			}
		glitch_len = grain_samples;
		}
	sample_loaded = false;
	write_en = false;
	sample_req = true;

	__enable_irq();
	}

void AudioBatEffectGranular::beginDivider_int(int grain_samples)
	{  //V114 teensy max_sample_len and grain_samples are always 16*1024;
	__disable_irq();
	divider_sample = 0; //samplecounter
	read_head = 0;
	grain_mode = 4;
	if (allow_len_change)
		{
		if (grain_samples > max_sample_len)
			{
			grain_samples = max_sample_len;
			}
		glitch_len = grain_samples;
		}
	sample_loaded = false;
	write_en = false;
	sample_req = true;
	__enable_irq();
	}


void AudioBatEffectGranular::stop()
	{
	grain_mode = 0;
	allow_len_change = true;
	}

void AudioBatEffectGranular::update(void)
	{
	audio_block_t* block;

	if (sample_bank == NULL)
		{
		block = receiveReadOnly(0);
		if (block) release(block);
		return;
		}

	block = receiveWritable(0);

	if (!block) return;

	if (grain_mode == 0)
		{
		// passthrough, no granular effect
		prev_input = block->data[AUDIO_BLOCK_SAMPLES - 1];
		}
	else if (grain_mode == 3)
		{
		//TIME EXPANSION ADDED for TEENSY BATDETECTOR 
		// process incoming block of samples
		for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++)
			{
			// if a sample is requested position both read and writeheads at start
			if (sample_req)
				{
				write_en = true; //start collecting a sample by setting write_enabled
				write_head = 0; //start at position 0
				read_head = 0;
				accumulator = 0; //new added 20210126 
				}

			//active collecting of all incoming data if write_enabled
			if (write_en)
				{
				sample_req = false; // stop sample request as we are allready recording

				// collect until maximum glitch_len samples are available in the sample_bank
				if (write_head >= glitch_len)
					{
					write_head = glitch_len - 1;
					//accumulator= 0;
					sample_loaded = true; //sample_bank is full 
					write_en = false; // stop sampling
					}

				play_sample = true; //replay is allowed since some data has been stored
				sample_bank[write_head] = block->data[k];
				write_head++; // next position to write a sample

				}

			//sample bank is full and can be used to transfer back to the audio system
			if (sample_loaded)
				{

				}


			if (play_sample) // move to next playing sample based on the TE_speed setting
				{ //accumulator += playpack_rate; // shift sampleposition  #old code 26012021
				  //read_head = (accumulator >> 16); //rightshift 16 == div by 65535 #old code 26012021
				accumulator++;  //new code 20210126 counts passing blocks
				if (accumulator == TE_speed) //enough blocks have passed 
					{
					read_head++; //new code 26012021
					accumulator = 0;
					}
				}

			if (play_sample)
				{
				block->data[k] = sample_bank[read_head]; //store old sample in the current output
				}
			else
				{
				block->data[k] = 0; //store empty samples when play should stop
				}

			//read head has passed the write head, stop playing
			if (read_head >= write_head)
				{
				read_head = 0;
				accumulator = 0;
				sample_req = false; //we dont want a new sample
				sample_loaded = false;
				play_sample = false;
				}


			}
		}
	else if (grain_mode == 4)
		{
		//FREQUENCY DIVIDER
		for (int k = 0; k < AUDIO_BLOCK_SAMPLES; k++)
			{
			if (samplecounter >= Fdivider) // every Xth sample is used dividerset times for the output 
				{
				nxt_sample = block->data[k];
				samplecounter = 0;
				}
			block->data[k] = nxt_sample;
			samplecounter++;
			}
		}

	transmit(block);
	release(block);
	}




