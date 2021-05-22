This directory contains HEX files that can be used directly on a teensy3.6 using the teensy loader.

**firmwareV1_2_20210522_080508_default.hex**
- this is the default release. Debug is OFF and PWM is ON. Even if you have not altered the TFT to have PWM controlling the backlight you can use this version.  

**firmwareV1_2_20210522_080550_debug.hex**
-this is the debug version (PWM is ON). This version can be of use if you encounter problems. The debug will be using the serialport at a 19200 baud 8N1 setting.

**firmwareV1_2_20210522_nopwm.hex**
- this is a version where PWM is not actively used. A few things will work slightly different and some menu options will possible be missing. !!THIS IS NOT TESTED AS MUCH AS THE ABOVE VERSIONS !!
