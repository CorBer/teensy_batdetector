# Teensy_Batdetector  (update 2023 01 28)
The TeensyBat detector is a close cooperation between Edwin Houwertjes (hardware) and Cor Berrevoets (software). Development started in 2018 and since then both software and hardware have gone through major changes. We aim at providing a low-cost build-yourself bat detetector for anyone.

<b>Software:</b>
This Github repository contains the latest release (build 2023 01 28):<br>
https://github.com/CorBer/teensy_batdetector/releases/tag/v1.5

<b>Hardware:</b>
Edwin has developed a great PCB (current version 0.8) that allows you to build our detector with minimal skills. <br>More details can be found at his website https://www.teensybat.com


## <b>change to GRANULAR STRETCH</b>
In previous releases we used the acronym "TE" for one of the modes of the detector. A producer of commercial detectors has objected to us using that acronym as they have a patent on a live TE technique. Although We Are not using their technique We Are now renaming this mode to GS. This stands for Granular Stretch, which is the actual method We Are usign based on the standard granular-effect library in the Teensy Audio library. (see https://forum.pjrc.com/threads/53036-Batdetector-Time-expansion-(slow-down-replay)-using-granular-effect?highlight=granular+effect).

All code and hex of previous releases are currently NOT available  as converting all of them to respect this demand would mean a lot of work. If anybody wants a particular version please request and we will create that as a HEX file with the new GS acronym.

<b>Teensy Batdetector forum</b>
If you have questions please share them at 
https://forum.pjrc.com/threads/38988-Bat-detector


<b>Startup picture of a Daubenton's Bat </b><br>
The startup picture of the detector was kindly provided by Paul van Hoof https://www.paulvanhoof.nl/

<b>Based on original-code from Frank (DD4WH)</b>
https://github.com/DD4WH/Teensy-Bat-Detector 
<br>

