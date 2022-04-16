/* TEENSYBAT DETECTOR (for TEENSY 3.6/4.1)

 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _BAT_SD_H
#define _BAT_SD_H

#include "bat_vars.h"

uint16_t filecounter = 0;
uint16_t dircounter = 0;

uint16_t autocounter; //count the number of recorded files in autorecording mode
uint32_t file_duration = 0;
char savedir[80];
boolean dir_create = false;
char filename[80];
char longfilename[80];
char dirname[80];
boolean root_active = false;
char active_dir[80];
char lastactive_dir[80];

boolean dir_choosen = false;
uint16_t playfolder = 0;
uint16_t last_active_folder = 0;

uint16_t active_folder = 0;
uint32_t SD_USED = 0;
uint32_t SD_SIZE = 0;

char guano_filename[80];

boolean SDFATactive = false;

//create 1/2Kb infoblok
//const unsigned int icomment_bytes=512-44;
const unsigned int iheaderSize = 256; //total size of infochunk in bytes

// Subchunk Descriptor (4 bytes)
//     Subchunk Size (4 byte integer, little endian)
//     Subchunk Data (size is Subchunk Size)

//create 256byte info chunck for wave file
FLASHMEM
char* InfoChunk()
  {
  static char iheader[iheaderSize];
  strcpy(iheader, "LIST");                     //infochunk is a LIST
  *(int32_t*)(iheader + 4) = iheaderSize - 8; //iheaderSize-8 4 bytes total length of upcoming info chunk so excluding 8 bytes from LIST en list_len
  //when using infochunk this is the default starting position of the infodata
  strcpy(iheader + 8, "INFO"); //4 bytes start of info chunk   (chunk=4) //infoblock starts, size was set in the previous line
  //default position for 1st fourcc block
  char tstr[40]; //maximum stringlength
  unsigned int bufpos = 12;
  unsigned int l = 0;
  strcpy(iheader + bufpos, "IART");
  bufpos += 4;                         //4 bytes artist name fourcc          8 //artist name, size of the textblock is on next line
  *(int32_t*)(iheader + bufpos) = 16; //4 bytes mxlength of IART chunk     12
  bufpos += 4;
  strcpy(iheader + bufpos, "TEENSYBAT "); //up to 16 bytes data for IART  28
  bufpos += 16;                           //positie 36

#ifdef USE_GPS //store GPSdata in TITLE FIELD fixed 40
  strcpy(iheader + bufpos, "INAM");
  bufpos += 4; //4 bytes artist name fourcc          8 //artist name, size of the textblock is on next line
  //"Position: 53.212368 5.907277
  //gps_latitude=53212368;
  //gps_longitude=5907277;
  l = snprintf(tstr, 40, "Position: %.7f %.7f ", float(gps_latitude) / 10000000, float(gps_longitude) / 10000000);
  *(int32_t*)(iheader + bufpos) = 40; //4 bytes mxlength of IART chunk     12
  bufpos += 4;
  strcpy(iheader + bufpos, tstr); //up to 16 bytes data for IART  28
  bufpos += 40;                   //positie 36
#endif

  strcpy(iheader + bufpos, "ICMT"); //4 bytes comment name fourcc        32
  bufpos += 4;
  *(int32_t*)(iheader + bufpos) = iheaderSize - bufpos - 4; //4 bytes mxlength of ICMT chunk     36
  bufpos += 4;
  // start saving comment data into the infochunk
  int HiP = 0; //set OFF

  HiP = HI_pass;
  if (HiP == HI_PASS_OFF)
    {
    HiP = 0;
    }
  if (AGC_mode == 1) //AGC was ON
    {
    l = snprintf(tstr, 40, "AGCGain %.1f to %.1fdB", gainstop2gain(AGC_low), gainstop2gain(AGC_high));
    strcpy(iheader + bufpos, tstr);
    bufpos += l;
    }
  else //AGC off normal gain
    {
    l = snprintf(tstr, 40, "Gain %.1fdB ", real_gain);
    strcpy(iheader + bufpos, tstr);
    bufpos += l;
    }

  l = snprintf(tstr, 40, "|GAIN-ATT ON");
  strcpy(iheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "|Hi_pass %02dKhz ", HiP);
  strcpy(iheader + bufpos, tstr);
  bufpos += l;

  if (AUTO_REC)
    {
    l = snprintf(tstr, 40, "|AREC_F %02d ", AREC_F);
    strcpy(iheader + bufpos, tstr);
    bufpos += l;

    l = snprintf(tstr, 40, "|AREC_S %04d ", AREC_S * 25);
    strcpy(iheader + bufpos, tstr);
    bufpos += l;
    }

#ifdef USE_DS18B20
  l = snprintf(tstr, 40, "|Temp %.1f C", bat_tempC);
  strcpy(iheader + bufpos, tstr);
  bufpos += l;
#endif

  l = snprintf(tstr, 40, " ");
  strcpy(iheader + bufpos, tstr);
  bufpos += l;
  return iheader;
  }

PROGMEM const unsigned int gheaderSize = 512;
// Guano chunk modelled on InfoChunk to append a Guano metadata section to the .wav file

//create 1/2Kb infoblok for GUANO data storage
FLASHMEM
char* GuanoChunk()
  {
  static char gheader[gheaderSize];
  strcpy(gheader, "guan"); // guano chunk has type guan

  *(int32_t*)(gheader + 4) = gheaderSize - 8;

  char tstr[40];           //maximum stringlength
  unsigned int bufpos = 8; //starting position of the chunk
  unsigned int l = 0;      //length of the last char-string

  l = snprintf(tstr, 40, "GUANO|Version: 1.0\n");
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Make: TeensyBat\n");
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Firmware Version: %s\n", batversion);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "TB| Gain: %.1fdB\n", real_gain);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  int HiP = 0; //set OFF
  HiP = HI_pass;
  if (HiP == HI_PASS_OFF)
    {
    HiP = 0;
    }
  l = snprintf(tstr, 40, "Filter HP: %02dKhz\n", HiP);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Samplerate: %06lu\n", SR[active_recording_SR].osc_frequency);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  //char shortname[11];
  //snprintf(shortname,11,guano_filename);
  l = snprintf(tstr, 40, "Original Filename: %11s\n", guano_filename);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Timestamp: %20s\n", tRecStart);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

#ifdef USE_GPS
  l = snprintf(tstr, 40, "Loc Position: %.6f %.6f \n", float(gps_latitude) / 10000000, float(gps_longitude) / 10000000);
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Loc Accuracy: %.2f \n", float(gps_HACC / 1000));
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

  l = snprintf(tstr, 40, "Loc Elevation: %.1f \n", float(gps_altitude / 1000));
  strcpy(gheader + bufpos, tstr);
  bufpos += l;

#endif
#ifdef USE_DS18B20
  float timecorrect = 1.0; // time based temperature correction in about 20 minutes the T41 warms up.

#ifdef TEMP_TIMECORRECT
  uint32_t temp_time = millis();
  if (temp_time < 20 * 60000)
    {
    timecorrect = float(temp_time) / float(20 * 60000);
    //Serial.println(timecorrect);
    }
#endif  

  if (bat_tempC != -127)
    {
    l = snprintf(tstr, 40, "Temperature Ext: %.1f \n", bat_tempC + T_corrector * timecorrect);
    }
  else
    {
    l = snprintf(tstr, 40, "Temperature Ext: NC \n");
    }
  strcpy(gheader + bufpos, tstr);
  bufpos += l;
#endif

  return gheader;
  }

const uint8_t tbat_chunk = 128; //v1 only 120 bytes for powerspectrum
const uint8_t tbat_data = tbat_chunk - 8;

const uint8_t wavHeaderSize = 44 + tbat_chunk;

//create wavheader (44 bytes) and tbat_powerspectrum (120 bytes)
FLASHMEM
char* wavHeader(uint32_t fileSize)
  {
  int fsamp = SR[active_recording_SR].osc_frequency;
  D_PRINTXY("FSAMP", fsamp)
    int nchan = 1;

  int nbits = 16;
  int nbytes = nbits / 8;

  int nsamp = (fileSize - wavHeaderSize) / (nbytes * nchan); //we steal 44 bytes from the original recording
  //
  static char wheader[48 + tbat_chunk]; //
  //
  strcpy(wheader, "RIFF");
  *(int32_t*)(wheader + 4) = 36 + tbat_chunk + iheaderSize + gheaderSize + nsamp * nchan * nbytes; //filesize specifier is samples+header+infochunk

  strcpy(wheader + 8, "WAVE");
  strcpy(wheader + 12, "fmt ");

  *(int32_t*)(wheader + 16) = 16;             // chunk_size
  *(int16_t*)(wheader + 20) = 1;              // PCM
  *(int16_t*)(wheader + 22) = 1;              // numChannels
  *(int32_t*)(wheader + 24) = fsamp;          // sample rate
  *(int32_t*)(wheader + 28) = fsamp * nbytes; // byte rate
  *(int16_t*)(wheader + 32) = nchan * nbytes; // block align
  *(int16_t*)(wheader + 34) = nbits;          // bits per sample
  strcpy(wheader + 36, "tbat");
  *(int32_t*)(wheader + 40) = tbat_data; // tbat datachunk_size, real tbat_data starts at 44
  strcpy(wheader + 44 + tbat_data, "data");
  *(int32_t*)(wheader + 48 + tbat_data) = nsamp * nchan * nbytes;
  return wheader;
  }

#define MAX_FILES 1000
#ifndef PSRAM
DMAMEM uint16_t fileindex[MAX_FILES];
DMAMEM uint16_t dirindex[MAX_FILES];
#else
EXTMEM uint16_t fileindex[MAX_FILES];
EXTMEM uint16_t dirindex[MAX_FILES];
#endif
uint16_t fileselect = 0;
int referencefile = 0;

/************************************USDFS****************************************************/

// #include "uSDFS.h"       // uSDFS lib  - do not uSDFS master !!

//   FRESULT rc;        /* Result code */
//   FATFS fatfs;      /* File system object */
//   FIL fil;        /* File object */

//   //uint32_t count=0;
//   uint32_t ifn=0;
//   uint32_t isFileOpen=0;

//  uint32_t t0=0;
//  uint32_t t1=0;

//switch to SDFAT operation (reading)
// void switch2SDfat()
// {  D_PRINTLN("SWITCH TO SDFAT");

//   //switch off the fatfs mount
//     SDFATactive=true;
//     const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
//     f_mount (nullptr, Device, 1);      /* Unmount the logical drive */
//     SD.begin(BUILTIN_SDCARD); //make ready for playing

// }

//switch to FATFS operation (writing)
// void switch2fatfs()
// {    D_PRINTLN("***SWITCH TO FATFS");

//     SDFATactive=false;
//     const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
//     rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
//     //Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
//     SD_SIZE=(fatfs.n_fatent-2)*fatfs.csize*0.000512f;
//     SD_USED=SD_SIZE- (fatfs.free_clst*fatfs.csize*0.000512f);
//     rc = f_chdrive(Device);
// }

//initialize the SDcard by first reading the FATFS cardsize etc and finally use SDFAT to open the card for reading
// int initSD(void)
//   { 

//     D_PRINTLN("***SWITCH TO FATFS at INITSD");

//     const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
//     rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
//     f_mount (nullptr, Device, 1);
//     D_PRINTLN("***SWITCH TO SDFAT at INITSD");


//     return rc; //SD.begin(BUILTIN_SDCARD);
//   }

//get ready to play a wave-file using SDFAT
// void initPlay(void)
// { 
//   //switch off the fatfs mount
//   // const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
//   // f_mount (nullptr, Device, 1);      /* Unmount the logical drive */
//   // SD.begin(BUILTIN_SDCARD); //make ready for playing
//   switch2SDfat();

// }

// //initialize recording setup by switching to fatFS (usdfs setup)
// void initREC(void)
// { 
//   D_PRINTLN_F(D_BOLDGREEN,"INITREC");

//  #ifdef USE_PSRAM // USE PSRAM
//    // switch2SDfat();
//  #else  //DONT USE PSRAM
//    // switch2fatfs();
//  #endif
// }

/*********************************** SDFAT mode 3****************************************/

// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3 // USE multitype

// select the fastest SD card configuration (SDIO)
#define SD_CONFIG SdioConfig(FIFO_SDIO)

// #if SD_FAT_TYPE == 0
// SdFat sd;
// FsFile dir;
// FsFile file;
// #elif SD_FAT_TYPE == 1
// SdFat32 sd;
// File32 dir;
// File32 file;
// #elif SD_FAT_TYPE == 2
// SdExFat sd;
// ExFile dir;
// ExFile file;
// #elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
FsFile root;

// #endif // SD_FAT_TYPE

/**********************************************************************************************/

//read the duration and samplerate from the wav-header, if stored also get the powerspectrum
FLASHMEM
uint32_t getfileSR()
  {
  uint32_t fileSR = 0;
  uint32_t filelen = 0;
  D_PRINTLN("GETFILESR");

  snprintf(longfilename, 80, "%s/%s", active_dir, filename);
  D_PRINTXY("longfile:", longfilename);

  File audioFile = SD.open(longfilename, FILE_READ);
  audioFile.seek(0);
  byte audiobuffer[128]; //large enough to read data
  audioFile.read(audiobuffer, 36);
  filelen = audiobuffer[4] + audiobuffer[5] * 256 + audiobuffer[6] * 256 * 256 + audiobuffer[7] * 256 * 256 * 256 - 36 - tbat_chunk - iheaderSize - gheaderSize;
#ifdef DEBUG_DETAIL
  D_PRINTXY("FILELEN", filelen);
#endif

  fileSR = audiobuffer[24] + audiobuffer[25] * 256 + audiobuffer[26] * 256 * 256;
  if (fileSR != 0)
    {
    file_duration = uint32_t(filelen * 500.0f / fileSR); //time in ms (filelength=bytes, 2bytes=1sample )
    }
  else
    {
    file_duration = 0;
    }

  D_PRINTXY("recordedSR ", fileSR);
  //test for spectrum

  //check if the file has a TBAT marking that is used for powerspectrum storage
  audioFile.read(audiobuffer, 4);

  if (memcmp(audiobuffer, "tbat", 4) == 0)
    {
    D_PRINTLN("SPECTRUM FOUND");
    wavPSpectrum = true;
    audioFile.read(audiobuffer, 4); //read size indicator
    audioFile.read(audiobuffer, 120);
    for (uint8_t i = 0; i < 120; i++)
      {
      wavPowerSpectrum[i] = audiobuffer[i];
      }
    }
  else
    {
    for (uint8_t i = 0; i < 120; i++)
      {
      wavPowerSpectrum[i] = 0;
      }
    }

  audioFile.close();

  return fileSR;
  }

//only count the no of files in a directory
uint16_t FilesinDirIndex(uint16_t idx)
  {
  uint16_t wavfiles = 0;
  dir.open("/");
  file.open(&dir, idx, O_RDONLY);
  file.getName(dirname, 80);
  file.close();
  dir.close();
  D_PRINTXY("DIR: ", dirname);
  dir.open(dirname);
  while (file.openNext(&dir, O_RDONLY))
    {
        {
        file.getName(filename, 80);
        if ((String(filename).endsWith(".WAV")) or (String(filename).endsWith(".wav")))
          {
          wavfiles++;
          }
        file.close();
        }
    }

  file.close();
  dir.close();

  return wavfiles;
  }

// count and store the index of wav-files in a directory (idx=directory_index)
FLASHMEM
void countFilesinDir_byindex(uint16_t idx)
  {
  D_PRINTLN("countFilesinDir_byindex");
  D_PRINTXY("idx", idx);

  dir.open("/");
  file.open(&dir, idx, O_RDONLY);
  file.getName(dirname, 80);
  file.close();
  dir.close();
  D_PRINTXY("DIR: ", dirname);

  dir.open(dirname);
  dir.rewind();

  filecounter = 0;
  boolean filesfound = false;
  while (file.openNext(&dir, O_RDONLY))
    {
        {
        file.getName(filename, 80);
        if ((String(filename).endsWith(".WAV")) or (String(filename).endsWith(".wav")))
          {
          fileindex[filecounter] = uint32_t(dir.curPosition() / 32) - 1; //records the fileposition

          filesfound = true;
          if (filecounter < MAX_FILES - 1)
            {
            filecounter++;
            }
          }
        file.close();
        }
    }

  file.close();
  dir.close();
  if (filesfound)
    {
    D_PRINTXY("COUNTED_FILES", filecounter);
    }
  else
    {
    D_PRINTLN("NO_FILES");
    }
  }

/// get the filename (and samplerate) or directoryname by its index
FLASHMEM
void readFileInfo_byindex(uint16_t idx)
  {
  D_PRINT("FILE READbyIndex:");
  D_PRINTXY("FILE idx:", idx);

  dir.close();
  dir.open(active_dir);
  D_PRINTXY("activedir:", active_dir);
  file.open(&dir, idx, O_RDONLY);
  file.getName(filename, 80);
  D_PRINTXY("name", filename);
  dir.close();

  if (!root_active) // in root do not try to read samplerate as we are looking at dirs
    {
    recorded_SR = getfileSR();
    }
  }

//check for dirs in the root of the SDcard and store the index to the directory (to allow fast readouts)
FLASHMEM
void countDirs_inroot(void)
  {
  // switch2SDfat();
  file.close();
  dir.close();

  dir.open("/");

  D_PRINTLN("countDirs_inroot");

  dir.rewind();
  dircounter = 0;
  boolean dirfound = false;
  while (file.openNext(&dir, O_RDONLY))
    {
    file.getName(dirname, 80);

    if (file.isDir())
      {
    #ifdef DEBUG_DETAIL
      D_PRINTXY("found ", dirname);
    #endif
      dirindex[dircounter] = uint32_t(dir.curPosition() / 32) - 1;
    #ifdef DEBUG_DETAIL
      D_PRINTXY("DIR IDX", dirindex[dircounter]);
    #endif
      dirfound = true;
      if (dircounter < MAX_FILES - 1)
        {
        dircounter++;
        }
      }
    file.close();
    }
  file.close();
  dir.close();

  //very crude sort
  dir.open("/");
  char temp_dir[80];

  uint16_t swapcount = dircounter;

  swapcount = 1; //fake
  for (int no = 0; no < (dircounter - 1); no++)
    {
    if (swapcount > 0) //only continue if the last run still had swaps
      {
      swapcount = 0;
      for (int o = 0; o < (dircounter - (no + 1)); o++)
        {
        file.open(&dir, dirindex[o], O_RDONLY);
        file.getName(dirname, 80); //save selected directory filename
        file.close();
        file.open(&dir, dirindex[o + 1], O_RDONLY);
        file.getName(temp_dir, 80); //save selected directory filename
        file.close();
        if (atoi(dirname) > atoi(temp_dir))
          {
          uint16_t t = dirindex[o];
          dirindex[o] = dirindex[o + 1];
          dirindex[o + 1] = t;
          swapcount++;
          }
        }
      // D_PRINTXY("SWAPS ",swapcount)
      }
    }
  if (dirfound)
    {
    D_PRINTXY("COUNTED_DIRS", dircounter);
    }
  else
    {
    D_PRINTLN("NO DIRS");
    }
  file.close();
  dir.close();
  }

//start recording to the SDcard based on sdFAT only
FLASHMEM
void startREC(void)
  {
  //initREC();

  D_PRINTLN_F(D_BOLDGREEN, "STARTREC");

  // always close file to be sure
  if (file.isOpen())
    {
    file.close();
    }

  seconds2time(getRTC_TSR());

  snprintf(savedir, 80, "%04d%02d%02d", tm_year + 1970, tm_mon, tm_mday);
  snprintf(filename, 80, "%04d%02d%02dT%02d%02d%02d.wav", tm_year + 1970, tm_mon, tm_mday, tm_hour, tm_min, tm_sec);
  snprintf(guano_filename, 80, filename);

  snprintf(tRecStart, 20, "%04d-%02d-%02dT%02d:%02d:%02d", tm_year + 1970, tm_mon, tm_mday, tm_hour, tm_min, tm_sec);
  D_PRINTXY("SAVEDIR ", savedir);
  bool result = root.open("/"); //point root at "/"
  D_PRINTXY("ROOT OPEN", result);
  result = dir.mkdir(&root, savedir); //create dir
  D_PRINTXY("mkdir ", result);

  D_PRINTXY("exists ", root.exists(savedir));

  if (result == true)
    {
    D_PRINTLN("SDFAT MKDIR TRUE");

    dir_create = true;                                         // a new dir was created
    snprintf(filename, 80, "/%s/%s", savedir, guano_filename); //change filename to filename and dir
    }
  else
    {
    if (root.exists(savedir))
      {
      D_PRINTLN("SDFAT DIR EXISTS ");
      snprintf(filename, 80, "/%s/%s", savedir, guano_filename); //change filename to filename and dir
      }
    else
      {
      D_PRINTLN("SDFAT MKDIR FALSE "); //dir was not created file will be fitted in root
      }
    }
  D_PRINTLN(filename);

  result = file.open(filename, O_WRONLY | O_CREAT);

  D_PRINTXY("FILE CREATE ", result);

  // File must be pre-allocated to avoid huge
  // delays searching for free clusters.
  result = file.preAllocate(15000000); //setup 15Mb of space for this file
  D_PRINTXY("FILE PREALLOCATE ", result);
  }

void stopREC()
  {
  D_PRINTLN_F(D_BOLDGREEN, "STOPREC FUNCTION");

  //empty current buffer
  // buffer as uint32_t
  uint32_t nb = 4096 * 4;            //chunck size
  uint32_t buffer_size = 512 * nj;   //only the last bit of samples
  uint32_t nRdWr = buffer_size / nb; //chuncks to write
      //push to SDcard
#ifdef DUMMYDATA
  uint16_t c = 0;
  for (uint32_t n = 0; n < (buffer_size / 2); n++)
    {
    sample_buffer[n * 2] = c & 0xff;
    sample_buffer[n * 2 + 1] = (c >> 8);
    c++;
    }
#endif
  for (uint32_t n = 0; n < nRdWr; n++)
    {
    // Set start and end of buffer.
    file.write(&sample_buffer[n * nb], nb);
    }
  D_PRINTXY("endwrite", millis())
    nj = 0;

  //empty the recorder-buffer also if some data was still collected during the last write action
  while (recorder.available() > 0)
    {
    file.write((byte*)recorder.readBuffer(), 256);
    recorder.freeBuffer();
    }

  //now truncate the filesize since preallocate reserved 15Mb
  file.truncate();

  D_PRINTXY("SR WAVHEADER", SR[active_recording_SR].osc_frequency)

    char header[wavHeaderSize]; //create space for a header
                                //position the reader head to the beginning of the file

  file.seek(0);

  uint32_t fileSize = file.fileSize();                //size of the raw data file
  memcpy(header, wavHeader(fileSize), wavHeaderSize); //create header adjusted for size

  file.seek(0);
  file.write(header, wavHeaderSize); //overwrite existing data (original dataset looses 44 bytes !)
  //write powerspectrum bytes
  file.seek(44);
  float cumPowerspectrumMax = 0;
  for (uint8_t i = 0; i < 120; i++)
    {
    if (cumsumPowerspectrum[i] > cumPowerspectrumMax)
      {
      cumPowerspectrumMax = cumsumPowerspectrum[i];
      }
    }

  if (cumPowerspectrumMax != 0)
    {
    cumPowerspectrumMax = 1 / cumPowerspectrumMax;
    }
  byte pSpectrum[120];
  for (uint8_t i = 0; i < 120; i++)
    {
    pSpectrum[i] = byte(cumsumPowerspectrum[i] * cumPowerspectrumMax * 200.0);
    }
  file.write(pSpectrum, 120);
  file.seek(fileSize); //go to the end of the file

  //add extra info at the end of the file
  int HiP = 0; //set OFF

  HiP = HI_pass;
  if (HiP == HI_PASS_OFF)
    {
    HiP = 0;
    }

  //write 256  byte header with additional information
  char iheader[iheaderSize];
  memcpy(iheader, InfoChunk(), iheaderSize);
  file.write(iheader, iheaderSize);

  //write 512  byte header with additional GUANO information
  char gheader[gheaderSize];
  memcpy(gheader, GuanoChunk(), gheaderSize);
  file.write(gheader, gheaderSize);
  file.close();

  D_PRINTXY("AUDIOMAX fileclose", AudioMemoryUsageMax());
  D_PRINTXY("AUDIONOW", AudioMemoryUsage());

#ifdef DEBUG
  SD_SIZE = SD.totalSize();
  SD_USED = SD.usedSize();
  D_PRINTXY("CARD USED", SD_USED / (1024 * 1024));
  D_PRINTXY("CARD TOTAL", SD_SIZE / (1024 * 1024));
#endif


  }

//data storage on SD uses large buffers in PSRAM and writes 512byte chuncks
void writeREC()
  {
  const uint32_t N_BUFFER = 2;
  const uint32_t N_LOOPS = BUFF * N_BUFFER; // !!! NLOOPS and BUFFSIZE ARE DEPENDENT !!! NLOOPS = BUFFSIZE/N_BUFFER

  // if more or equal than N_BUFFER sampleblocks are available in the recorder write them to the sample_buffer
  while (recorder.available() >= int(N_BUFFER))
    { // one buffer = 256 (8bit)-bytes = block of 128 16-bit samples
      //read N_BUFFER sample-blocks into memory

    for (uint32_t i = 0; i < N_BUFFER; i++)
      { //copy a new bufferblock from the audiorecorder into memory
      memcpy(sample_buffer + i * 256 + nj * 256 * N_BUFFER, recorder.readBuffer(), 256);
      //free the last buffer that was read
      recorder.freeBuffer();
      }

  #ifdef AGC_MARKER // insert a marker to detect the moment AGC has changed
    if (AGC_change)
      {
      uint16_t v = uint16_t(128 * 256);
      sample_buffer[nj * 256 * N_BUFFER] = v & 0xff;
      sample_buffer[nj * 256 * N_BUFFER + 1] = v >> 8;
      AGC_change = false;
      }
  #endif

    nj++; //keep track of the no of sampleblocks written to the sample_buffer (steps are N_BUFFER)
    }

  //if the sample_buffer is full write it to SD and the teensy will keep collecting in the recorder (interrupts)
  if (nj > (N_LOOPS - 1))
    {                         // buffer as uint32_t
    uint32_t nb = 4096 * 4; //chunck size to write
    uint32_t buffer_size = N_BUFFER * 256 * N_LOOPS;
    uint32_t nRdWr = buffer_size / nb; //chuncks to write
    //dummydata can be used to test if the writes are good aligned (incremental values)
    uint8_t dummydata[AUDIO_BLOCK_SAMPLES * 2];

  #ifdef BUFFER_MARK //mark the start of the buffer with a special sampleblock
    for (uint32_t n = 0; n < 128; n++)
      {
      uint16_t v = uint16_t(n * 256);
      dummydata[n * 2] = v & 0xff;
      dummydata[n * 2 + 1] = (v >> 8);
      }
    file.write((byte*)dummydata, 256);
    //file.flush();
  #endif

  #ifdef DUMMYDATA
    uint16_t c = 0;
    for (uint32_t n = 0; n < (buffer_size / 2); n++)
      {
      sample_buffer[n * 2] = c & 0xff;
      sample_buffer[n * 2 + 1] = (c >> 8);
      c++;
      }
  #endif

    for (uint32_t n = 0; n < nRdWr; n++)
      {
      file.write(&sample_buffer[n * nb], nb);
      //file.flush();
      }

  #ifdef BUFFER_MARK //mark the end of the buffer with a special sampleblock
    for (uint32_t n = 0; n < 128; n++)
      {
      uint16_t v = uint16_t((128 - n) * 256);
      dummydata[n * 2] = v & 0xff;
      dummydata[n * 2 + 1] = (v >> 8);
      }
    file.write((byte*)dummydata, 256);
    file.flush();

  #endif
    //sample buffer now is emptied
    //now reset sample_buffer counter and add the data collected during file.write to the sample_buffer
    nj = 0;
    // OLDER ROUTINE before 20210724
    // while (recorder.available() >= int(N_BUFFER))
    //    {for (uint32_t i = 0; i < N_BUFFER; i++)
    //       { //copy a new bufferblock from the audiorecorder into memory
    //         memcpy(sample_buffer + i*256 + nj * 256 * N_BUFFER, recorder.readBuffer(), 256);
    //         //free the last buffer that was read
    //         recorder.freeBuffer();
    //       }
    //     nj++;
    //    }
    uint32_t i = 0;
    memset(dummydata, 0, sizeof dummydata); //create a zero dummydata block
    while (recorder.available())            //check if any sampleblocks are left and write them to the large sample_buffer
      {
      memcpy(sample_buffer + i * 256, recorder.readBuffer(), 256);
      //free the last buffer that was read
      recorder.freeBuffer();
      if (memcmp(dummydata, sample_buffer + i * 256, 256)) //skip complete zero blocks probably due to interrupts
        {
        i++;
        }
      }
    nj = i / 2;

    } //nj>LOOPS
  }

#ifdef USE_PSRAM
void flushRing(AudioBatRingQueue* ringBuffer)
  {
  //IF THE RINGBUFFER IS FULL, TAIL IS THE STARTINGPOSITION
  //IF THE RINGBUFFER IS NOT FULL TAIL SHOULD BE ZERO !
  // generalize this section, request X samples X is normally a full buffer but could be less
  // for autorecord.

  if (AUTO_REC) //autorec only uses a section of the prebuffer
    {
    uint32_t samples_arec = (uint32_t)SR_real / 128 * (AREC_PREBUFFER / 2);
    if (samples_arec > ring_buffer_samples)
      {
      samples_arec = ring_buffer_samples; //user wants complete buffer, default routine is OK
      }

    else                      //user wants partial buffer
      if (ringBuffer->full) //we have a full ringbuffer
        {
        if (ringBuffer->head > samples_arec) //the header is progressed far enough
          {
          ringBuffer->tail = ringBuffer->head - samples_arec;
          }
        else //the header is not far enough away
          {    //ex we want 200 samples from 1000 buffer and head is at 100
            //                1000              -   200     +     100 = 900 -> samples from 900-1000 -0-100
          ringBuffer->tail = ring_buffer_samples - samples_arec + ringBuffer->head;
          }
        }
      else //partial filled ringbuffer
        {
        if (ringBuffer->head > samples_arec) //enough samples available
          {
          ringBuffer->tail = ringBuffer->head - samples_arec;
          }
        else
          {
          ringBuffer->tail = 0;
          }
        }
    }

  D_PRINTXY("RINGBUFF counter", ringBuffer->counter);
  uint32_t t;
  t = ringBuffer->tail; //use the tailposition as a start to write samples

  if (!AUTO_REC)
    {
    if (ringBuffer->full == false) // the buffer was not fully used yet so start tail at 0
      {
      t = 0;
      D_PRINTXY("RINGBUFF NOT FULL ", ringBuffer->counter)
        ringBuffer->tail = 0;
      }
    }

  while (t != ringBuffer->head) //write data from tail to head
    {
    ringBuffer->flushblock = ringBuffer->ring[t];
    if (ringBuffer->flushblock != NULL) //if any slot is empty dont save !
      {
      file.write((byte*)ringBuffer->flushblock->data, 256);
      }
    t++;
    if (t >= ringBuffer->ring_buffers) //edge
      {
      t = 0;
      }
    }

  ringBuffer->flushed = true;

#ifdef PREBUFFER_MARK //mark the end of the prebuffer with a special sampleblock
  uint8_t dummydata[AUDIO_BLOCK_SAMPLES * 2];
  for (uint32_t n = 0; n < 128; n++)
    {
    uint16_t v = uint16_t(n * 256);
    dummydata[n * 2] = v & 0xff;
    dummydata[n * 2 + 1] = (v >> 8);
    }
  file.write((byte*)dummydata, 256);
#endif
  }

#endif

#endif