# Teensy_Batdetector V1.0 beta
<b>Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)</b>
https://github.com/DD4WH/Teensy-Bat-Detector )

<b>Teensy Batdetector forum</b>
https://forum.pjrc.com/threads/38988-Bat-detector

<b>Hardware:</b>
Edwin (PE1WF) has developed a great PCB to build these detectors. More details can be found at https://forum.pjrc.com/threads/38988-Bat-detector?p=212852&viewfull=1#post212852 

<b>startup picture of a Daubenton's Bat </b>
kindly provided by Paul van Hoof https://www.paulvanhoof.nl/

<i>This is a work in progress </i>

<b> v1.1development version 20210409 </b>
Repository will become available in april 2021 <br>
https://github.com/CorBer/teensy_batdetector/tree/master/src/update20210409_v1.1dev

<b> v1.0development version 20200925 </b>
Repository is available with all recent code (subdirectory of src) and pre-compiled hexfiles. 
https://github.com/CorBer/teensy_batdetector/tree/master/src/update20200925

<b> v1.0beta 2020815 </b>
Repository is available with all recent code and libraries. 

Older versions of this project can be found at https://github.com/CorBer/OLD-teensy_batdetector

<i>This is a work in progress project, the code will keep changing. If you have a specific request or otherwise questions please share them on the forum. 

The src-directory contains all current sourcecode, several specific libraries have been added to this repository, in de directory <b>lib</b> you can find those.

I am using a non-default programming setup for Teensy. My primary setup is that I am on Linux Mint 19.3 and I use Microsoft VisualCode as my programming editor (IDE) with the PlatformIO plugin. That means some specific parts of the code (especially libraries) might not work directly in for instance the TeensyDuino environment. 
Thats why pre_compiled_hex files can be found on this repository.</i>

Setup @2020-08-15

Operating system Linux Mint 19.3

<b>Visual Code 1.48.1</b><br>
-PlatformIO Core 4.3.4·Home 3.2.3

-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

