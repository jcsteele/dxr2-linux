#include <stdio.h>
#include <errno.h>
#include "interface.h"
#include "config-files.h"

void read_config(player_info_t* player_info, dxr2_status_info_t* dxr2_info)
{
  char* config_file="/etc/dxr2player.conf";
  FILE* config;
  char input[20];

  config = fopen(config_file, "r");

  if(errno) {
    destroy_interface();
    fprintf(stderr, "Error loading config file!\n");
    exit(1);
  }
  else {
    while(!feof(config)) {
      fscanf(config, "%s", input);
      if(input[0]=='#') /* skip to end of line */
	while(input[0]!='\n')
	  fscanf(config, "%c", input);
      else {
	if(strcmp(input, "drive:")==0) 
	  fscanf(config, "%s", player_info->dvd_device);
	else if(strcmp(input, "dxr2:")==0) 
	  fscanf(config, "%s", player_info->dxr2_device);
	else if(strcmp(input, "mountpoint:")==0) 
	  fscanf(config, "%s", player_info->dvd_mountpoint);
	else if(strcmp(input, "firmware:")==0)
	  fscanf(config, "%s", player_info->uCode_file);
	else if(strcmp(input, "audio_format:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "AC3")==0)
	    dxr2_info->audio_stream.arg1=DXR2_STREAM_AUDIO_AC3;
	  else if(strcmp(input, "MPEG")==0)
	    dxr2_info->audio_stream.arg1=DXR2_STREAM_AUDIO_MPEG;
	  else if(strcmp(input, "PCM")==0 || strcmp(input,"LPCM")==0)
	    dxr2_info->audio_stream.arg1=DXR2_STREAM_AUDIO_LPCM;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is an invalid stream type!", 
		    config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "audio_freq:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "44.1")==0)
	    dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_441;
	  else if(strcmp(input, "48")==0)
	    dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_48;
	  else if(strcmp(input, "96")==0)
	    dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_96;
	  else if(strcmp(input, "2205")==0)
	    dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_2205;
	  else if(strcmp(input, "32")==0)
	    dxr2_info->audio_freq.arg=DXR2_AUDIO_FREQ_32;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is an invalid frequency!", 
		    config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "audio_width:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "16")==0)
	    dxr2_info->audio_width.arg=DXR2_AUDIO_WIDTH_16;
	  else if(strcmp(input, "20")==0)
	    dxr2_info->audio_width.arg=DXR2_AUDIO_WIDTH_20;
	  else if(strcmp(input, "24")==0)
	    dxr2_info->audio_width.arg=DXR2_AUDIO_WIDTH_24;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument is an %s invalid bitstream width!", 
		    config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "tv_format:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "NTSC")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_NTSC;
	  else if(strcmp(input, "NTSC60")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_NTSC_60;
	  else if(strcmp(input, "PAL_M")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_M;
	  else if(strcmp(input, "PAL_M_60")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_M_60;
	  else if(strcmp(input, "PAL_BDGHI")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_BDGHI;
	  else if(strcmp(input, "PAL_N")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_N;
	  else if(strcmp(input, "PAL_Nc")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_Nc;
	  else if(strcmp(input, "PAL_60")==0)
	    dxr2_info->tv_format.arg=DXR2_OUTPUTFORMAT_PAL_60;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument is an %s invalid tv format!", 
		    config_file, input);
	    exit(1);
	  }
	}
        else if(strcmp(input, "video_freq:")==0) {
          fscanf(config, "%s", input);
          if(strcmp(input, "30")==0)
            dxr2_info->video_freq.arg1=DXR2_SRC_VIDEO_FREQ_30;
          else if(strcmp(input, "25")==0)
            dxr2_info->video_freq.arg1=DXR2_SRC_VIDEO_FREQ_25;
          else {
	    destroy_interface();
            print_error("ERROR in config file %s, argument %s is an invalid video frequency", config_file, input);
            exit(1);
          }
        }
	else if(strcmp(input, "video_format:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "letterbox")==0)
	    dxr2_info->scaling_mode.arg=DXR2_ASPECTRATIOMODE_LETTERBOX;
	  else if(strcmp(input, "normal")==0)
	    dxr2_info->scaling_mode.arg=DXR2_ASPECTRATIOMODE_NORMAL;
	  else if(strcmp(input, "pan_scan")==0)
	    dxr2_info->scaling_mode.arg=DXR2_ASPECTRATIOMODE_PAN_SCAN;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument is an %s invalid video format!", 
		    config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "subpicture:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "OFF")==0)
	    dxr2_info->subpicture.arg=DXR2_SUBPICTURE_OFF;
	  else if(strcmp(input, "ON")==0)
	    dxr2_info->subpicture.arg=DXR2_SUBPICTURE_ON;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is invalid!", config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "overlay:")==0) {
	  fscanf(config, "%s", input);
	  if(strcmp(input, "ON")==0)
	    dxr2_info->overlay_mode.arg=DXR2_OVERLAY_WINDOW_COLOUR_KEY;
	  else if(strcmp(input, "OFF")==0)
	    dxr2_info->overlay_mode.arg=DXR2_OVERLAY_DISABLED;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is invalid!", config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "overlay_geom:")==0) {
	  fscanf(config, "%s", input);
	  if (parse_geom(dxr2_info, input))
	    ;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is invalid!", config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "overlay_crop:")==0) {
	  fscanf(config, "%s", input);
	  if (parse_crop(dxr2_info, input))
	    ;
	  else {
	    destroy_interface();
	    print_error("ERROR in config file %s, argument %s is invalid!", config_file, input);
	    exit(1);
	  }
	}
	else if(strcmp(input, "red:")==0) {
	  fscanf(config, "%x-%x", &dxr2_info->color_key.arg1, &dxr2_info->color_key.arg2);
	  if(dxr2_info->color_key.arg1 < 0x00 || dxr2_info->color_key.arg1 > 0xff ||
	     dxr2_info->color_key.arg2 < 0x00 || dxr2_info->color_key.arg2 > 0xff ||
	     dxr2_info->color_key.arg2 < dxr2_info->color_key.arg1) {
	    destroy_interface();
	    print_error("ERROR: Red values must be between 0x00 and 0xff.  Max must not be below min\n");
	    exit(1);
	  }
	}
	else if(strcmp(input, "green:")==0) {
	  fscanf(config, "%x-%x", &dxr2_info->color_key.arg3, &dxr2_info->color_key.arg4);
	  if(dxr2_info->color_key.arg3 < 0x00 || dxr2_info->color_key.arg3 > 0xff ||
	     dxr2_info->color_key.arg4 < 0x00 || dxr2_info->color_key.arg4 > 0xff ||
	     dxr2_info->color_key.arg4 < dxr2_info->color_key.arg3) {
	    destroy_interface();
	    print_error("ERROR: Green values must be between 0x00 and 0xff.  Max must not be below min\n");
	    exit(1);
	  }
	}
	else if(strcmp(input, "blue:")==0) {
	  fscanf(config, "%x-%x", &dxr2_info->color_key.arg5, &dxr2_info->color_key.arg6);
	  if(dxr2_info->color_key.arg5 < 0x00 || dxr2_info->color_key.arg5 > 0xff ||
	     dxr2_info->color_key.arg6 < 0x00 || dxr2_info->color_key.arg6 > 0xff ||
	     dxr2_info->color_key.arg6 < dxr2_info->color_key.arg5) {
	    destroy_interface();
	    print_error("ERROR: Blue values must be between 0x00 and 0xff.  Max must not be below min\n");
	    exit(1);
	  }
	}
      }
    }
  }
}

int parse_geom(dxr2_status_info_t* dxr2_info, char* geom)
{
  int width,height,xoffset,yoffset;

  width=height=xoffset=yoffset=0;

  if(sscanf(geom, "%dx%d+%d+%d", &width, &height, &xoffset, &yoffset) != 3) {
    dxr2_info->overlay_pos.arg1=xoffset;
    dxr2_info->overlay_pos.arg2=yoffset;
    dxr2_info->overlay_dim.arg1=width;
    dxr2_info->overlay_dim.arg2=height;
    return 1;
  }
  return 0;
}

int parse_crop(dxr2_status_info_t* dxr2_info, char* crop)
{
  int left,right,top,bottom;

  if(sscanf(crop, "%dx%dx%dx%d", &left,&right,&top,&bottom) != 3) {
    dxr2_info->overlay_crop.arg1=left;
    dxr2_info->overlay_crop.arg2=right;
    dxr2_info->overlay_crop.arg3=top;
    dxr2_info->overlay_crop.arg4=bottom;
    return 1;
  }
  return 0;
}
