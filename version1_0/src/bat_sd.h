/* BatDetector on Teensy 3.6
 * Copyright (c) 2020, Cor Berrevoets
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
File root;
int filecounter=0;
int filemax=0;
uint autocounter; //count the number of recorded files in autorecording mode

char filename[80];
//char shortfilename[13];

#define FFORMAT_WAV 1
#define FFORMAT_RAW 0

//default RAW
uint8_t FFORMAT=FFORMAT_WAV;

#include "bat_vars.h"

const uint icomment_bytes=40;
const uint iheaderSize=44+icomment_bytes; //total size of infochunk in bytes

char * InfoChunk(const char* comment)
{  
  char tempstr[icomment_bytes];
  snprintf(tempstr,icomment_bytes,comment);

  static char iheader[iheaderSize]; 
  strcpy(iheader,"LIST");   //infochunk is a LIST
  
  *(int32_t*)(iheader+4)= 76;     //iheaderSize-8 4 bytes total length of upcoming info chunk so excluding 8 bytes from LIST en list_len        

  //when using infochunk this is the default starting position of the infodata     
  strcpy(iheader+8,"INFO");       //4 bytes start of info chunk   (chunk=4) //infoblock starts, size was set in the previous line

  //default position for 1st fourcc block
  strcpy(iheader+12,"IART");       //4 bytes artist name fourcc          8 //artist name, size of the textblock is on next line
  *(int32_t*)(iheader+16)=16;      //4 bytes mxlength of IART chunk     12
  strcpy(iheader+20,"TEENSYBAT v1.0");  //up to 16 bytes data for IART  28

  strcpy(iheader+36,"ICMT");       //4 bytes comment name fourcc        32
  *(int32_t*)(iheader+40)=icomment_bytes;      //4 bytes mxlength of ICMT chunk     36 
  strcpy(iheader+44,tempstr);      //40 bytesdata for ICMT    56 bytes total
  return iheader;
}

const uint wavHeaderSize=44;

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
  *(int32_t*)(wheader+4)=36+ iheaderSize + nsamp*nchan*nbytes; //filesize specifier is samples+header+infochunk

   return wheader;
}


#define MAX_FILES    999
#define MAX_FILE_LENGTH  13   // 8 chars plus 4 for.RAW plus NULL

char filelist[ MAX_FILES ][ MAX_FILE_LENGTH];
int fileselect=0;
int referencefile=0;

int initSD(void)
  {
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


void stamplog(char * s)
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

void countRAWfiles(void)
  {   
      delay(1000);
      filecounter=0;
      root = SD.open("/");
      int temp_filemax=0;   
      char tstr[30];  
      while (true) {
          File entry =  root.openNextFile();
                             
          if ((! entry) and (filecounter < MAX_FILES )) {
              break;
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
             int p=fname.indexOf("_");
             int num=fname.substring(1,p).toInt(); 
             if (num>temp_filemax)
                 {temp_filemax=num;}
                        
             //tft.println(entry.name());
             filecounter++;
            }
          }
          entry.close();
        }

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
      for (uint i = 0; i < N_BUFFER; i++) 
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
       f_write(&fil,header,wavHeaderSize,&wr); //overwrite existing data 
       f_lseek(&fil,fileSize); //go to the end of the file 

       //add extra info at the end of the file
       
       char tstr[icomment_bytes];
       int HiP=0; //set OFF
       #ifdef USE_HIPASS_FILTER
        HiP=HI_pass;
        if (HiP==HI_PASS_OFF)
          {  HiP=0;
          }
       #endif
       
       if (AUTO_REC)  
           snprintf(tstr,40,"Gain %02d HI_pass %02d AREC_F %02d AREC_S %02d",mic_gain, HiP, AREC_F, AREC_S);
       else
       {
         snprintf(tstr,40,"Gain %02d HI_pass %02d",mic_gain, HiP);
       
       }
           
        
       
       char iheader[iheaderSize];
       memcpy(iheader,InfoChunk(tstr),iheaderSize);
       f_write(&fil,iheader,iheaderSize,&wr);

       //file.close();
     }
       
    rc = f_close(&fil);
    
  #ifdef USE_LOGGER 
    isFileOpen=0;
     sprintf(filename, "B%u_%s.log", filemax, SR[rec_SR].txt);
     rc = f_stat( filename, 0);
     rc = f_open (&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
     #ifdef ADAPTED_SGTL_LIB
       unsigned int mic_ref=sgtl5000.read(0x0028); //MICREF
       unsigned int mic_ctrl=sgtl5000.read(0x002a); //MICCTRL
       unsigned int ana_adc_ctrl=sgtl5000.read(0x0020); //ANA_ADC_CTRL
       sprintf(filename, "Gain %02d M_REF %04X M_CTRL %04X ANA_ADC %04X", mic_gain, mic_ref, mic_ctrl, ana_adc_ctrl);
     #endif
     f_write(&fil, filename,80,&wr);
     rc = f_close(&fil);
   #endif
   
    countRAWfiles(); //update the counter !
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

    if (filecounter<MAX_FILES) // limit files to MAX_FILES
    if(!isFileOpen)
    {
      filemax++;

      if (filemax>MAX_FILES)
        {filemax=MAX_FILES;}

      //automated filename BA_S.raw where A=file_number and S shows samplerate. Has to fit 8 chars
      // so max is B999_192.raw
      if (FFORMAT==FFORMAT_RAW)
       {sprintf(filename, "B%03u_%s.raw", filemax, SR[rec_SR].txt);
       }
      else
      {
        sprintf(filename, "B%03u_%s.wav", filemax, SR[rec_SR].txt);
        #ifdef USE_HEX_FILENAMES
           sprintf(filename,"%lX.wav",RTC_TSR); //convert current time in seconds to a 8byte filename      
        #endif
      }

      stamplog(filename);

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