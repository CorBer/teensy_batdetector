/* Audio Library RingBuffer for Teensy 3.X/4.X
 * Copyright (c) 2021 Cor Berrevoets registax@gmail.com
 *
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
#include "bat_ring_queue.h"
#include "utility/dspinst.h"

int AudioBatRingQueue::available(void)
	{ 
	uint32_t h, t;
	h = head;
	t = tail;
	if (h >= t) return h - t;
	return ring_buffers + h - t;

	}


void AudioBatRingQueue::clear(void)
	{ 
	if (ringuserblock)
		{ 
		release(ringuserblock);
		ringuserblock = NULL;
		}


	}

int16_t* AudioBatRingQueue::readBuffer(void)
	{ 
	uint32_t t;
	if (ringuserblock) return NULL;
	t = tail;
	if (t == head) return NULL; //nothing left to read 
	if (++t >= ring_buffers) t = 0;
	ringuserblock = ring[t];
	tail = t;

	return ringuserblock->data;
	}

void AudioBatRingQueue::freeBuffer(void)
	{ 
	if (ringuserblock == NULL) return;
	release(ringuserblock);
	ringuserblock = NULL;
	}

void AudioBatRingQueue::update(void)
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
	counter++;
	if (h >= ring_buffers) h = 0;
	// if (h == tail) { 
	// 	release(block); //dont store
	// } else { 
	// 	ring[h] = block; //new position so store
	// 	head = h;
	// }
	if (ring[h] != NULL)
		{ //slot is occupied so release
		release(ring[h]);
		}
	ring[h] = block;
	head = h;
	tail = h + 1;
	if (tail >= ring_buffers)
		{		
tail = 0;
		}

	}


