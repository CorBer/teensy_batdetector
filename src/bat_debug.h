#ifndef _BAT_DEBUG_H
#define _BAT_DEBUG_H

#if COMPILER == PIO
  #define ANSICOL //allow ansicolor and other codes to be used
#endif

#ifdef DEBUG 
 #define D_DEFAULT "\033[0m"
 #define D_BOLDRED "\033[1;31m"
 #define D_BOLDGREEN "\033[1;32m"

 #define SERIALINIT   Serial.begin(9600); delay(1000);
  #ifdef ANSICOL 
  #define D_PRINT_FORMAT(colcode, x)  Serial.print(colcode); Serial.print (x); Serial.print(D_DEFAULT);
  #define D_PRINT_FORMATLN(colcode, x)   Serial.print(colcode);  Serial.println (x); Serial.print(D_DEFAULT);
  #else
  #define D_PRINT_FORMAT(colcode, x) Serial.print(x);
  #define D_PRINT_FORMATLN(colcode, x)   Serial.println (x); 
  #endif 
 #define D_PRINT(x) Serial.print(x);
 #define D_PRINTLN(x)  Serial.println (x);
 #define D_PRINTXY(x,y) Serial.print(" "); Serial.print(x); Serial.print(" "); Serial.println(y);
 #define D_PRINTHEX(x) Serial.println(x,HEX);
#else
 #define SERIALINIT
 #define D_PRINT(x)
 #define D_PRINTXY(x,y) 
 #define D_PRINTLN(x)
 #define D_PRINTHEX(x)
 #define D_PRINT_FORMAT(colcode,x) 
 #define D_PRINT_FORMATLN(colcode, x)   
#endif


#endif