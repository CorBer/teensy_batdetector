/* TEENSYBAT DETECTOR (for TEENSY 3.6/4.1)

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

#ifndef _BAT_DEBUG_H
#define _BAT_DEBUG_H
 // this library allows DEBUG statements inside the code that will "vanish" the moment the DEBUG define
 // is commented out.

#ifdef DEBUG 

#define ANSICOL //allow ansicolors and other codes (bold/italic) to be used for the debugger

#define D_DEFAULT "\033[0m"
#define D_BOLDRED "\033[1;31m"
#define D_BOLDGREEN "\033[1;32m"
#define D_BOLDYELLOW "\033[1;33m"
#define D_BOLDBLUE "\033[1;34m"
#define D_BOLDWHITE "\033[1;37m"

#define SERIALINIT   Serial.begin(19200); delay(1000);

#ifdef ANSICOL 
#define D_PRINT_F(colcode, x)  Serial.print(colcode); Serial.print (x); Serial.print(D_DEFAULT);
#define D_PRINTLN_F(colcode, x)   Serial.print(colcode);  Serial.println (x); Serial.print(D_DEFAULT);
#else
#define D_PRINT_F(colcode, x) Serial.print(x);
#define D_PRINTLN_F(colcode, x)   Serial.println (x); 
#endif 

#define D_PRINT(x) Serial.print(x);
#define D_PRINTLN(x)  Serial.println (x);
#define D_PRINTXY(x,y) Serial.print(x); Serial.print(" "); Serial.println(y);
#define D_PRINTXYZ(x,y,z) Serial.print(x); Serial.print(" "); Serial.print(y); Serial.print(" "); Serial.println(z);
#define D_PRINTHEX(x) Serial.println(x,HEX);

#define D_TIME() Serial.printf("%02d:%02d:%02d\n",hour(),minute(),second());

#else //creat empty defines so no code will be produced by the compiler 
#define SERIALINIT

#define D_PRINT_F(colcode,x) 
#define D_PRINTLN_F(colcode, x) 

#define D_PRINT(x)
#define D_PRINTLN(x)
#define D_PRINTXY(x,y) 
#define D_PRINTXYZ(x,y,z) 
#define D_PRINTHEX(x)

#define D_TIME()  
#endif


#endif