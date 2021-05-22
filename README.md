# Teensy_Batdetector  (update 2021 05 22)

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

# NEW RELEASE 
<b> v1.2 release 20210522 </b>
Repository available at: <br>
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

Older versions of this project can be found at https://github.com/CorBer/OLD-teensy_batdetector

<i>This is a work in progress project, the code will keep changing. If you have a specific request or otherwise questions please share them on the forum. 

For each version you will find a src-directory containing all current sourcecode, several specific libraries have been added to lib-directory for each version. Specific HEX (pre-compiled version) can be found for each version or in the overall collection at https://github.com/CorBer/teensy_batdetector/tree/master/pre_compiled_hex

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

