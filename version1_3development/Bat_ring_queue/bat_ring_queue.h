/* Audio Library RingBuffer for Teensy 3.X/4.X
 * Copyright (c) 2021 Cor Berrevoets registax@gmail.com
 *
 * Base on the Audio Library for Teensy (Paul Stoffregen)
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

#ifndef Batring_queue_h_
#define Batring_queue_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioBatRingQueue : public AudioStream
	{ 
	public:
	#if defined(__IMXRT1062__) 
		static const uint32_t ring_buffers = ring_buffer_samples;
	#endif	

	#if defined(__MK66FX1M0__) 
		static const uint32_t ring_buffers = 53;
	#endif


		AudioBatRingQueue(void) : AudioStream(1, inputQueueArray),
			ringuserblock(NULL), head(0), tail(0), enabled(0) { }
		void begin(void) { 
			clear();
			enabled = 1;
			tail = 0;
			head = 0;
			flushed = false;
			full = false;
			counter = 0;
			}
		int available(void);

		void clear(void);
		int16_t* readBuffer(void);
		void freeBuffer(void);
		void start(void) { 
			enabled = 1;
			tail = 0;
			head = 0;
			flushed = false;
			full = false;
			counter = 0;
			}
		void stop(void) { 
			enabled = 0;
			if (counter > ring_buffers)
				{				
full = true;
				}

			}
		virtual void update(void);
		uint32_t counter = 0; //counts samples added to the buffer
		boolean flushed = false; //marks a flushed buffer
		boolean full = false; //marks a full buffer (so after this tail is 1 position before head) 
		audio_block_t* inputQueueArray[1];
		audio_block_t* volatile ring[ring_buffers];
		audio_block_t* ringuserblock;
		audio_block_t* flushblock;
		volatile uint32_t head, tail, enabled;
	private:

	};

#endif
