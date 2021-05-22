/* TEENSYBAT DETECTOR (for TEENSY 3.6) 

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


  #include "SD.h"
  
  File root;

#include "bat_vars.h"

uint16_t filecounter=0;
uint16_t dircounter=0;

uint16_t autocounter; //count the number of recorded files in autorecording mode
uint32_t file_duration=0;
char savedir[80];
boolean dir_create=false;
char filename[80];
char longfilename[80];
char dirname[80];
boolean root_active=false;
char active_dir[80];
char lastactive_dir[80];

boolean dir_choosen=false;
uint16_t playfolder=0;
uint16_t last_active_folder=0;

uint16_t active_folder=0;
uint32_t SD_USED=0;
uint32_t SD_SIZE=0;

char guano_filename[80];

boolean SDFATactive=false;

#include "bat_vars.h"

//create 1/2Kb infoblok 
//const unsigned int icomment_bytes=512-44;
const unsigned int iheaderSize=256; //total size of infochunk in bytes

// Subchunk Descriptor (4 bytes)
//     Subchunk Size (4 byte integer, little endian)
//     Subchunk Data (size is Subchunk Size)

//create 256byte info chunck for wave file 
char * InfoChunk()
{  
  static char iheader[iheaderSize]; 
  strcpy(iheader,"LIST");   //infochunk is a LIST
  *(int32_t*)(iheader+4)= iheaderSize-8;     //iheaderSize-8 4 bytes total length of upcoming info chunk so excluding 8 bytes from LIST en list_len        
  //when using infochunk this is the default starting position of the infodata     
  strcpy(iheader+8,"INFO");       //4 bytes start of info chunk   (chunk=4) //infoblock starts, size was set in the previous line
  //default position for 1st fourcc block
  char tstr[40]; //maximum stringlength
  unsigned int bufpos=12;
  unsigned int l=0;
  strcpy(iheader+bufpos,"IART");
  bufpos+=4;       //4 bytes artist name fourcc          8 //artist name, size of the textblock is on next line
  *(int32_t*)(iheader+bufpos)=16;      //4 bytes mxlength of IART chunk     12
  bufpos+=4;
  strcpy(iheader+bufpos,"TEENSYBAT ");  //up to 16 bytes data for IART  28
  bufpos+=16; //positie 36

  #ifdef USE_GPS //store GPSdata in TITLE FIELD fixed 40
    strcpy(iheader+bufpos,"INAM");
    bufpos+=4;       //4 bytes artist name fourcc          8 //artist name, size of the textblock is on next line
    //"Position: 53.212368 5.907277
    //gps_latitude=53212368;
    //gps_longitude=5907277;
    l=snprintf(tstr,40,"Position: %.6f %.6f ", float(gps_latitude)/1000000, float(gps_longitude)/1000000);
    *(int32_t*)(iheader+bufpos)=40;      //4 bytes mxlength of IART chunk     12
    bufpos+=4;
    strcpy(iheader+bufpos,tstr);  //up to 16 bytes data for IART  28
    bufpos+=40; //positie 36
  #endif

  strcpy(iheader+bufpos,"ICMT");       //4 bytes comment name fourcc        32
  bufpos+=4;
  *(int32_t*)(iheader+bufpos)=iheaderSize-bufpos-4;   //4 bytes mxlength of ICMT chunk     36 
  bufpos+=4;
  // start saving comment data into the infochunk
  int HiP=0; //set OFF
  
        HiP=HI_pass;
        if (HiP==HI_PASS_OFF)
          {  HiP=0;
          }
  
  l=snprintf(tstr,40,"Gain %02d ",mic_gain); 
  strcpy(iheader+bufpos,tstr);     
  bufpos+=l;
 
  l=snprintf(tstr,40,"|Hi_pass %02d ",HiP);
  strcpy(iheader+bufpos,tstr);     
  bufpos+=l;

  if (AUTO_REC)
   {
     l=snprintf(tstr,40,"|AREC_F %02d ",AREC_F);
     strcpy(iheader+bufpos,tstr);    
     bufpos+=l;
  
     l=snprintf(tstr,40,"|AREC_S %04d ",AREC_S*25);
     strcpy(iheader+bufpos,tstr);     
     bufpos+=l;
    }
    
 #ifdef USE_DS18B20
   l=snprintf(tstr,40,"|Temp %.1f ",bat_tempC); 
   strcpy(iheader+bufpos,tstr);     
   bufpos+=l;
 #endif
   
  l=snprintf(tstr,40," ");
  strcpy(iheader+bufpos,tstr);     
  bufpos+=l;
  return iheader;
}


const unsigned int gheaderSize=512;
// Guano chunk modelled on InfoChunk to append a Guano metadata section to the .wav file

//create 1/2Kb infoblok for GUANO data storage
char * GuanoChunk()
{
  static char gheader[gheaderSize];
  strcpy(gheader,"guan"); // guano chunk has type guan

  *(int32_t*)(gheader+4)=gheaderSize-8;

  char tstr[40]; //maximum stringlength
  unsigned int bufpos=8; //starting position of the chunk
  unsigned int l=0; //length of the last char-string
 
  l=snprintf(tstr,40,"GUANO|Version: 1.0\n");
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;

  l=snprintf(tstr,40,"Make: TeensyBat\n");
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  
  l=snprintf(tstr,40,"Firmware Version: %s\n",batversion);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;

  l=snprintf(tstr,40,"TB| Gain: %02d\n",mic_gain);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
    
  #ifdef USE_DS18B20
   l=snprintf(tstr,40,"Temperature Ext: %.1f\n",bat_tempC); 
   strcpy(gheader+bufpos,tstr);     
   bufpos+=l;
  #endif

  
  int HiP=0; //set OFF
  HiP=HI_pass;
  if (HiP==HI_PASS_OFF)
    {  HiP=0;
    }
  l=snprintf(tstr,40,"Filter HP: %02d\n",HiP);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  

  l=snprintf(tstr,40,"Samplerate: %06lu\n",SR[rec_SR].osc_frequency);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  
  l=snprintf(tstr,40,"Original Filename: %11s\n",guano_filename);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  
  l=snprintf(tstr,40,"Timestamp: %20s\n",tRecStart);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
    
  #ifdef USE_GPS
    l=snprintf(tstr,40,"Loc Position: %.6f %.6f \n", float(gps_latitude)/1000000, float(gps_longitude)/1000000);
    strcpy(gheader+bufpos,tstr);     
    bufpos+=l;
  
    l=snprintf(tstr,40,"Loc Accuracy: %.2f \n", float(gps_HDOP)/100);
    strcpy(gheader+bufpos,tstr);     
    bufpos+=l; 

  #endif  
  
  return gheader;
  
}

const uint8_t tbat_chunk=128; //v1 only 120 bytes for powerspectrum
const uint8_t tbat_data=tbat_chunk-8;

const uint8_t wavHeaderSize=44+tbat_chunk;

//create wavheader (44 bytes) and tbat_powerspectrum (120 bytes)
char * wavHeader(uint32_t fileSize)
{
  int fsamp = SR[rec_SR].osc_frequency;
  int nchan=1;

  int nbits=16;
  int nbytes=nbits/8;

  int nsamp=(fileSize-wavHeaderSize)/(nbytes*nchan); //we steal 44 bytes from the original recording 
  //
  static char wheader[48+tbat_chunk]; //
  //
  strcpy(wheader,"RIFF");
  *(int32_t*)(wheader+4)=36+tbat_chunk+ iheaderSize + gheaderSize+ nsamp*nchan*nbytes; //filesize specifier is samples+header+infochunk

  strcpy(wheader+8,"WAVE");
  strcpy(wheader+12,"fmt ");
  
  *(int32_t*)(wheader+16)= 16;// chunk_size
  *(int16_t*)(wheader+20)= 1; // PCM 
  *(int16_t*)(wheader+22)=1;// numChannels 
  *(int32_t*)(wheader+24)= fsamp; // sample rate 
  *(int32_t*)(wheader+28)= fsamp*nbytes; // byte rate
  *(int16_t*)(wheader+32)= nchan*nbytes; // block align
  *(int16_t*)(wheader+34)= nbits; // bits per sample 
  strcpy(wheader+36,"tbat");
  *(int32_t*)(wheader+40)= tbat_data;// tbat datachunk_size, real tbat_data starts at 44
  strcpy(wheader+44+tbat_data,"data");
  *(int32_t*)(wheader+48+tbat_data)=nsamp*nchan*nbytes; 
   return wheader;
}

#define MAX_FILES  2000
uint16_t fileindex[MAX_FILES];
uint16_t dirindex[MAX_FILES];

uint16_t fileselect=0;
int referencefile=0;

/************************************USDFS****************************************************/

#include "uSDFS.h"       // uSDFS lib  - do not uSDFS master !!
  
  FRESULT rc;        /* Result code */
  FATFS fatfs;      /* File system object */
  FIL fil;        /* File object */
 
  //uint32_t count=0;
  uint32_t ifn=0;
  uint32_t isFileOpen=0;

//  uint32_t t0=0;
//  uint32_t t1=0;
 
  
//switch to SDFAT operation (reading)
void switch2SDfat()
{  D_PRINTLN("SWITCH TO SDFAT")
  //switch off the fatfs mount
    SDFATactive=true;
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    f_mount (nullptr, Device, 1);      /* Unmount the logical drive */
    SD.begin(BUILTIN_SDCARD); //make ready for playing
    
}  

//switch to FATFS operation (writing)
void switch2fatfs()
{    D_PRINTLN("***SWITCH TO FATFS")
    SDFATactive=false;
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
    //Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
    SD_SIZE=(fatfs.n_fatent-2)*fatfs.csize*0.000512f;
    SD_USED=SD_SIZE- (fatfs.free_clst*fatfs.csize*0.000512f);
    rc = f_chdrive(Device);
}

//initialize the SDcard by first reading the FATFS cardsize etc and finally use SDFAT to open the card for reading
int initSD(void)
  { 
    D_PRINTLN("***SWITCH TO FATFS at INITSD")
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
    if (rc==0)
    //Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
    {SD_SIZE=(fatfs.n_fatent-2)*fatfs.csize*0.000512f;
     SD_USED=SD_SIZE- (fatfs.free_clst*fatfs.csize*0.000512f);
    }
    f_mount (nullptr, Device, 1);  
    D_PRINTLN("***SWITCH TO SDFAT at INITSD")
    return SD.begin(BUILTIN_SDCARD); 
  }

  
//get ready to play a wave-file using SDFAT
  void initPlay(void)
  { 
    //switch off the fatfs mount
    // const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    // f_mount (nullptr, Device, 1);      /* Unmount the logical drive */
    // SD.begin(BUILTIN_SDCARD); //make ready for playing
    switch2SDfat();
    
  }

  //initialize recording setup by switching to fatFS (usdfs setup)
  void initREC(void)
  { 
    D_PRINTLN_F(D_BOLDGREEN,"INITREC")
   
    switch2fatfs();
    // const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    // rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
    // //Serial.printf("File System %s\n", fileSystem[fatfs.fs_type]);
    // SD_SIZE=(fatfs.n_fatent-2)*fatfs.csize*0.000512f;
    // SD_USED=SD_SIZE- (fatfs.free_clst*fatfs.csize*0.000512f);
    
    // D_PRINTXY("USED",SD_USED)
    // D_PRINTXY("SIZE",SD_SIZE)
    //rc = f_chdrive(Device);
    
  }

/*********************************** SDFAT mode 3****************************************/

// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3 // USE multitype

// select the fastest SD card configuration (SDIO)
#define SD_CONFIG SdioConfig(FIFO_SDIO)

#if SD_FAT_TYPE == 0
SdFat sd;
FsFile dir;
FsFile file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 dir;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile dir;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
#endif  // SD_FAT_TYPE

/**********************************************************************************************/


//read the duration and samplerate from the wav-header, if stored also get the powerspectrum 
uint32_t getfileSR()
{     
      uint32_t fileSR=0;
      uint32_t filelen=0;
      D_PRINTLN("GETFILESR")
      snprintf(longfilename,80, "%s/%s",active_dir,filename);
      D_PRINTXY("longfile:",longfilename);
    
      File audioFile = SD.open(longfilename, FILE_READ);
      audioFile.seek(0);
      byte audiobuffer[128]; //large enough to read data
      audioFile.read(audiobuffer, 36); 
      filelen= audiobuffer[4]+audiobuffer[5]*256+audiobuffer[6]*256*256+audiobuffer[7]*256*256*256-36-tbat_chunk- iheaderSize - gheaderSize;
      #ifdef DEBUG_DETAIL
      D_PRINTXY("FILELEN",filelen);
      #endif

      fileSR= audiobuffer[24]+audiobuffer[25]*256+audiobuffer[26]*256*256;
      if (fileSR!=0)
        {file_duration=uint32_t(filelen*500.0f/fileSR); //time in ms (filelength=bytes, 2bytes=1sample )
        }
      else
        {file_duration=0;}  
      #ifdef DEBUG_DETAIL
      D_PRINTLN(audiobuffer[24])
      D_PRINTLN(audiobuffer[25])
      D_PRINTLN(audiobuffer[26])
      D_PRINTLN(audiobuffer[27])
      #endif

      D_PRINTXY("recordedSR ",fileSR);
      //test for spectrum


      //check if the file has a TBAT marking that is used for powerspectrum storage
      audioFile.read(audiobuffer, 4);

      #ifdef DEBUG_DETAIL
      D_PRINTLN((char)audiobuffer[0])
      D_PRINTLN((char)audiobuffer[1])
      D_PRINTLN((char)audiobuffer[2])
      D_PRINTLN((char)audiobuffer[3])
      #endif
    
      if  (memcmp (audiobuffer, "tbat", 4)==0)
      { D_PRINTLN("SPECTRUM FOUND");
        wavPSpectrum=true;
        audioFile.read(audiobuffer, 4); //read size indicator
        audioFile.read(audiobuffer, 120);
        for (uint8_t i = 0; i < 120; i++)
          { wavPowerSpectrum[i]=audiobuffer[i];
          } 
      }
      else
      {
        for (uint8_t i = 0; i < 120; i++)
          { wavPowerSpectrum[i]=0;
          } 
      }
        
      audioFile.close();
      

      return fileSR ;
}

//only count the no of files in a directory
uint16_t FilesinDirIndex(uint16_t idx)
 { uint16_t wavfiles=0;
   dir.open("/");
   file.open(&dir,idx, O_RDONLY);
   file.getName(dirname,80);
   file.close();
   dir.close();
   D_PRINTXY("DIR: ",dirname);
   dir.open(dirname);
   while (file.openNext(&dir, O_RDONLY)) {
       {
        file.getName(filename,80);
        if ((String(filename).endsWith(".WAV")) or (String(filename).endsWith(".wav")))
          { wavfiles++;
          } 
        file.close();
       }
   }   

  file.close();      
  dir.close();
  
  return wavfiles;

 }

// count and store the index of wav-files in a directory (idx=directory_index)  
void countFilesinDir_byindex(uint16_t idx)
{  
  D_PRINTLN("countFilesinDir_byindex");
  D_PRINTXY("idx",idx);
  
  dir.open("/");
  file.open(&dir,idx, O_RDONLY);
  file.getName(dirname,80);
  file.close();
  dir.close();
  D_PRINTXY("DIR: ",dirname);

  dir.open(dirname);
  dir.rewind();

  filecounter=0;
  boolean filesfound=false;
  while (file.openNext(&dir, O_RDONLY)) {
       {
        file.getName(filename,80);
        if ((String(filename).endsWith(".WAV")) or (String(filename).endsWith(".wav")))
          { 
            fileindex[filecounter]=uint32_t(dir.curPosition()/32)-1; //records the fileposition
            #ifdef DEBUG_DETAIL
            D_PRINTXY(filename,fileindex[filecounter]);
            #endif
            filesfound=true;
            if (filecounter<MAX_FILES-1)
              {filecounter++;
              }
          } 
        file.close(); 
       }

   }   

  file.close();      
  dir.close();
  if (filesfound)
     {D_PRINTXY("COUNTED_FILES",filecounter);
     }
  else
    {
      D_PRINTLN("NO_FILES");
    }   
  
}

/// get the filename (and samplerate) or directoryname by its index   
void readFileInfo_byindex(uint16_t idx)
{  
   D_PRINT("FILE READbyIndex:");  
   D_PRINTXY("FILE idx:",idx);
   
   dir.close();
   dir.open(active_dir); 
   D_PRINTXY("activedir:",active_dir);
   file.open(&dir, idx, O_RDONLY);
   file.getName(filename,80);
   D_PRINTXY("name", filename);
   dir.close();
   
   if (!root_active) // in root do not try to read samplerate as we are looking at dirs
     {recorded_SR=getfileSR();
     } 

}

//check for dirs in the root of the SDcard and store the index to the directory (to allow fast readouts)
void countDirs_inroot(void)
  { 
    switch2SDfat();
    file.close();
    dir.close();

    dir.open("/");
    
    D_PRINTLN("countDirs_inroot")
    dir.rewind();
    dircounter=0;
    boolean dirfound=false; 
    while (file.openNext(&dir, O_RDONLY)) 
       {
        file.getName(dirname,80);
        
        if (file.isDir()) {
          #ifdef DEBUG_DETAIL
          D_PRINTXY("found ", dirname);
          #endif
          dirindex[dircounter]=uint32_t(dir.curPosition()/32)-1;
          #ifdef DEBUG_DETAIL
          D_PRINTXY("DIR IDX",dirindex[dircounter]);
          #endif
          dirfound=true;
          if (dircounter<MAX_FILES-1)
            {  dircounter++;
             }
           }
        file.close();
       }
    file.close();    
    dir.close();
    
    if (dirfound)    
      {D_PRINTXY("COUNTED_DIRS",dircounter);
      }
    else
      {
        D_PRINTLN("NO DIRS");
      }     
           
        
  }


//main data storage on SD routine uses large buffers and writes 512byte chuncks
void writeREC(AudioRecordQueue *recorder)
  {  
    const uint32_t N_BUFFER = 2;
    const uint32_t N_LOOPS = BUFF*N_BUFFER; // !!! NLOOPS and BUFFSIZE ARE DEPENDENT !!! NLOOPS = BUFFSIZE/N_BUFFER

    // buffer size total = 256 * n_buffer * n_loops
    // queue: write n_buffer blocks * 256 bytes to buffer at a time; free queue buffer;
    // repeat n_loops times ( * n_buffer * 256 = total amount to write at one time)
    // then write to SD card

    if (recorder->available() >= int(N_BUFFER) )
    {// one buffer = 256 (8bit)-bytes = block of 128 16-bit samples
      //read N_BUFFER sample-blocks into memory
      for (uint32_t i = 0; i < N_BUFFER; i++) 
      {
        //copy a new bufferblock from the audiorecorder into memory
        memcpy(sample_buffer + i*256 + nj * 256 * N_BUFFER, recorder->readBuffer(), 256);
        //free the last buffer that was read
        recorder->freeBuffer();
      }
      nj++;

      //write only full buffers to the SD to limit the write moments
      if (nj >  (N_LOOPS-1))
      {
        nj = 0;
        //push to SDcard
        rc =  f_write (&fil, sample_buffer, N_BUFFER * 256 * N_LOOPS, &wr);
      }
    }


  }
  
//stop the recording and finish writing data to SD  
void stopREC(AudioRecordQueue *recorder)
  {
   D_PRINTLN_F(D_BOLDGREEN,"STOPREC FUNCTION")
      
    recorder->end();
    recorderActive=false;

    while (recorder->available() > 0) 
    {
      rc = f_write (&fil, (byte*)recorder->readBuffer(), 256, &wr);
  //      frec.write((byte*)recorder->readBuffer(), 256);
        recorder->freeBuffer();
    }
    //close file
    
       char header[wavHeaderSize]; //create space for a header
       //position the reader head to the beginning of the file

       f_lseek(&fil,0);
       //f_read(&fil,header,wavHeaderSize,&wr); //read current data into the header

       uint32_t fileSize = f_size(&fil); //size of the raw data file
       memcpy(header,wavHeader(fileSize),wavHeaderSize); //create header adjusted for size 
       f_lseek(&fil,0);
       f_write(&fil,header,wavHeaderSize,&wr); //overwrite existing data (original dataset looses 44 bytes !)
       //write powerspectrum bytes
       f_lseek(&fil,44);//move to position 44 to save 120 bytes of powerspectrum information
       float cumPowerspectrumMax=0;
       for (uint8_t i = 0; i < 120; i++)
         { if (cumsumPowerspectrum[i]>cumPowerspectrumMax)
          {cumPowerspectrumMax=cumsumPowerspectrum[i];
          }
         } 

       if (cumPowerspectrumMax!=0)
        {cumPowerspectrumMax=1/cumPowerspectrumMax;
         }
       //char indicator[9]="spectrum";
       //f_write(&fil,indicator,8,&wr);
       byte pSpectrum[120];  
       for (uint8_t i = 0; i < 120; i++)
         { pSpectrum[i]= byte(cumsumPowerspectrum[i]*cumPowerspectrumMax*200.0);
           //D_PRINTXY("sp",cumsumPowerspectrum[i]);
           //D_PRINTXY("sp",pSpectrum[i]);
          } 
       f_write(&fil,pSpectrum,120,&wr);
       f_lseek(&fil,fileSize); //go to the end of the file 

       //add extra info at the end of the file
       
       int HiP=0; //set OFF
       
        HiP=HI_pass;
        if (HiP==HI_PASS_OFF)
          {  HiP=0;
          }
       
       //write 256  byte header with additional information
       
       char iheader[iheaderSize];
       memcpy(iheader,InfoChunk(),iheaderSize);
       f_write(&fil,iheader,iheaderSize,&wr);
       
       //write 512  byte header with additional GUANO information
       char gheader[gheaderSize];
       memcpy(gheader,GuanoChunk(),gheaderSize);
       f_write(&fil,gheader,gheaderSize,&wr);

     
       
    rc = f_close(&fil);

  
    
    
  }


//start recording to the SDcard using USDFS
void startREC(void)
  { 
     
    initREC();
  
    D_PRINTLN_F(D_BOLDGREEN,"STARTREC")
    // close file
    if (isFileOpen)
    { //close file
      rc = f_close(&fil);
      isFileOpen=0;
    }

    if(!isFileOpen)
    {
      //automated filename BA_S.raw where A=file_number and S shows samplerate. Has to fit 8 chars
      // so max is B999_192.raw
      seconds2time(getRTC_TSR());
      
      snprintf(savedir,80,"%04d%02d%02d",tm_year+1970,tm_mon,tm_mday);
      snprintf(filename,80,"%04d%02d%02dT%02d%02d%02d.wav",tm_year+1970,tm_mon,tm_mday,tm_hour,tm_min,tm_sec);
      snprintf(guano_filename,80,filename);

      
      snprintf(tRecStart,20,"%04d-%02d-%02dT%02d:%02d:%02d",tm_year+1970,tm_mon,tm_mday,tm_hour,tm_min,tm_sec);
      D_PRINTXY("SAVEDIR",savedir);
      rc=f_mkdir(savedir); 
      //dir_create=false; 
      //fail save method. 1st create directory.
      //If creating works or directory exists expand filename with directoryname
      //otherwise save to root

      if (rc==FR_OK)
        {D_PRINTLN("F_MKDIR TRUE")
         dir_create=true; // a new dir was created
         snprintf(filename,80,"%s/%s",savedir,guano_filename); //change filename to filename and dir
        }
      else
        {if (rc==FR_EXIST)
             { D_PRINTXY("F_MKDIR EXISTS rc:",rc);
               snprintf(filename,80,"%s/%s",savedir,guano_filename); //change filename to filename and dir
              }
         else
          {D_PRINTXY("F_MKDIR FALSE rc:",rc);
           }     
        }
      
      rc = f_stat( filename, 0);
      rc = f_open (&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
      // check if file has errors
      if(rc == FR_INT_ERR)
      { // only option then is to close file
          
          rc = f_close(&fil);
          if(rc == FR_INVALID_OBJECT)
          {
            rc = f_unlink(filename);
            
          }
          D_PRINTXY(" ERROR:",filename)
        
      }
      // retry open file
      D_PRINTXY(" file:",filename)
      //rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
     
      isFileOpen=1;
    }
    
  }




#endif