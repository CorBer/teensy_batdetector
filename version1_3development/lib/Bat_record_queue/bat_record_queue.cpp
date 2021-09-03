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

#include <Arduino.h>
#include "bat_record_queue.h"
#include "utility/dspinst.h"

int AudioBatRecordQueue::available(void)
	{ 
	uint32_t h, t;
	h = head;
	t = tail;
	if (h >= t) return h - t;
	return rec_buffers + h - t;

	}

void AudioBatRecordQueue::clear(void)
	{ 
	uint32_t t;

	if (userblock)
		{ 
		release(userblock);
		userblock = NULL;
		}

	t = tail;
	while (t != head)
		{ 
		if (++t >= rec_buffers) t = 0;
		release(queue[t]);
		}
	tail = t;

	}

int16_t* AudioBatRecordQueue::readBuffer(void)
	{ 
	uint32_t t;
	if (userblock) return NULL;
	t = tail;
	if (t == head) return NULL;
	if (++t >= rec_buffers) t = 0;
	userblock = queue[t];
	tail = t;

	return userblock->data;
	}

void AudioBatRecordQueue::freeBuffer(void)
	{ 
	if (userblock == NULL) return;
	release(userblock);
	userblock = NULL;
	}

void AudioBatRecordQueue::update(void)
	{ 
	audio_block_t* block;
	uint32_t h;

	block = receiveReadOnly();
	if (!block) return;
	if (!enabled)
		{ 
		release(block);
		return;
		}

	h = head + 1;
	if (h >= rec_buffers) h = 0;
	if (h == tail)
		{ 
		release(block); //buffer is full !
		}
	else
		{ 
	#ifdef MICROMARK
		block->data[0] = micros() / 10; //dummymark
	#endif
		queue[h] = block;
		head = h;
		}

	}


