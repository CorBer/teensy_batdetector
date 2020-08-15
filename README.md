# teensy_batdetector V1.0 beta
Batdetector on Teensy3.6 (Based on original-code from Frank (DD4WH)
https://github.com/DD4WH/Teensy-Bat-Detector )

<b>Hardware:</b>
Edwin (PE1WF) has developed a great PCB to build these detectors. More details can be found at https://forum.pjrc.com/threads/38988-Bat-detector?p=212852&viewfull=1#post212852 

<b>* v1.0beta 20200722 </b>
coming soon

<b>* v0.99/v.100 </b> 
several initial changes to allow automatic recording. This will allow the detector to wait for a signal in a band of the spectrum and start recording X (user set) seconds. After this it will wait Y (user set) seconds before trying to detect the next signal.
This mode allows the detector to record unattended but at the cost of high-continous powerusage (so either connect to a large battery or another powersource)

<b>* v0.97/0.98 </b> 
- fixed bug in the replay mode: when choosing a file the replay-sample_rate changed also
- added routines to monitor VIN (A3 ADC on pin17 using a voltage divider and capacitor) see #define USE_VIN_ADC
- added routine to control TFT PWM from pin3, see #define USE_PWMTFT
- added compiler option PIO (platformiobased project) or ARDUINO (teensyduino/arduino IDE) 

*<b> v0.96 (20190824) MAJOR CHANGES </b>
- compacted the main menu to have all less often changed settings in a separate SETTINGS page <br>
- setting of both time and date can be done from the new settings page
- updated startup from EEprom stored settings (more settings stored (see settings page))<br>
- improved spectrum-graph functionality (peak-hold now visibible, was present for many versions but invisible) <br>

- made as many as possible user-changeable options cyclic (if the user tries to rotate the encoder above the highest value automatically go to the lowest value) <br>
- time was updating still on screen every minute during recording, now has been removed
- added a routine to monitor VIN (or anything else) voltage on A3 (pin17) <br>
- read unique SN (allows identifying different detectors, one for testing with additional functions) <br>


latest changes<br>
<b> v0.94 important changes:</b>
- Lower menu format on screen now follows the alignment of the encoders/buttons on Edwins PCB. So the encoders control the lower line on bottom part of the screen(SETTINGS) and the buttons control the upper line on the bottom of the screen (MODES). <br>
- EEprom saving was not always when asked to restart in the user mode<br>
- when no SD card was mounted the Left Encodermenu was not cyclic (jumped back to volume)<br>
- when no sD card was mounted user could still set the LEFT button to RECORD or PLAY<br>

<b>minor changes</b>:
- store the detectormode when starting a recording and restore after stopping a recording<br>

 <b> bug repair:</b>
  in previous versions the recording system was not checking if a filename allready existed but only checked the number of files on the SD. If a file was removed (for instance from a PC) this would lead to overwriting existing files. <br>
  In the new setup the new filenumber for a recording will be checked against highest filenumber in the
          allready stored files.

<b>v0.93</b>
minor changes:<br>
 - changing sample_rate was not stored properly in previous versions<br>
 - added system-voltage in startup-display (for future battery managment)<br>
 

<b>v0.92</b>
 - further work on the setup to record/play files <b>NEEDS TESTING</b> <br>
 - filename display shortened so no overlap with other menu <br>
 - filename selection (when playing) is now cyclic <br>
 To go to the latest recording just turn counterclockwise instead of stepping through all previous  <br>
 - rightside menu allows setting of samplerate (in all modes) <br>
 - during playing of files microphones will be switched off <br>
 - maximum number of files to be stored 999 (BXXX_RRR.RAW) XXX=0..999 RRR=samplerate in Khz <br>


<b>0.91</b><br> intermediate test version !! 
-this hopefully solves the issues with replay of recorded files that were present since v0.84 

<b>IMPORTANT: updates since version 0.84 have problems with replaying files from the SD-card. Recording works fine. From V0.91 working again</b> 

<b>0.89</b><br> 
- eeprom version 0001
- indicator for low-high detectionrange in graph for AUTO_TE <br>
- menu updated and more uniform (all choosen values shown in white next to menusetting also for TE_LOW/TE_SPD) <br>
- added counter to keep track of EEprom saving (still reset to 0 for each EEprom version)<br>
- added text to display after pressing PRESET USER to save to EEprom<br>


<b>0.88</b><br>(source and hex)<br>
-EEprom saving added (initial test)<br>
-Menu expanded with <br>
- Preset: Default (as programmed) or User (saved at that moment)
-  TE_speed : replay speed when using Auto_TimeExpansion (20 means 1/20th of original speed)<br>
-  TE_low : lowest frequency to check for possible signalpeaks that will trigger the TimeExpansion<br>
  


<b>0.86</b><br>
-changes by WMXZ to use latest uSDFS library and not be depending on ff_utils and SD.h<br>


<b>v0.85</b> (23 06 2019) <br>
  -changed the usage of seconds2tm from the ff_utils library,<br> 
   created ff_utils_copy library that is independent of other uSDFS libraries.


This is a work in progress project, the code and setup is still changing. If you have a specific request or otherwise questions please share them. 

Several specific libraries have been added to this repository, in de directory <b>lib</b> you can find them.

I am using a non-default programming setup for Teensy, that means that the code might not work in other environments.

Setup @2019-06-22

Operating system Linux Mint 19.1

<b>Visual Code 1.35.1</b><br>
-PlatformIO Home 2.2.0·Core 3.6.7<br>
-platformio.ini<br>
  [env:teensy36]<br>
  platform = teensy<br>
  board = teensy36<br>
  framework = arduino<br>

