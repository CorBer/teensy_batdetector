
<h1> <b> TeensyBat1.4beta1 release (20221020)</b> </h1>

<H1> <b>HEX FILES</b> </H1>
There are only 3 HEX files, one for the Teensy36(T36) and two for the Teensy41 (T41).
For the T41 the HEX is compiled with and without mounted PSRAM. The setup of the current
code allows us to have all features available for TeensyBat. At startup the software will
test which features are actively in use and will only use those. This allows for instance
to add a GPS module later to your PCB without the need for recompiling.
<br><b>
If you have problems, the DEBUG versions of the HEX files can be used to see possible issues
on the serialport of the Teensy.</b>

<H1> <b>SOURCE CODE</b> </H1>
The raw code is available in a single zip file. This contains all the necessary 
code and libraries we used for compilation on PlatformIO under Visual Studio Code.
Several changes have been made in the platformio.ini file to make this 
optimized for easier code generation and clearer naming of the firmware.hex files.
This was tested both on Linux (Cor) as on Windows (Edwin).  

