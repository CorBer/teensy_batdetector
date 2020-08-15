# teensy_batdetector V1.0 beta
Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)
https://github.com/DD4WH/Teensy-Bat-Detector )

<b>Hardware:</b>
Edwin (PE1WF) has developed a great PCB to build these detectors. More details can be found at https://forum.pjrc.com/threads/38988-Bat-detector?p=212852&viewfull=1#post212852 

<b>* v1.0beta 2020815 </b>
Repository is available with all recent code and libraries. 

Older versions of this project can be found at https://github.com/CorBer/teensy_batdetector-OLD-

This is a work in progress project, the code will keep changing. If you have a specific request or otherwise questions please share them on the forum. 

The src-directory contains all current sourcecode, several specific libraries have been added to this repository, in de directory <b>lib</b> you can find those.

I am using a non-default programming setup for Teensy. My primary setup is that I am on Linux Mint 19.3 and I use Microsoft VisualCode as my programming editor (IDE) with the PlatformIO plugin. That means some specific parts of the code (especially libraries) might not work directly in for instance the TeensyDuino environment. 
Thats why pre_compiled_hex files can be found on this repository.

Setup @2020-08-15

Operating system Linux Mint 19.3

<b>Visual Code 1.48.1</b><br>
-PlatformIO Core 4.3.4·Home 3.2.3

-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

