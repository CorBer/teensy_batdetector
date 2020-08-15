# teensy_batdetector V1.0 beta
Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)
https://github.com/DD4WH/Teensy-Bat-Detector )

<b>Hardware:</b>
Edwin (PE1WF) has developed a great PCB to build these detectors. More details can be found at https://forum.pjrc.com/threads/38988-Bat-detector?p=212852&viewfull=1#post212852 

<b>* v1.0beta 2020815 </b>
Currently building up the repository with all necessary code. Older versions can still be found at https://github.com/CorBer/teensy_batdetector

This is a work in progress project, the code and setup is still changing. If you have a specific request or otherwise questions please share them. 

Several specific libraries have been added to this repository, in de directory <b>lib</b> you can find them.

I am using a non-default programming setup for Teensy, that means that the code might not work in other environments. Thats why pre_compiled_hex files can be found on this repository.

Setup @2019-06-22

Operating system Linux Mint 19.1

<b>Visual Code 1.35.1</b><br>
-PlatformIO Home 2.2.0·Core 3.6.7<br>
-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

