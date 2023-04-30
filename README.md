# Teensy_Batdetector  (update 2023 03 17)
The Teensy Batdetector is cooperation between Edwin Houwertjes (hardware) and Cor Berrevoets (software). Development started in 2018 and since then both software and hardware have gone through major changes. We aim at providing a low-cost build-yourself bat detector.

# <b>Hardware:</b>
<b>Edwin </b> has developed a great PCB (current version 0.8) that allows you to build our detector with minimal skills. More details can be found at his website https://www.teensybat.com

![](images/overviewTeensybat.png?raw=true)

# <b>Current version v1.6 (released on 2023 03 17, sourcecode added 2023 04 30) </b>
see https://github.com/CorBer/teensy_batdetector/releases/tag/v1.6<br>


This version was developed in feb/mar 2023. 
The most important changes:
- <b> MediaTransferProtocol</b>: The detector now can be connected to a PC/Laptop and will show up as an MTP-device.
this will allow copying of files from the detector to a PC without removing the SD-card.
- <b>Screen on during recording</b>: the screen can now be left active (default is OFF) both during manual and automatic recording sessions (this feature is supported fully on T41but experimental for T36)
- <b>Deepsleep improvements</b>: Deepsleep now will behave properly based in the start-end times of the recording period. Too keep battery-usage minimal the T41 version will always switch to a lower clockspeed in the deepsleep mode (both during sleep and recording). To assist the user a status-screen information is now available when the screen is off during deepsleep, long-pressing the right-pushbutton will provide feedback of the status.
- <b>GPS improvements</b>: The GPS module (if mounted) demands quite a bit of current during usage, in the new version you can switch the GPS on/off to simply get a position and still preserve battery. 
- <b>Battery-indicator</b>: if a lipo battery-indicator is mounted this will provide better feedback of the battery-status.  This indicator also has an experimental estimator of the hours that are left in the battery.

# <b>previous version 1.5 :</b>
This Github repository contains the previous release (build 2023 01 28):<br>
https://github.com/CorBer/teensy_batdetector/releases/tag/v1.5
The original release on 2023 01 25 did not show the proper release-version.  


## <b>change to GRANULAR STRETCH</b>
In previous releases we used the acronym "TE" for one of the modes of the detector. A producer of commercial detectors has objected to us using that acronym as they have a patent on a live TimeExpansion technique. Although we are not using their technique we have renamedd our technique to GranularStretch(GS). The method we are using are enhancements of the standard granular-effect library from the Teensy Audio library. (see https://forum.pjrc.com/threads/53036-Batdetector-Time-expansion-(slow-down-replay)-using-granular-effect?highlight=granular+effect).

All code and hex of previous releases are therefore NOT available as converting all of them to respect this demand would mean a lot of work. If anybody wants a particular version please request and we will create that as a HEX file with the new GS acronym.

<b>Questions/Requests for the Teensy based Batdetector</b>
If you have questions please share them at 
https://forum.pjrc.com/threads/38988-Bat-detector

<b>Startup picture of a Daubenton's Bat </b><br>
The startup picture of the detector was kindly provided by Paul van Hoof https://www.paulvanhoof.nl/

<b>Based on original-code from Frank (DD4WH)</b>
https://github.com/DD4WH/Teensy-Bat-Detector 
<br>

