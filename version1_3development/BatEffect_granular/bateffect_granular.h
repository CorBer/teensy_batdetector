/*
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
 */

#ifndef bateffect_granular_h_
#define bateffect_granular_h_

#include "AudioStream.h"

class AudioBatEffectGranular : public AudioStream
	{
	public:
		AudioBatEffectGranular(void) : AudioStream(1, inputQueueArray) {}
		void begin(int16_t* sample_bank_def, int16_t max_len_def);

		void setTESpeed(uint8_t speed)
			{
			TE_speed = speed;
			}
		// void setSpeed(float ratio)
		// 	{
		// 	if (ratio < 0.01)
		// 		ratio = 0.01; //100 times slower
		// 	else if (ratio > 8.0)
		// 		ratio = 8.0; //8 times faster ?
		// 	//playpack_rate = ratio * 65536.0 + 0.499;
		// 	}
		void setdivider(uint8_t ratio)
			{
			Fdivider = ratio;

			// if (ratio < 1)
			// 	ratio = 1;
			// else if (ratio > 50)
			// 	ratio = 50;

			// Rdivider = 1 / ratio;

			dividerStart = 0; //keeps track of sampleposition
			samplecounter = 0;

			}

		void beginTimeExpansion(uint16_t grain_length)
			{
			if (grain_length <= 0.0)
				return;
			beginTimeExpansion_int(grain_length);
			}

		void stopTimeExpansion()
			{
			sample_loaded = true;
			//sample_req=false;
			}

		void beginDivider(float grain_length)
			{
			if (grain_length <= 0.0)
				return;
			beginDivider_int(grain_length);
			}

		void stop();
		virtual void update(void);

	private:
		void beginTimeExpansion_int(uint16_t grain_samples);
		void beginDivider_int(int grain_samples);
		audio_block_t* inputQueueArray[1];
		int16_t* sample_bank;
		//	uint32_t playpack_rate;
			//float Rdivider;
		uint8_t Fdivider;
		uint8_t dividerStart = 0;
		uint8_t samplecounter = 0;
		uint8_t TE_speed;
		uint32_t accumulator;
		int16_t max_sample_len;
		int16_t write_head;
		int16_t read_head;
		int16_t grain_mode;
		//int16_t freeze_len;
		int16_t prev_input;
		int16_t glitch_len;
		int32_t divider_sample;
		uint16_t nxt_sample = 0;

		uint32_t samplesum = 0;

		long sum_sample = 0;

		bool play_sample;
		bool allow_len_change;
		bool sample_loaded;
		bool write_en;
		bool sample_req;
	};

#endif