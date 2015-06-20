/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *   
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "player.h"
#include "config.h"
#include "config-files.h"
#include "interface.h"
#include "multifile.h"

/* UNCOMMENT THIS TO TRY OVERLAY IN A WINDOW */
#include "overlay.h"

void initialize(int argc, char** argv, player_info_t* player_info, dxr2_status_info_t* dxr2_info)
{
  memset(player_info, 0, sizeof(player_info_t));
  player_info->mpegFD=player_info->driveFD=player_info->dxr2FD=-1;
  memset(dxr2_info, 0, sizeof(dxr2_status_info_t));
  dvd_initialise(&player_info->dvd_con);

  player_info->dxr2_info = dxr2_info;

  init_dxr2_info(dxr2_info);
  strcpy(player_info->uCode_file, "dvd1.ux");
  strcpy(player_info->dvd_mountpoint, "/cdrom");

  read_config(player_info, dxr2_info);
  if(!init_and_parse_args(argc, argv, player_info, dxr2_info)) {
    cleanup_and_exit(player_info);
  }

  if (player_info->file_name[0]==0 || argc < 2) {
    destroy_interface();
    printf("Syntax: %s [-cTO] [-s <bitstream type>] [-a <audio stream>]\n",argv[0]);
    printf("                  [-q <audio freq>] [-v <video freq>] [-s <bitstream>]\n");
    printf("                  [-W <TV type>] [-u <audio type>] [-w <audio width>]\n");
    printf("                  [-r <output ratio>] [-t <output type>] [-S <subtitle jid>]\n");
    printf("                  [-g <geom>] [-p <crop>] [-b <brightness>][-A <angle>] <filename>\n");
    printf("\n");
    printf("-c turns on CSS decryption\n");
    printf("-a will select the audio stream to decode (from 0 to 7)\n");
    printf("-r is either 0 1 or 2, 0 being letterbox, 1 being 'normal' and 2 being pan/scan\n");
    printf("-t is either 0 to 7 corresponding to: NTSC,NTSC_60,PAL_M,PAL_M_60,PAL_BDGHI,PAL_N,PAL_Nc,PAL_60\n");
    printf("-q selects the audio frequency.  0-4 corresponding to:  44.1, 48. 96, 2205, 32\n");
    printf("-v selects the video frequency.  0-1 corresponding to: 30fps, 20fps.  30fps is default for NTSC.  25fps is default for PAL.\n");
    printf("-W Selects whether you have a Widescreen TV(1) or a Normal Tv(0)\n");
    printf("-w selects the audio width.  0-2 corresponding to: 16, 20, 24\n");
    printf("-s selects the bitstream type.  0-4 corresponding to: VOB(DVD), CDROM VCD, MPEG VCD, CDDA, Unknown\n");
    printf("-u selects the audio bitstream type.  0-2 corresponding to: AC3 / MPEG / LPCM\n");
    printf("-S select subtitle language. 0-<number of subtitles languages on the DVD -1>\n");
    printf("-T turns on IFO parsing.  Plays specified title, 1-?  based on the number of titles on your dvd.\n");
    printf("-A selects which angle to view.  0-<maxangle>\n");
    printf("-O turns on the VGA overlay\n");
    printf("-g sets the geometry of the VGA overlay.  standard wxh+x+y format.\n");
    printf("-p sets the cropping info.  <left shift>x<right shift>x<top shift>x<bottom shift>\n");
    printf("-b sets the brightness for the VGA overlay.  0-63\n");
    exit(1);
  }

  // open DVD device
  if ((player_info->dxr2FD = open(player_info->dxr2_device, O_WRONLY)) < 0) {
    
    print_error("ERROR: Cannot open DVD device (%s): %s\n", "/dev/dxr2", strerror(errno));
    wait_if_needed();
    cleanup_and_exit(player_info);
  }  

  dxr2_css_set_dxr2_fd(player_info->dxr2FD);
  
  install_firmware(player_info);

  // reset the player
  dxr2_init(player_info->dxr2FD);

  // setup the rest of the parameters
  dxr2_set_params(dxr2_info);

  open_files(player_info);
  
  // **************************************************
  // THIS SHOULD BE CHANGED. IT IS A NASTY HACK!
  // initialise the overlay if necessary
  if(dxr2_info->overlay_mode.arg != DXR2_OVERLAY_DISABLED) {

    setup_overlay_params(player_info->dxr2FD, dxr2_info, "/etc/dxr2vga.conf");
    player_info->do_overlay = 1;
  // **************************************************

  /* UNCOMMENT THIS TO TRY OVERLAY IN A WIN */
  init_overlay(argc, argv, dxr2_info);
  }
}

void init_dxr2_info(dxr2_status_info_t* dxr2_info)
{
  // Default values
  dxr2_info->tv_format.arg= DXR2_OUTPUTFORMAT_NTSC;
  // Set this to an invalid value so that we set it later based on tv_format
  dxr2_info->video_freq.arg1=-1;
  dxr2_info->video_freq.arg2=0x2d0;
  dxr2_info->video_freq.arg3=0x1e0;
  dxr2_info->output_aspect_ratio.arg=DXR2_ASPECTRATIO_4_3;
  dxr2_info->source_aspect_ratio.arg=DXR2_ASPECTRATIO_4_3;
  dxr2_info->scaling_mode.arg=DXR2_ASPECTRATIOMODE_LETTERBOX;
  dxr2_info->bitstream_type.arg=DXR2_BITSTREAM_TYPE_MPEG_VOB;
  dxr2_info->macro_vision.arg=DXR2_MACROVISION_OFF;
  dxr2_info->pixel_mode.arg=DXR2_PIXEL_CCIR601;
  dxr2_info->interlaced_mode.arg=DXR2_INTERLACED_ON;
  dxr2_info->x75ire_mode.arg=DXR2_75IRE_OFF;
  dxr2_info->volume.arg=19;
  dxr2_info->mute.arg=DXR2_AUDIO_MUTE_OFF;
  dxr2_info->audio_width.arg=DXR2_AUDIO_WIDTH_16;
  dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_441;
  dxr2_info->iec_output_mode.arg=DXR2_IEC958_ENCODED;
  dxr2_info->subpicture.arg=DXR2_SUBPICTURE_OFF;
  dxr2_info->subpicture_stream_id.arg1=DXR2_STREAM_SUBPICTURE;
  dxr2_info->subpicture_stream_id.arg2=0;
  dxr2_info->audio_stream.arg1=DXR2_STREAM_AUDIO_AC3;
  dxr2_info->audio_stream.arg2=0;
  dxr2_info->video_stream.arg1=DXR2_STREAM_VIDEO;
  dxr2_info->video_stream.arg2=0;

  /* VGA OVERLAY INFO */
  dxr2_info->overlay_crop.arg1=0; // left
  dxr2_info->overlay_crop.arg2=0; // right
  dxr2_info->overlay_crop.arg3=20; // top
  dxr2_info->overlay_crop.arg4=20;  // bottom
  dxr2_info->overlay_pos.arg1=81; // origin X
  dxr2_info->overlay_pos.arg2=36; // origin Y
  dxr2_info->overlay_dim.arg1=800; // size horiz
  dxr2_info->overlay_dim.arg2=600; // size vert
  dxr2_info->overlay_mode.arg=DXR2_OVERLAY_DISABLED;
  dxr2_info->gain.arg1 = 63; // common gain (0-63)
  dxr2_info->gain.arg2 = 26; // red-gain (0-63)
  dxr2_info->gain.arg3 = 40; // green-gain (0-63)
  dxr2_info->gain.arg4 = 40; // blue-gain (0-63)
  dxr2_info->picture.arg1 = 10; // gamma
  dxr2_info->picture.arg2 = 100; // contrast
  dxr2_info->picture.arg3 = 100; // brightness
  dxr2_info->picture.arg4 = 0; // saturation INVALID
  dxr2_info->color_key.arg1 = 0x00; // red-min
  dxr2_info->color_key.arg2 = 0x20; // red-max
  dxr2_info->color_key.arg3 = 0x00; // green-min
  dxr2_info->color_key.arg4 = 0x20; // green-max
  dxr2_info->color_key.arg5 = 0x60; // blue-min
  dxr2_info->color_key.arg6 = 0xff; // blue-max
}

int init_and_parse_args(int argc, char** argv, player_info_t* player_info, dxr2_status_info_t* dxr2_info)
{ 
  char optionsString[] = "ca:g:p:r:t:q:w:s:u:v:OT:S:W:A:b:"; 
  char geom[40];
  char crop[40];
  int ratio, output_type=-1, current_opt, audio_freq, audio_width;
  int video_freq=-1;
  int audio_type=-1;
  int subtitle_id=-1;
  
  strcpy(player_info->file_name, *(argv+argc-1));
  strcpy(geom, "");
  strcpy(crop, "");

  while((current_opt = getopt(argc,argv,optionsString))!=-1) {
    if(current_opt == 'a')
      sscanf(optarg,"%d",&dxr2_info->audio_stream.arg2);
    else if (current_opt=='r')
      sscanf(optarg,"%d",&ratio);
    else if (current_opt=='t')
      sscanf(optarg,"%d",&output_type);
    else if(current_opt =='c')
      player_info->encrypted = 1;
    else if(current_opt == 'g')
      sscanf(optarg,"%s", geom);
    else if(current_opt == 'p')
      sscanf(optarg, "%s", crop);
    else if(current_opt =='w')
      sscanf(optarg,"%d",&dxr2_info->audio_width.arg);
    else if(current_opt =='q')
      sscanf(optarg,"%d",&dxr2_info->audio_freq.arg);
    else if(current_opt =='s')
      sscanf(optarg,"%d",&dxr2_info->bitstream_type.arg);
    else if(current_opt =='u')
      sscanf(optarg,"%d",&audio_type);
    else if(current_opt == 'v')
      sscanf(optarg, "%d", &video_freq);
    else if(current_opt == 'A')
      sscanf(optarg, "%d", &player_info->dvd_con.angle);
    else if(current_opt == 'T')
    {
      player_info->dvd = 1;
      sscanf(optarg, "%d", &player_info->dvd_con.title);
      player_info->dvd_con.title--;
    }
    else if(current_opt == 'W')
      sscanf(optarg, "%d", &dxr2_info->output_aspect_ratio.arg);
    else if(current_opt =='S') {
      dxr2_info->subpicture.arg=DXR2_SUBPICTURE_ON;
      sscanf(optarg,"%d",&subtitle_id);
    }
    else if(current_opt=='O') 
      dxr2_info->overlay_mode.arg=DXR2_OVERLAY_WINDOW_COLOUR_KEY;
    else if(current_opt=='b')
      sscanf(optarg, "%d", &dxr2_info->picture.arg1);
  }

  if (player_info->dvd)
  {
    player_info->do_css = 1;
    if (snprintf(player_info->file_name, 80, "%s",
		 player_info->dvd_mountpoint)>75)
    {
      print_error("Not enough space allocated to file_name\n");
      return 0;
    }
  }

  if(subtitle_id >= 0)
    dxr2_info->subpicture_stream_id.arg2=subtitle_id;

  if (ratio == 0) 
    dxr2_info->scaling_mode.arg = DXR2_ASPECTRATIOMODE_LETTERBOX;
  else if (ratio == 2)
    dxr2_info->scaling_mode.arg = DXR2_ASPECTRATIOMODE_PAN_SCAN;
  else if (ratio == 2)
    dxr2_info->scaling_mode.arg = DXR2_ASPECTRATIOMODE_NORMAL;

  if (output_type >=0  && output_type <= 7) {
    dxr2_info->tv_format.arg = output_type;
  }
  else if(output_type != -1) {
    print_error("Invalid Output format.  must be 0-6 corresponding to: \n");
    print_error("NTSC, NTSC_60, PAL_M, PAL_M_60, PAL_BDGHI, PAL_N, PAL_Nc\n");
  }

  if(dxr2_info->audio_freq.arg < 0 || dxr2_info->audio_freq.arg > 4) {
    print_error("Invalid frequency.  0=44.1 KHz, 1=48 KHz, 2=96 KHz,\n");
    print_error("   3=2205 Hz, 4=32 Khz.\n");
    return 0;;
  }
 
  if(dxr2_info->audio_width.arg < 0 || dxr2_info->audio_width.arg > 2) {
    print_error("Invalid width.  0=16, 1=20, 2=24\n");
    return 0;
  }

  if(dxr2_info->bitstream_type.arg < 0 || dxr2_info->bitstream_type.arg > 4) {
    print_error("Invalid biststream type.  0=VOB (DVD), 1=CDROM VCD (raw sector mode?), 2=MPEGVCD, 3=CDDA, 4=unknown\n");
    return 0;
  }
  else if(dxr2_info->bitstream_type.arg==1)
    player_info->vcd = 1;

  // setup audio types properly (this should probably be configurable?)
  if(audio_type == -1 && dxr2_info->audio_stream.arg1==0) {
    // if we havn't specified something, either here or in the config file...
    if(dxr2_info->bitstream_type.arg == DXR2_BITSTREAM_TYPE_MPEG_VOB) {
      dxr2_info->audio_stream.arg1 = DXR2_STREAM_AUDIO_AC3;
    }
    else if (dxr2_info->bitstream_type.arg == DXR2_BITSTREAM_TYPE_MPEG_VCD) {
      dxr2_info->audio_stream.arg1 = DXR2_STREAM_AUDIO_MPEG;
    }
  }
  else if(audio_type == -1)
    ; // specified in the config file, but not at command line.  do nothing.
  else if(audio_type < 0 || audio_type > 2) {
    print_error("Invalid audio type.  0==AC3, 1==MPEG, 2==LPCM\n");
    return 0;
  }
  else
    dxr2_info->audio_stream.arg1 = audio_type+2;  // shift to correspond to correct numbers.

  // If we have not set the video frequency yet, set it to default.
  if(dxr2_info->video_freq.arg1==-1 && video_freq == -1)
    if ((output_type == DXR2_OUTPUTFORMAT_NTSC) ||
	(output_type == DXR2_OUTPUTFORMAT_NTSC_60)) {
      
      video_freq = DXR2_SRC_VIDEO_FREQ_30;
    }
    else {

      video_freq = DXR2_SRC_VIDEO_FREQ_25;
    }

  if(player_info->dvd_con.angle < 0) {
    print_error("ERROR: -A must choose an angle > 0\n");
    destroy_interface();
    exit(1);
  }

  if(video_freq==0) 
    dxr2_info->video_freq.arg1 = DXR2_SRC_VIDEO_FREQ_30;
  else if(video_freq==1) 
    dxr2_info->video_freq.arg1 = DXR2_SRC_VIDEO_FREQ_25;

  if(dxr2_info->output_aspect_ratio.arg < 0 || dxr2_info->output_aspect_ratio.arg > 1) {
    print_error("ERROR: -W must have either a 0 or a 1 as its argument.\n");
    destroy_interface();
    exit(0);
  }

  if(strcmp(geom, "") != 0)
    parse_geom(dxr2_info, geom);

  if(strcmp(crop, "") != 0)
    parse_crop(dxr2_info, crop);

  if(dxr2_info->picture.arg1 < -128 || dxr2_info->picture.arg1 > 127) {
    print_error("ERROR: invalid brightness level. must be 0-63\n");
    destroy_interface();
    exit(1);
  }

  return 1;
}

void open_files(player_info_t* player_info)
{
  open_new_vob(player_info, player_info->file_name);

  // open drive
  if (player_info->dvd)
  {
    if ((player_info->driveFD = open(player_info->dvd_device, O_RDONLY)) < 0) {
      
      print_error("ERROR: Cannot open DVD drive (%s): %s\n", player_info->dvd_device, strerror(errno));
      wait_if_needed();
      cleanup_and_exit(player_info);
    }  
  }
}

int open_new_vob(player_info_t* player_info, char* file_name)
{
  char** names;

  if (file_name != player_info->file_name) {
#ifdef __DVD_DEBUG
    print_info("setting file name to %s", file_name);
#endif
    strcpy(player_info->file_name, file_name);
  }

  names = generate_filenames(file_name);

  if (player_info->file_name[0] == '-') {
    print_error("using stdin for mpeg file\n");
    player_info->mpegFD= STDIN_FILENO;
  }
  else if (player_info->dvd)
  {
    if (!dvd_setup(&player_info->dvd_con, file_name))
    {
      print_error("ERROR: Could not open VOB title.  Try it without -T\n");
      wait_if_needed();
      cleanup_and_exit(player_info); 
    }
  }
  else if ((player_info->mpegFD = multi_open(names, player_info->vcd)) < 0) {
      print_error("ERROR: Could not open mpeg (%s): %s\n", player_info->file_name, strerror(errno));
      wait_if_needed();
      cleanup_and_exit(player_info);
  }
  return 1;
}


void install_firmware(player_info_t* player_info)
{
  int uCodeFD;
  int uCodeSize;
  dxr2_uCode_t* uCode;

  if ((uCodeFD = open(player_info->uCode_file, O_RDONLY)) < 0) {
    
    print_error("ERROR: Could not open uCode (%s): %s\n", player_info->uCode_file, strerror(errno));
    wait_if_needed();
    cleanup_and_exit(player_info);
  }
  uCodeSize = lseek(uCodeFD, 0, SEEK_END);
  if ((uCode = malloc(uCodeSize + 4)) == NULL) {
    
    print_error("ERROR: Could not allocate memory for uCode: %s\n", strerror(errno));
    wait_if_needed();
    cleanup_and_exit(player_info);
  }
  lseek(uCodeFD, 0, SEEK_SET);
  if (read(uCodeFD, uCode+4, uCodeSize) != uCodeSize) {
    
    print_error("ERROR: Could not read uCode uCode: %s\n", strerror(errno));
    wait_if_needed();
    cleanup_and_exit(player_info);
  }
  close(uCodeFD);
  uCode->uCodeLength = uCodeSize;

  // upload ucode
  if (dxr2_install_firmware(player_info->dxr2FD, uCode)) {
    
    print_error("uCode upload failed!\n");
    wait_if_needed();
    cleanup_and_exit(player_info);
  }
}




