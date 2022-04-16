This is the repository of the TeensyBat V1_31 development released on 20220416
This is not a "full" release but a development update. The available code will support both the Teensy 3.6 and the Teensy 4.1

All provided sourcecode can be used directly in platformIO. The sourcecode should compile in any Arduino IDE environment but will need carefull setup of libaries. In PlatformIO all local libraries are in the "lib" subdirectory of the projectfile.

HEX: several prebuild HEX files for both Teensy 3.6 and Teensy 4.1
lib/src: source and libraries for PlatformIO (incl platformio.ini)

WARNING :If you want to use GPS make sure your module supports serial communication at 115200 baudrate and uses Ublox. We tested using the Beitian 180 and Beitian 220 GPS modules.

