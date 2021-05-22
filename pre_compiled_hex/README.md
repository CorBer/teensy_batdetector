This directory contains pre_compiled_hex versions to be used on Teensy3.6 

-firmware20200815.hex  - V1.0 beta<br>
-firmwaredev_20201006.hex V1.0 development version<br>
<br>
-firmware_20210409_v1_1dev.hex V1.1 development version<br>
-firmware_20210409_v1_1dev_DEBUG.hex V1.1 development version with DEBUG enabled (serial 9600 8N1)

**firmwareV1_2_20210522_080508_default.hex**
this is the default release. Debug is OFF and PWM is ON. Also if you have not altered the TFT to use PWM to control the backlight you can still use this version.

**firmwareV1_2_20210522_080550_debug.hex**
this is the debug version (PWM is ON). This version can be of use if you encounter problems. The debug will be using the serialport at a 19200 baud 8N1 setting.

**firmwareV1_2_20210522_nopwm.hex**
this is a version where PWM is not actively used. A few things will work slightly different and some menu options will possible be missing.


