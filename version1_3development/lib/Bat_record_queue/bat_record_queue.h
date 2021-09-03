/* Audio Library Recorder for Teensy 3.X/4.x
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Adapted for TeensyBat, copyright (c) 2021 Cor Berrevoets registax@gmail.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
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

#ifndef Batrecord_queue_h_
#define Batrecord_queue_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioBatRecordQueue : public AudioStream
	{ 
	private:
	#if defined(__IMXRT1062__) 
	#ifdef PSRAM
		static const uint32_t rec_buffers = rec_buffer_samples;
	#else
		static const int rec_buffers = rec_buffer_samples;
	#endif

	#endif	

	#if defined(__MK66FX1M0__) 
		static const int rec_buffers = rec_buffer_samples;
	#endif


	public:
		AudioBatRecordQueue(void) : AudioStream(1, inputQueueArray),
			userblock(NULL), head(0), tail(0), enabled(0) {}

		void begin(void) { 
			clear();
			enabled = 1;
			}
		int available(void);
		void clear(void);
		int16_t* readBuffer(void);
		void freeBuffer(void);
		void end(void) { 
			enabled = 0;
			}
		virtual void update(void);
	private:
		audio_block_t* inputQueueArray[1];
		audio_block_t* volatile queue[rec_buffers];
		audio_block_t* userblock;
		volatile uint32_t head, tail, enabled;

	};

#endif
