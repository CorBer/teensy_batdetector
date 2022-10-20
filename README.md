# Teensy_Batdetector  (update 2022 10 18)

<b>Teensy Batdetector forum</b>
https://forum.pjrc.com/threads/38988-Bat-detector

<b>Hardware:</b>
Edwin (PE1WF) has developed a great PCB to build these detectors. <br>More details can be found at https://forum.pjrc.com/threads/38988-Bat-detector?p=212852&viewfull=1#post212852 or <br>
at Edwins GDrive https://drive.google.com/drive/folders/1NRtWXN9gGVnbPbqapPHgUCFOQDGjEV1q
or<br> at https://www.teensybat.com

<b>Startup picture of a Daubenton's Bat </b>
kindly provided by Paul van Hoof https://www.paulvanhoof.nl/

<b>Based on original-code from Frank (DD4WH)</b>
https://github.com/DD4WH/Teensy-Bat-Detector 
<br>
# update 1.4beta1 available (2022 10 20)
- by default all optional hardware (GPS, LIPO etc) is tested at startup and will be used if available. This reduces the need for separate HEX files for different configurations. Only T41 with/without PSRAM will stay two separate HEX files. <br>
- monitoring of LIPO batterylife based on a MAX17043 chip (latest PCB from Edwin) was added<br>
- saving and restoring the configuration from an SD has improved. This allows transfer of configuration between TeensyBat machines <br>
       and also allows easy restoration of your settings between releases. There are also more settings stored in the configfiles.<br>
- deepsleep functionality had a limitation in V1_3, you could not set a wakeup of the device after 0h, now this is also possible<br>
- when playing a recording the switching between replay rates and Direct replay was not always reliable, now works as planned<br>
- Autorecording was not restoring all default settings after it was stopped by the user, this has been improved<br>
- in autoRecording the PRE-Buffer was deactivated in V1_31 it seems, this is restored <br>
- the GPS menu also shows coordinates in HH MM SS <br>
<br>
# update for 1.31 released after a bug-report (2022 04 16)
<i> About a week ago we got a bug-report that pointed towards an issue for both the T36 and the T41 versions
  of the teensybat using AutoRecord. When stopping the autorecord sequence manually with the recording active this could lead to a wav-file with a wrong header. Also the sample-rate after stopping autorecord was not as the user had set it. We have uploaded source and HEX files for V1.31 
</i>

# BUG in SPARKFUN GPS library
Beginning march 2022 a bug was found in the GPS readout, it seems the sparkfun library had a bug that reported 2022 as a leapyear. The library for teensybat has been updated and the bat_gps.h file is updated.

# LATEST RELEASE 
<b><h2> v1.31 development release update 20220416</h2> </b>
Complete Repository (HEX src/lib for platformIO) available at: <br>
"https://github.com/CorBer/teensy_batdetector/tree/master/version1_31%20development"
<br>
<br>

<b><h2> v1.3 development release 20210909</h2> </b>
Complete Repository (HEX src/lib for platformIO) available at: <br>
https://github.com/CorBer/teensy_batdetector/tree/master/version1_3development
<br>
<br>

<b><h2> v1.2 release 20210522</h2> </b>
Complete Repository (HEX/src/lib) available at: <br>
https://github.com/CorBer/teensy_batdetector/tree/master/version1_2
<br>
<br>
# AVAILABLE SOURCECODE / HEXFILES

<b> v1.1development version 20210409 </b>
Repository available since 9 april 2021 <br>
https://github.com/CorBer/teensy_batdetector/tree/master/version1_1develop

<b> v1.0development version 20200925 </b>
Repository is available with all recent code (subdirectory of src) and pre-compiled hexfiles. 
https://github.com/CorBer/teensy_batdetector/tree/master/version1_0

Repository is available with all recent code and libraries. 

<i>This is a work in progress project, the code will keep changing. If you have a specific request or otherwise questions please share them on the forum. 

For each version you will find a src-directory containing all current sourcecode, several specific libraries have been added to lib-directory for each version. 

I am using a non-default programming setup for Teensy. My primary setup is that I am on Linux Mint 19.3 and I use Microsoft VisualCode as my programming editor (IDE) with the PlatformIO plugin. Pre_compiled_hex files made on my machine can be found on the various versions.</i>

Setup @2021-05-19

Operating system Linux Mint 19.3

<b>Visual Code 1.56.2</b><br>
-PlatformIO Core 5.1.1·Home 3.3.4

-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

