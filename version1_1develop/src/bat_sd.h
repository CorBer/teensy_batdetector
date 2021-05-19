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

//#define USE_STAMPLOG
#include <SD.h>
// #ifdef DEBUG
//  Sd2Card card;
//  SdVolume volumeSD;
// #endif
File root;
int filecounter=0;
uint16_t filemax=0;
uint16_t autocounter; //count the number of recorded files in autorecording mode

char filename[80];
//char shortfilename[13];

#define FFORMAT_WAV 1
#define FFORMAT_RAW 0

//default RAW
uint8_t FFORMAT=FFORMAT_WAV;

#include "bat_vars.h"

//create 1/2Kb infoblok 
//const unsigned int icomment_bytes=512-44;
const unsigned int iheaderSize=256; //total size of infochunk in bytes

// Subchunk Descriptor (4 bytes)
//     Subchunk Size (4 byte integer, little endian)
//     Subchunk Data (size is Subchunk Size)

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
  #ifdef USE_HIPASS_FILTER
        HiP=HI_pass;
        if (HiP==HI_PASS_OFF)
          {  HiP=0;
          }
  #endif
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
  
     l=snprintf(tstr,40,"|AREC_S %02d ",AREC_S);
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

//create 1/2Kb infoblok guan
const unsigned int gheaderSize=512;
// Guano chunk modelled on InfoChunk to append a Guano metadata section to the .wav file
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

  #ifdef USE_HIPASS_FILTER
  int HiP=0; //set OFF
  HiP=HI_pass;
  if (HiP==HI_PASS_OFF)
    {  HiP=0;
    }
  l=snprintf(tstr,40,"Filter HP: %02d\n",HiP);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  #endif

  l=snprintf(tstr,40,"Samplerate: %06d\n",SR[rec_SR].osc_frequency);
  strcpy(gheader+bufpos,tstr);     
  bufpos+=l;
  
  l=snprintf(tstr,40,"Original Filename: %11s\n",filename);
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


const uint8_t wavHeaderSize=44;

char * wavHeader(uint32_t fileSize)
{
  int fsamp = SR[rec_SR].osc_frequency;
  int nchan=1;

  int nbits=16;
  int nbytes=nbits/8;

  int nsamp=(fileSize-44)/(nbytes*nchan); //we steal 44 bytes from the original recording 
  //
  static char wheader[48]; //
  //
  strcpy(wheader,"RIFF");
  strcpy(wheader+8,"WAVE");
  strcpy(wheader+12,"fmt ");
  strcpy(wheader+36,"data");
  *(int32_t*)(wheader+16)= 16;// chunk_size
  *(int16_t*)(wheader+20)= 1; // PCM 
  *(int16_t*)(wheader+22)=nchan;// numChannels 
  *(int32_t*)(wheader+24)= fsamp; // sample rate 
  *(int32_t*)(wheader+28)= fsamp*nbytes; // byte rate
  *(int16_t*)(wheader+32)=nchan*nbytes; // block align
  *(int16_t*)(wheader+34)=nbits; // bits per sample 
  *(int32_t*)(wheader+40)=nsamp*nchan*nbytes; 
  *(int32_t*)(wheader+4)=36+ iheaderSize + gheaderSize+ nsamp*nchan*nbytes; //filesize specifier is samples+header+infochunk

   return wheader;
}


#define MAX_FILES  500
#define MAX_FILE_LENGTH  13   // 8 chars plus 4 for.RAW plus NULL

char filelist[ MAX_FILES ][ MAX_FILE_LENGTH];
int fileselect=0;
int referencefile=0;

int initSD(void)
  {
    // #ifdef DEBUG
    //  boolean status = card.init(SPI_FULL_SPEED, BUILTIN_SDCARD);
    //  if (status) {
    //   Serial.println("SD card is connected :-)");
    //   } else {
    //     Serial.println("SD card is not connected or unusable :-(");
    //   return;
    //    }
    // int type = card.type();
    // if (type == SD_CARD_TYPE_SD1 || type == SD_CARD_TYPE_SD2) {
    //   Serial.println("Card type is SD");
    // } else if (type == SD_CARD_TYPE_SDHC) {
    //   Serial.println("Card type is SDHC");
    // } else {
    //   Serial.println("Card is an unknown type (maybe SDXC?)");
    // }
    // Serial.flush();
    // status = volumeSD.init(card);
    // if (!status) {
    //   Serial.println("Unable to access the filesystem on this card. :-(");
    //   return;
    // }
    // else
    //   { Serial.println("filesystem found");
    
    //   }
    // Serial.flush();
    // float size = volumeSD.blocksPerCluster() * volumeSD.clusterCount();
    // size = size * (512.0 / 1e6); // convert blocks to millions of bytes
    // Serial.print("File system space is ");
    // Serial.print(size);
    // Serial.println(" Mbytes.");
    // Serial.flush();
    // #endif
     return SD.begin(BUILTIN_SDCARD); 
  }


#include "uSDFS.h"       // uSDFS lib  - do not uSDFS master !!
  
  FRESULT rc;        /* Result code */
  FATFS fatfs;      /* File system object */
  FIL fil;        /* File object */
 
  //uint32_t count=0;
  uint32_t ifn=0;
  uint32_t isFileOpen=0;

//  uint32_t t0=0;
//  uint32_t t1=0;
  // ************** //

  void initPlay(void)
  { 
    //switch to the player using device 0 
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    f_mount (nullptr, Device, 1);      /* Unmount a logical drive */
    SD.begin(BUILTIN_SDCARD); //make ready for playing
    
  }

  void initREC(void)
  { 
    D_PRINT_FORMATLN(D_BOLDGREEN,"INITREC")
    const char *Device = "1:/";  // 0: SPI 1;SDIO 2:MSC
    rc = f_mount (&fatfs, Device, 1);      /* Mount a logical drive */
    rc = f_chdrive(Device);
    
  }


void stamplog(const char * s)
{   
  #ifdef USE_STAMPLOG
    char tstr[50]; 
    struct tm tx = seconds2time(RTC_TSR); //seconds since 1 jan 1970

    snprintf(tstr,50, "%02d%02d_%02d:%02d:%02d %s", tx.tm_mon, tx.tm_mday, tx.tm_hour, tx.tm_min, tx.tm_sec, s);
    isFileOpen=0;
    rc = f_open (&fil,(char const *)"TIME_LOG.log", FA_WRITE | FA_OPEN_APPEND);

    f_printf(&fil, tstr);
    rc = f_close(&fil);
   #endif   
}

void count_SDfiles(void)
  {   
      filecounter=0;
      root = SD.open("/");
      int temp_filemax=0;   
      char tstr[30];  
      D_PRINTLN("COUNT_SDFILES")
      while (true) {
          File entry =  root.openNextFile();
                             
          if ((!entry)) //we have space for at most 500 filenames 
          {   D_PRINTXY("NO FURTHER FILES, FILECOUNTER:",filecounter)
              break;
              
          }
          if (filecounter>=MAX_FILES)
           {   D_PRINTXY("MAX_FILES, FILECOUNTER:",filecounter)
              break;
              
          }

          if (filecounter%50==0)
            {
               D_PRINTXY("FILECOUNTER:",filecounter)
            }

          if (entry.isDirectory()) {
            // do nothing, only look for raw files in the root
          }
          else   {
          String fname=entry.name();

          if ((fname.indexOf(".RAW")>=0) or (fname.indexOf(".WAV")>=0) )
            {strcpy(filelist[filecounter],entry.name() );
              //
             //construct filenumber from filename to avoid overwriting
             #ifndef USE_HEX_FILENAMES
               int p=fname.indexOf("_");
               int num=fname.substring(1,p).toInt(); 
               if (num>temp_filemax)
                   {temp_filemax=num;}
             #endif           
             //tft.println(entry.name());
             filecounter++;
            }
          }
          entry.close();
        }
    
    D_PRINTXY("COUNTED_FILES",filecounter)
      
    if (temp_filemax>=filemax) 
       { filemax=temp_filemax;
         if (filemax>MAX_FILES)
            {
              filemax=MAX_FILES;
              stamplog("MAX FILES");      
            }
       }
    else
    {   snprintf(tstr,30,"ERR count %3d filemx %3d",temp_filemax, filemax);
        stamplog(tstr);
    }
          
    D_PRINTXY("  RAW/WAV files:",filecounter)    
            
        
  }

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
      if (nj >  (N_LOOPS-1))
      {
        nj = 0;
        //push to SDcard
        rc =  f_write (&fil, sample_buffer, N_BUFFER * 256 * N_LOOPS, &wr);
      }
    }
  }
  
void stopREC(AudioRecordQueue *recorder)
  {
   D_PRINT_FORMATLN(D_BOLDGREEN,"STOPREC FUNCTION")
    
   #ifdef USE_SD  
    recorder->end();
    recorderActive=false;

    while (recorder->available() > 0) 
    {
      rc = f_write (&fil, (byte*)recorder->readBuffer(), 256, &wr);
  //      frec.write((byte*)recorder->readBuffer(), 256);
        recorder->freeBuffer();
    }
    //close file
    if (FFORMAT==FFORMAT_WAV)
     {
       char header[wavHeaderSize]; //create space for a header
       //position the reader head to the beginning of the file

       f_lseek(&fil,0);
       //f_read(&fil,header,wavHeaderSize,&wr); //read current data into the header

       uint32_t fileSize = f_size(&fil); //size of the raw data file
       memcpy(header,wavHeader(fileSize),wavHeaderSize); //create header adjusted for size 
       f_lseek(&fil,0);
       f_write(&fil,header,wavHeaderSize,&wr); //overwrite existing data (original dataset looses 44 bytes !)
       f_lseek(&fil,fileSize); //go to the end of the file 

       //add extra info at the end of the file
       
       int HiP=0; //set OFF
       #ifdef USE_HIPASS_FILTER
        HiP=HI_pass;
        if (HiP==HI_PASS_OFF)
          {  HiP=0;
          }
       #endif
       
       
      //  char tstr[icomment_bytes];
      //  if (AUTO_REC)                   
      //      snprintf(tstr,icomment_bytes,"Gain %02d HI_pass %02d AREC_F %02d AREC_S %02d",mic_gain, HiP, AREC_F, AREC_S);
      //  else
      //  {                                
      //    snprintf(tstr,icomment_bytes,"Gain %02d HI_pass %02d",mic_gain, HiP);
      //  }
       
       char iheader[iheaderSize];
       memcpy(iheader,InfoChunk(),iheaderSize);
       f_write(&fil,iheader,iheaderSize,&wr);
       
      
       char gheader[gheaderSize];
       memcpy(gheader,GuanoChunk(),gheaderSize);
       f_write(&fil,gheader,gheaderSize,&wr);

       //file.close();
     }
       
    rc = f_close(&fil);
    
  #ifndef USE_HEX_FILENAMES
    count_SDfiles(); //update the counter !
  #endif

  #endif    
    stamplog(filename);
    stamplog("ended \r\n");
    
  }



void startREC(void)
  { 
    stamplog("started ");

    D_PRINT_FORMATLN(D_BOLDGREEN,"STARTREC")
    // close file
    if(isFileOpen)
    { //close file
      rc = f_close(&fil);
     
      isFileOpen=0;
    }

   #ifndef USE_HEX_FILENAMES // in old serial_numbers mode limit to MAX_FILES
    if (filecounter<MAX_FILES) // limit files to MAX_FILES
   #endif   
    if(!isFileOpen)
    {
      filemax++;
      if (filemax>MAX_FILES)
        {filemax=MAX_FILES;} //prevent overrun for old_file modus

      //automated filename BA_S.raw where A=file_number and S shows samplerate. Has to fit 8 chars
      // so max is B999_192.raw
      if (FFORMAT==FFORMAT_RAW)
       {
         #ifdef USE_HEX_FILENAMES
           sprintf(filename,"%lX.raw",RTC_TSR); //convert current time in seconds to a 8byte filename      
         #else
         sprintf(filename, "B%03u_%s.raw", filemax, SR[rec_SR].txt);
         #endif
       }
      else
      {
        #ifdef USE_HEX_FILENAMES
           sprintf(filename,"%lX.wav",RTC_TSR); //convert current time in seconds to a 8byte filename      
        #else
         sprintf(filename, "B%03u_%s.wav", filemax, SR[rec_SR].txt);
        #endif
      }

      stamplog(filename);
      
      struct tm tx=seconds2time(RTC_TSR);
      snprintf(tRecStart,20,"%04d-%02d-%02dT%02d:%02d:%02d",tx.tm_year+1970,tx.tm_mon,tx.tm_mday,tx.tm_hour,tx.tm_min,tx.tm_sec);

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
          stamplog(filename);
          stamplog(" ERROR \r\n");
          
      }
      // retry open file
      D_PRINTXY(" file:",filename)
      rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
     
      isFileOpen=1;
    }
  }


#endif