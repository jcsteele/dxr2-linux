/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     October 20, 1999     Andrew de Quincey    Rewrote and extended
  *                          Lucien Murray-Pitts  original incomplete 
  *                                               driver.
  *
  *     April 18, 1999       Andrew Veliath       Original Driver
  *                                               implementation
  *
  **********************************************************************
  *
  *     This program is free software; you can redistribute it and/or
  *     modify it under the terms of the GNU General Public License as
  *     published by the Free Software Foundation; either version 2 of
  *     the License, or (at your option) any later version.
  *
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *
  *     You should have received a copy of the GNU General Public
  *     License along with this program; if not, write to the Free
  *     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
  *     USA.
  *
  **********************************************************************
  */



/*
 **************************************************************************
 **************************************************************************
 * And now, a quick note about CSS and this software:                     *
 * 									  *
 **************************************************************************
 * Although this program permits playing of encrypted DVDs, 		  *
 * there are no "CSS secrets" contained in it, and none were used in 	  *
 * it's construction. No decrypted keys/data are handled by this program  *
 * AT ANY TIME. The DXR2 card does all the CSS decryption on-board, 	  *
 * in hardware. All this program does is permit the exchange of 	  *
 * *encrypted* keys and *encrypted* data between the drive and 		  *
 * DXR2 card's hardware.						  *
 **************************************************************************
 *									  *
 * The authors are NOT connected, in any way, with any of the 		  *
 * "software CSS decryption" programs out there, and *DO NOT* endorse 	  *
 * their use in any way. Piracy is a crime!				  *
 *									  *
 * Just wanted to make that ABSOLUTELY CLEAR. 				  *
 * 									  *
 **************************************************************************
 **************************************************************************
 */




/**
 *
 * test program for playing VCDs/DVDs etc
 *
 */


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/types.h>
#include <dxr2ioctl.h>
#include <fcntl.h>
#include <curses.h>
#include <unistd.h>
#include <linux/limits.h>
#include "css.h"
#include <math.h>


int main(int argc, char* argv[]) 
{
  int t;
  int dxr2FD;
  int driveFD = -1;
  int mpegFD;
  int uCodeFD;
  int uCodeSize;
  int lba;
  dxr2_oneArg_t buf1;
  dxr2_threeArg_t buf3;
  dxr2_sixArg_t buf6;
  dxr2_nineArg_t buf9;
  dxr2_uCode_t* uCode;
  dxr2_vgaParams_t vgaBuf;
  int size;
  int i;
  int current_opt;
  char optionsString[] = "f:d:a:r:t:";
  char file_name[PATH_MAX];
  char dvd_device[PATH_MAX];
  int audio_stream = 0;
  int ratio = 0;
  int output_type = 0;
  char buffer[0x20000];
  dvd_authinfo authBuf;
  int LBA;
  int pid;
  int xRes;
  int yRes;
  int xpos;

  file_name[0]=dvd_device[0]='\0';
  
  while((current_opt = getopt(argc,argv,optionsString))!=-1) {

    if(current_opt == 'a')
      sscanf(optarg,"%d",&audio_stream);
    else if (current_opt=='r')
      sscanf(optarg,"%d",&ratio);
    else if (current_opt=='t')
      sscanf(optarg,"%d",&output_type);
    else if (current_opt=='f')
      sscanf(optarg,"%s",&file_name);
    else if(current_opt =='d')
      sscanf(optarg,"%s",&dvd_device);
  }
  
  
  if (file_name[0]==0) {

    printf("Syntax: %s -f <filename> [-d </path/to/dvd/device>] [-a <audio stream>] [-r <output ratio>] [-t <output type>]\n",argv[0]);
    printf("\n");
    printf("N.B. To use the card's onboard hardware CSS decryption,\n");
    printf("simply specify your DVD drive DEVICE (e.g. /dev/hdc) with the -d argument.\n");
    printf("if you don't specify this, CSS will NOT be used\n");
    printf("\n");
    printf("-a will select the audio stream to decode (from 0 to 7)\n");
    printf("-r is either 0 1 or 2, 0 being letterbox, 1 being 'normal' and 2 being pan/scan\n");
    printf("-t is either 0 1 2 3 4 5 6 or 7\n");
    exit(1);
  }
  
  // open the mpeg
  if (file_name[0] == '-') {
    
    fprintf(stderr,"using stdin for mpeg file\n");
    mpegFD= STDIN_FILENO;
  }
  else if ((mpegFD = open(file_name, O_RDONLY)) < 0) {
    
    fprintf(stderr, "ERROR: Could not open mpeg (%s): %s\n", file_name, strerror(errno));
    exit(1);
  }

  // open DVD device
  if ((dxr2FD = open("/dev/dxr2", O_WRONLY)) < 0) {
    
    fprintf(stderr, "ERROR: Cannot open DVD device (%s): %s\n", "/dev/dxr2", strerror(errno));
    exit(1);
  }  

  //LUCIEN
  buf6.arg1 = 0;
  buf6.arg2 = 0;
  buf6.arg3 = 30;
  buf6.arg4 = 0;
  printf("set overlay cropping status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_CROPPING, &buf6));

  // open drive
  if (dvd_device[0] != '\0') {

    // open the drive
    if ((driveFD = open(dvd_device, O_RDONLY)) < 0) {
      
      fprintf(stderr, "ERROR: Cannot open DVD drive (%s): %s\n", dvd_device, strerror(errno));
      exit(1);
    }  
  }

  // Read ucode
  if ((uCodeFD = open("/usr/src/dvd12.ux", O_RDONLY)) < 0) {
    
    fprintf(stderr, "ERROR: Could not open uCode (%s): %s\n", "dvd1.ux", strerror(errno));
    exit(1);
  }
  uCodeSize = lseek(uCodeFD, 0, SEEK_END);
  if ((uCode = malloc(uCodeSize + 4)) == NULL) {
    
    fprintf(stderr, "ERROR: Could not allocate memory for uCode: %s\n", strerror(errno));
    exit(1);
  }
  lseek(uCodeFD, 0, SEEK_SET);
  if (read(uCodeFD, uCode+4, uCodeSize) != uCodeSize) {
    
    fprintf(stderr, "ERROR: Could not read uCode uCode: %s\n", strerror(errno));
    exit(1);
  }
  close(uCodeFD);
  uCode->uCodeLength = uCodeSize;

  // upload ucode
  if (ioctl(dxr2FD, DXR2_IOC_INIT_ZIVADS, uCode)) {
    
    fprintf(stderr, "uCode upload failed!\n");
    exit(1);
  }


  // reset the ziva
  printf("reset status = %i\n", ioctl(dxr2FD, DXR2_IOC_RESET));

  // only if CSS was activated
  if (driveFD > 0) {
    
    // get the LBA
    css_get_lba(file_name, &LBA);

    // OK, do CSS authentication
    css_do_disc_key(driveFD, dxr2FD, &authBuf);
    css_do_title_key(driveFD, dxr2FD, &authBuf, LBA);
  }



  // CHANGEME
  // BITSTREAM TYPE
  // can be DXR2_BITSTREAM_TYPE_MPEG_VCD for MPEG1 stream (i.e. video CDs), or DXR2_BITSTREAM_TYPE_MPEG_VOB for
  // (unencrypted) DVD .VOB files
  //  buf3.arg1 = DXR2_BITSTREAM_TYPE_MPEG_VCD;
  buf3.arg1 = DXR2_BITSTREAM_TYPE_MPEG_VOB;
  printf("bitstream type status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_BITSTREAM_TYPE, &buf3));

  
  // CHANGEME
  // TV mode. can be DXR2_OUTPUTFORMAT_PAL, DXR2_OUTPUTFORMAT_NTSC
  buf3.arg1 = output_type;
  printf("output tv format status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_TV_OUTPUT_FORMAT, &buf3));


  // interlaced mode on
  buf3.arg1 = DXR2_INTERLACED_ON;
  printf("output tv format status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_TV_INTERLACED_MODE, &buf3));


  // source video frequency
  // set to DXR2_SRC_VIDEO_FREQ_30 for NTSC mpegs, or DXR2_SRC_VIDEO_FREQ_25 for PAL ones
  buf3.arg1 = DXR2_SRC_VIDEO_FREQ_25;
  buf3.arg2 = 0x2d0;
  buf3.arg3 = 0x1e0;
  printf("source video format status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_SOURCE_VIDEO_FORMAT, &buf3));


  // output aspect ratio 4:3
  buf3.arg1 = DXR2_ASPECTRATIO_4_3;
  printf("output aspect ratio status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OUTPUT_ASPECT_RATIO, &buf3));


  // source aspect ratio 4:3
  buf3.arg1 = DXR2_ASPECTRATIO_4_3;
  printf("source aspect ratio status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_SOURCE_ASPECT_RATIO, &buf3));


  // use normal scaling mode (i.e. NOT letterboxed or pan/scanned
  if (ratio == 0) 
    buf3.arg1 = DXR2_ASPECTRATIOMODE_LETTERBOX;
  else if (ratio == 2)
    buf3.arg1 = DXR2_ASPECTRATIOMODE_PAN_SCAN;
  else if (ratio == 2)
    buf3.arg1 = DXR2_ASPECTRATIOMODE_NORMAL;
  printf("aspect ratio mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_ASPECT_RATIO_MODE, &buf3));


  // macrovision off
  buf3.arg1 = DXR2_MACROVISION_OFF;
  printf("macrovision status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_TV_MACROVISION_MODE, &buf3));



  // volume = max
  buf3.arg1 = 19;
  printf("volume status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_VOLUME, &buf3));


  // mute is OFF
  buf3.arg1 = DXR2_AUDIO_MUTE_OFF;
  printf("mute off = %i\n", ioctl(dxr2FD, DXR2_IOC_AUDIO_MUTE, &buf3));


  // 16 bit audio
  buf3.arg1 = DXR2_AUDIO_WIDTH_16;
  printf("audio width = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_DATA_WIDTH, &buf3));


  // audio freq = 44.1 kHz
  buf3.arg1 = DXR2_AUDIO_FREQ_96;
  printf("audio width = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_SAMPLE_FREQUENCY, &buf3));


  // not quite sure actually... 
  buf3.arg1 = DXR2_IEC958_ENCODED;
  printf("iec958 status = %i\n", ioctl(dxr2FD, DXR2_IOC_IEC958_OUTPUT_MODE, &buf3));


  // turn off subpictures (i.e. annoying subtitles)
  buf3.arg1 = DXR2_SUBPICTURE_ON;
  printf("subpicture status = %i\n", ioctl(dxr2FD, DXR2_IOC_ENABLE_SUBPICTURE, &buf3));


  // CHANGEME
  // select audio stream.
  // arg1 is the type of audio stream... for MPEG1 (VCD), you'll want DXR2_STREAM_AUDIO_MPEG,
  // and for AC3, you'll want DXR2_STREAM_AUDIO_AC3
  // arg2 is the ID of WHICH audio stream to play.
  buf3.arg1 = DXR2_STREAM_AUDIO_AC3;
  //  buf3.arg1 = DXR2_STREAM_AUDIO_MPEG;
  buf3.arg2 = audio_stream;
  printf("select stream status = %i\n", ioctl(dxr2FD, DXR2_IOC_SELECT_STREAM, &buf3));


  // select video stream. Just default to the first ATM
  buf3.arg1 = DXR2_STREAM_VIDEO;
  buf3.arg2 = 0;
  printf("select stream status = %i\n", ioctl(dxr2FD, DXR2_IOC_SELECT_STREAM, &buf3));

  // subpicture off
  buf3.arg1 = DXR2_SUBPICTURE_OFF;
  printf("subpicture status = %i\n", ioctl(dxr2FD, DXR2_IOC_ENABLE_SUBPICTURE, &buf3));


  buf6.arg1 = 0;
  buf6.arg2 = 0;
  buf6.arg3 = 55;
  buf6.arg4 = 300;
  printf("set overlay cropping status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_CROPPING, &buf6));



  // get screen res
  getScreenRes(&xRes, &yRes);

  // OK, fork off another process to do the whitescreen thang.
  pid = fork();
  if (pid == 0) { // i.e. the child process
    
    whitescreen();
    exit(0);
  }

  // set colour key
  buf9.arg1 = 0x40;
  buf9.arg2 = 0xff;
  buf9.arg3 = 0x40;
  buf9.arg4 = 0xff;
  buf9.arg5 = 0x40;
  buf9.arg6 = 0xff;
  printf("set overlay colour status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_COLOUR, &buf9));



  // setup window key
  buf6.arg1 = 1000;
  printf("set overlay ratio status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_RATIO, &buf6));

  buf6.arg1 = 100;
  buf6.arg2 = 3;
  printf("set overlay position status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_POSITION, &buf6));

  buf6.arg1 = xRes;
  buf6.arg2 = yRes;
  printf("set overlay dimension status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_DIMENSION, &buf6));

  // back to window keying
  buf3.arg1 = 3;
  printf("set overlay mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_IN_DELAY, &buf3));


  // turn window AND colour keying on
  buf3.arg1 = DXR2_OVERLAY_WINDOW_COLOUR_KEY;
  printf("set overlay mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_MODE, &buf3));

  // calculate VGA parameters
  vgaBuf.xScreen = xRes;
  vgaBuf.yScreen = yRes;
  vgaBuf.hOffWinKey = 100;
  vgaBuf.vOffWinKey = 3;
  printf("calc = %i\n", ioctl(dxr2FD, DXR2_IOC_CALCULATE_VGA_PARAMETERS, &vgaBuf));

  // set VGA parameters
  printf("set = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_VGA_PARAMETERS, &vgaBuf));

  buf6.arg1 = xRes;
  buf6.arg2 = yRes;
  printf("set overlay dimension status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_DIMENSION, &buf6));

  buf6.arg1 = 0;
  buf6.arg2 = 0;
  printf("set overlay position status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_POSITION, &buf6));

  kill( pid, 9 ) ;

  // back to window keying
  buf3.arg1 = DXR2_OVERLAY_WINDOW_KEY;
  printf("set overlay mode status = %i\n", ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_MODE, &buf3));

  

  // enter play mode!
  printf("play status = %i\n", ioctl(dxr2FD, DXR2_IOC_PLAY, NULL));

  /*
  // Silly test thing - thanks for the input Lucien :-P
  if (!fork()) {
    int i ;

    double vppos_ang = 0 ;
    double vpsize_ang = 0 ;

    while(1) {
      buf6.arg1 = ((xRes/4.0) - ((1.0) + sin(vpsize_ang)) * (xRes/9.0) );
      buf6.arg2 = ((yRes/4.0) - ((1.0) + cos(vpsize_ang)) * (yRes/9.0) );
      ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_DIMENSION, &buf6);
      vpsize_ang = vpsize_ang + (3.14159/360);

      
      buf6.arg1 = ((xRes/2.0)+(sin(vppos_ang)*(xRes/4.0)));
      buf6.arg2 = ((yRes/2.0)+(cos(vppos_ang)*(yRes/4.0)));
      ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_POSITION, &buf6);
      vppos_ang = vppos_ang + (3.14159/360);


      for(i=0; i< 1000000; i++);
    }
    exit(1);
  }
*/

  // main loop, sending data to card.
  size=1;
  while(size != 0) {
    

    size = read(mpegFD, buffer, 0x8000);
    write(dxr2FD, buffer, size);
  }

  // close it again
  close(dxr2FD);
  if(mpegFD != STDIN_FILENO)
    close(mpegFD);
  if (driveFD != -1) {

    close(driveFD);
  }

  return(0);
}
