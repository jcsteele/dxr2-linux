#include "dxr2-api.h"
#include <dxr2ioctl.h>
#include <malloc.h>

static int dxr2FD = -1;
static int current_playmode = -1;

int dxr2_init(int _dxr2FD)
{
  dxr2FD = _dxr2FD;

  dxr2_reset();
}

int dxr2_set_params(dxr2_status_info_t* dxr2_info)
{ 
  dxr2_select_video(dxr2_info->video_stream.arg2);
  dxr2_set_video_freq(dxr2_info->video_freq.arg1);
  dxr2_set_video_type(dxr2_info->bitstream_type.arg);


  dxr2_set_tv_format(dxr2_info->tv_format.arg);
  dxr2_set_output_ratio(dxr2_info->output_aspect_ratio.arg);
  dxr2_set_source_ratio(dxr2_info->source_aspect_ratio.arg);
  dxr2_set_scaling_mode(dxr2_info->scaling_mode.arg);
  dxr2_set_macro_vision(dxr2_info->macro_vision.arg);
  dxr2_set_pixel_mode(dxr2_info->pixel_mode.arg);
  dxr2_set_interlace_mode(dxr2_info->interlaced_mode.arg);

  // what are these?  Do we need a fn for them?
  ioctl(dxr2FD, DXR2_IOC_SET_TV_75IRE_MODE, &dxr2_info->x75ire_mode);
  ioctl(dxr2FD, DXR2_IOC_IEC958_OUTPUT_MODE, &dxr2_info->iec_output_mode);
  
  
  dxr2_subpicture(dxr2_info->subpicture.arg);
  dxr2_select_subpicture(dxr2_info->subpicture_stream_id.arg2);

  dxr2_select_audio(dxr2_info->audio_stream.arg1, dxr2_info->audio_stream.arg2);
  dxr2_set_audio_width(dxr2_info->audio_width.arg);
  dxr2_set_audio_freq(dxr2_info->audio_freq.arg);
  dxr2_set_volume(dxr2_info->volume.arg);
  dxr2_mute(dxr2_info->mute.arg);

  dxr2_set_overlay_gain(dxr2_info->gain.arg1, dxr2_info->gain.arg2,
			dxr2_info->gain.arg3, dxr2_info->gain.arg4);

  // should be setup before autodetection occurs
  dxr2_set_overlay_crop(dxr2_info->overlay_crop.arg1, dxr2_info->overlay_crop.arg2,
			dxr2_info->overlay_crop.arg3, dxr2_info->overlay_crop.arg4);

  // THIS does not appear to be implimented yet.
  //  dxr2_set_overlay_picture_controls(dxr2_info->picture.arg1, dxr2_info->picture.arg2,
  //				    dxr2_info->picture.arg3, dxr2_info->picture.arg4);

  dxr2_set_in_delay(dxr2_info->in_delay.arg);
}
 

int dxr2_reset()
{
  return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_RESET);
}

int dxr2_install_firmware(int dxr2FD, dxr2_uCode_t* uCode)
{
  if(uCode != NULL)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_INIT_ZIVADS, uCode);
  else
    return -1;
}

int dxr2_set_playmode(int playmode)
{
  int ioc=-1;

  if(playmode >= DXR2_PLAYMODE_STOPPED && playmode <= DXR2_PLAYMODE_FASTBACKWARDS && dxr2FD >= 0)
    switch(playmode) {
    case DXR2_PLAYMODE_STOPPED:
      ioc = DXR2_IOC_STOP;
      break;
    case DXR2_PLAYMODE_PAUSED:
      ioc = DXR2_IOC_PAUSE;
      break;
    case DXR2_PLAYMODE_SLOWFORWARDS:
      ioc = DXR2_IOC_SLOW_FORWARDS;
      break;
    case DXR2_PLAYMODE_SLOWBACKWARDS:
      ioc = DXR2_IOC_SLOW_BACKWARDS;
      break;
    case DXR2_PLAYMODE_SINGLESTEP:
      ioc = DXR2_IOC_SINGLE_STEP;
      break;
    case DXR2_PLAYMODE_PLAY:
      ioc = DXR2_IOC_PLAY;
      break;
    case DXR2_PLAYMODE_REVERSEPLAY:
      ioc = DXR2_IOC_REVERSE_PLAY;
      break;
    case DXR2_PLAYMODE_FASTFORWARDS:
      return -1;
    case DXR2_PLAYMODE_FASTBACKWARDS:
      return -1;
    }

  current_playmode = playmode;

  if(ioc >= 0)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, ioc, NULL);
  else
    return -1;
}

int dxr2_fast_forwards(int rate)
{
  dxr2_oneArg_t newRate;

  if(rate<DXR2_PLAYRATE_2x || rate>DXR2_PLAYRATE_6x)
    return -1;

  newRate.arg = rate;

  current_playmode = DXR2_PLAYMODE_FASTFORWARDS;


  return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_FAST_FORWARDS, &newRate);
}

int dxr2_slow_forwards(int rate)
{
  dxr2_oneArg_t newRate;

  if(rate<DXR2_PLAYRATE_2x || rate>DXR2_PLAYRATE_6x)
    return -1;

  newRate.arg = rate;

  current_playmode = DXR2_PLAYMODE_SLOWFORWARDS;


  return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SLOW_FORWARDS, &newRate);
}

int dxr2_set_tv_format(int format)
{
  if(format >= DXR2_OUTPUTFORMAT_NTSC && format <= DXR2_OUTPUTFORMAT_PAL_60)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_TV_OUTPUT_FORMAT, &format);
  else
    return -1;
}

int dxr2_set_output_ratio(int ratio)
{
  if(ratio >= DXR2_ASPECTRATIO_4_3 && ratio <= DXR2_ASPECTRATIO_16_9)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OUTPUT_ASPECT_RATIO, &ratio);
  else
    return -1;
}

int dxr2_set_source_ratio(int ratio)
{
  if(ratio >= DXR2_ASPECTRATIO_4_3 && ratio <= DXR2_ASPECTRATIO_16_9)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_SOURCE_ASPECT_RATIO, &ratio);
  else
    return -1;
}

int dxr2_set_scaling_mode(int mode)
{
  if(mode >= DXR2_ASPECTRATIOMODE_NORMAL && mode <= DXR2_ASPECTRATIOMODE_LETTERBOX)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_ASPECT_RATIO_MODE, &mode);
  else
    return -1;
}

int dxr2_set_macro_vision(int mode)
{
  if(mode >= DXR2_MACROVISION_OFF && mode <= DXR2_MACROVISION_AGC_4COLOURSTRIPE)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_TV_MACROVISION_MODE, &mode);
  else
    return -1;
}

int dxr2_set_pixel_mode(int mode)
{
  if(mode >= DXR2_PIXEL_CCIR601 && mode <= DXR2_PIXEL_SQUARE)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_TV_PIXEL_MODE, &mode);
  else
    return -1;
}

int dxr2_set_interlace_mode(int mode)
{
  if(mode == DXR2_INTERLACED_OFF || mode == DXR2_INTERLACED_ON)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_TV_INTERLACED_MODE, &mode);
  else
    return -1;
}

int dxr2_subpicture(int mode)
{
  if(mode == DXR2_SUBPICTURE_ON || mode==DXR2_SUBPICTURE_OFF)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_ENABLE_SUBPICTURE, &mode);
  else
    return -1;
}

/*  This should do the subtitles correctly  */
int dxr2_set_subpicture_palettes(dxr2_palette_t pal)
{
  return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_SUBPICTURE_PALETTE, &pal);
}

int dxr2_select_subpicture(int stream)
{
  dxr2_twoArg_t stream_id;

  stream_id.arg1 = DXR2_STREAM_SUBPICTURE;
  stream_id.arg2 = stream;

  if(stream >= 0 && stream <= 31)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SELECT_STREAM, &stream_id);
  else
    return -1;
}

int dxr2_select_audio(int type, int stream)
{
  dxr2_threeArg_t stream_id;

  stream_id.arg1 = type;
  stream_id.arg2 = stream;

  if(stream >= 0 && stream <= 7)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SELECT_STREAM, &stream_id);
  else
    return -1;
}

int dxr2_set_audio_width(int width)
{
  if(width >= DXR2_AUDIO_WIDTH_16 && width <= DXR2_AUDIO_WIDTH_24)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_DATA_WIDTH, &width);
  else
    return -1;
}

int dxr2_set_audio_freq(int freq)
{
  if(freq >= DXR2_AUDIO_FREQ_441 && freq <= DXR2_AUDIO_FREQ_32)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_SAMPLE_FREQUENCY, &freq);
  else
    return -1;
}

int dxr2_set_volume(int volume)
{
  if(volume >= 0 && volume <= 19)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_AUDIO_VOLUME, &volume);
  else
    return -1;
}

int dxr2_mute(int status)
{
  if(status == DXR2_AUDIO_MUTE_ON || status == DXR2_AUDIO_MUTE_OFF)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_AUDIO_MUTE, &status);
  else
    return -1;
}

int dxr2_select_video(int stream)
{
  dxr2_twoArg_t stream_id;

  stream_id.arg1 = DXR2_STREAM_VIDEO;
  stream_id.arg2 = stream;

  if(stream >= 0) // is there a max stream?
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SELECT_STREAM, &stream_id);
  else
    return -1;
}

int dxr2_set_video_freq(int freq)
{
  dxr2_threeArg_t video_freq;

  video_freq.arg1 = freq;
  video_freq.arg2 = 0x2d0; // what is this?
  video_freq.arg3 = 0x1e0; // what is this?

  if(freq == DXR2_SRC_VIDEO_FREQ_30 || freq == DXR2_SRC_VIDEO_FREQ_25)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_SOURCE_VIDEO_FORMAT, &video_freq);
  else
    return -1;
}

int dxr2_set_video_type(int type)
{
  if(type >= DXR2_BITSTREAM_TYPE_MPEG_VOB && type <= DXR2_BITSTREAM_TYPE_4)
    return  dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_BITSTREAM_TYPE, &type);
  else
    return -1;
}

int dxr2_set_overlay_mode(int mode)
{
  if(mode >= DXR2_OVERLAY_DISABLED && mode <= DXR2_OVERLAY_WINDOW_COLOUR_KEY)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_MODE, &mode);
  else
    return -1;
}

int dxr2_set_overlay_ratio(int ratio)
{
  if(ratio >= 0 && ratio <= 2500) // is this the correct max?
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_RATIO, &ratio);
  else
    return -1;
}

int dxr2_set_overlay_geom(geom_t geom)
{
  return dxr2_set_overlay_position(geom.x, geom.y) ||
         dxr2_set_overlay_size(geom.width, geom.height);
}

int dxr2_set_overlay_position(int x, int y)
{
  dxr2_twoArg_t pos;

  pos.arg1=x;
  pos.arg2=y;

  if(x >= 0 && y >= 0)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_POSITION, &pos);
  else
    return -1;
}

int dxr2_set_overlay_size(int width, int height)
{
  dxr2_twoArg_t size;

  size.arg1=width;
  size.arg2=height;

  if(width >= 0 && height >= 0)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_DIMENSION, &size);
  else
    return -1;
}

int dxr2_set_overlay_crop(int left, int right, int top, int bottom)
{
  dxr2_fourArg_t crop;
  
  crop.arg1=left;
  crop.arg2=right;
  crop.arg3=top;
  crop.arg4=bottom;

  if(left >= 0 && right>=0 && top>=0 && bottom>=0)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_CROPPING, &crop);
  else
    return -1;
}

int dxr2_set_in_delay(int _delay)
{
  dxr2_oneArg_t delay;

  delay.arg = _delay;
  if(_delay >= 0 && _delay <= 3)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_IN_DELAY, &delay);
  else
    return -1;
}

int dxr2_set_overlay_gain(int common_gain, int red_gain, int green_gain, int blue_gain)
{
  dxr2_fourArg_t picture;

  picture.arg1=common_gain;
  picture.arg2=red_gain;
  picture.arg3=green_gain;
  picture.arg4=blue_gain;

  if( common_gain >= 0 && common_gain <= 63 &&
      red_gain >= 0 && red_gain <= 63 &&
      green_gain >= 0 && green_gain <= 63 &&
      blue_gain >= 0 && blue_gain <= 63 )
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_GAIN, &picture);
  return -1;
}

int dxr2_set_overlay_picture_controls(int gamma, int contrast, int brightness,
				      int saturation)
{
  dxr2_fourArg_t picture;

  picture.arg1 = gamma;
  picture.arg2 = contrast;
  picture.arg3 = brightness;
  picture.arg4 = saturation;

  if( gamma>=0 && gamma<= 96 &&
      contrast>=-128 && contrast<=127 &&
      brightness>=-128 && brightness<=127 &&
      saturation>=-128 && saturation<=127)
    return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_PICTURE_CONTROLS, &picture);

  return -1;
}

int dxr2_set_overlay_color_key(int red_min, int red_max, int green_min, int green_max,
			       int blue_min, int blue_max)
{
  dxr2_sixArg_t colorKey;

  colorKey.arg1 = red_min;
  colorKey.arg2 = red_max;
  colorKey.arg3 = green_min;
  colorKey.arg4 = green_max;
  colorKey.arg5 = blue_min;
  colorKey.arg6 = blue_max;

  return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_SET_OVERLAY_COLOUR, &colorKey);
}

int dxr2_measure_vga_parameters(dxr2_vgaParams_t* vgaParams)
{
  return dxr2FD<0 ? -1 : ioctl(dxr2FD, DXR2_IOC_CALCULATE_VGA_PARAMETERS, vgaParams);
}

 
int dxr2_setup_overlay(dxr2_vgaParams_t* vgaParams, dxr2_status_info_t* dxr2_info)
{
  dxr2_sixArg_t buf6;

  ioctl(dxr2FD, DXR2_IOC_SET_VGA_PARAMETERS, vgaParams);

  dxr2_set_overlay_color_key(dxr2_info->color_key.arg1, dxr2_info->color_key.arg2,
			     dxr2_info->color_key.arg3, dxr2_info->color_key.arg4,
			     dxr2_info->color_key.arg5, dxr2_info->color_key.arg6);
  dxr2_set_overlay_crop(dxr2_info->overlay_crop.arg1, dxr2_info->overlay_crop.arg2,
			dxr2_info->overlay_crop.arg3, dxr2_info->overlay_crop.arg4);
  dxr2_set_overlay_size(dxr2_info->overlay_dim.arg1, dxr2_info->overlay_dim.arg2);
  dxr2_set_overlay_position(dxr2_info->overlay_pos.arg1, dxr2_info->overlay_pos.arg2);

  return dxr2_set_overlay_mode(dxr2_info->overlay_mode.arg);
}




