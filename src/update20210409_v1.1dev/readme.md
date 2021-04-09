DEVELOPMENT UPDATE
- this is not a definitive release but a development update
- the source and HEX files have new and changed features 
- most of the features have been tested but not by many people

/* TEENSY 3.6 PINSETUP (20210409) <br>
    pins marked with X are default in use<br>
    pins marked with * are used for optional functions<br>
    pins mared with . are not (yet) in use<br>
   
                 GND X                X Vin  - PREAMP V+  
       GPS TX -> RX0 *                X  Analog GN D    
       GPS RX -> TX1 *                X  3.3V - MEMS MIC         
                   2 .                X  23 AUDIO -LRCLK         
      I2C reserved 3 *                X  22 AUDIO -TX             
      I2C reserved 4 *                X  21 <-TFT CS                
                   5 .                X  20 <-TFT DC             
       AUDIO MEMCS 6 X                X  19 AUDIO - SCL         
       AUDIO MOSI  7 X                X  18 AUDIO - SDA         
                   8 .                *  17 A3                <  TFT-PWM option        
       AUDIO BCLK  9 X                *  16 A2 - ADC          <- ADC_IN function                               
       AUDIO SDCS 10 X                X  15 AUDIO -VOL                      
       AUDIO MCLK 11 X                X  14 AUDIO -SCLK                     
       AUDIO MISO 12 X                X  13 AUDIO -RX                       
                3.3V X                X  GND                 
                  24 .                .  A22 DAC1
                  25 .                .  A21 DAC0 
                  26 .                X  39  TFT MISO
      TFT SCLK->  27 X                X  38  MICROPUSH_L
      TFT MOSI->  28 X                X  37  MICROPUSH_R
     ENC_L-BUTTON 29 X                X  36  ENC_R-BUTTON
     ENC_L A      30 X                X  35  ENC_R A
     ENC_L B      31 X                X  34  ENC_R B
     DS18B20  T   32 *                .  33

*/
