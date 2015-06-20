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

/**
 *
 * Overall driver for the Creative DXR2 card
 * Ioctl code
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <dxr2.h>
#include <bt865.h>
#include <zivaDS.h>



static int maxStreamTable[] = {
  
  // stream type 0 (MPEG_VOB)
  1,
  32,
  8,
  8,
  8,
  0,


  // stream type (CDROM_VCD)
  1,
  0,
  0,
  1,
  0,
  3,

  // stream type 2 (MPEG_VCD)
  1,
  0,
  0,
  1,
  0,
  3,

  // stream type 3 (CDDA)
  1,
  0,
  0,
  0,
  0,
  3,
  
  // stream type 4 (TYPE_4)
  1,
  0,
  0,
  0,
  0,
  3,
};


/**
 *
 * Table of clock values for LPCM streams
 *
 */

static int clockTable[] = {
  
  1,
  1,
  2,

  1,
  2,
  2,

  1,
  1,
  1,

  1,
  1,
  1,

  1,
  1,
  1 };



/**
 *
 * Get the region code for the card. Places it in supplied buffer
 *
 * @param instance DXR2 instance to use
 * @param buffer Instance of dxr2_oneArg_t. arg will be set to the region code
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_get_region_code(dxr2_t* instance, char* buffer) 
{
  int tmp;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;
  
  // read rom check
  tmp = dxr2_eeprom_read_byte(instance, 4) << 8;
  tmp |= dxr2_eeprom_read_byte(instance, 5);
  if (tmp != 0x6ee6) {
    
    put_user(0, &(argBuffer->arg));
    return(-EIO);
  }
  
  // read the country value
  tmp = dxr2_eeprom_read_byte(instance, 6) << 8;
  tmp |= dxr2_eeprom_read_byte(instance, 7);

  // copy the country code to the supplied buffer
  if (put_user(tmp, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // OK
  return(0);
}



/**
 *
 * Set the tv output TV format
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_OUTPUTFORMAT_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_tv_output_format(dxr2_t* instance, char* buffer)
{
  int format;
  int status;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(format, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check parameters
  if ((format < DXR2_OUTPUTFORMAT_NTSC) || 
      (format > DXR2_OUTPUTFORMAT_PAL_60)) {
    

    return(-EINVAL);
  }
  
  // set it  
  status = bt865_set_output_mode(instance->bt865Instance, format);
  
  // if we get here, it is an error
  return(status);
}


/**
 *
 * Set the tv output interlaced mode
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_INTERLACED_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_tv_interlaced_mode(dxr2_t* instance, char* buffer)
{
  int format;
  int status;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(format, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check parameters
  if ((format < DXR2_INTERLACED_OFF) || 
      (format > DXR2_INTERLACED_ON)) {
    
    return(-EINVAL);
  }
  
  // set it  
  status = bt865_set_interlaced_mode(instance->bt865Instance, format);
  
  // if we get here, it is an error
  return(status);
}




/**
 *
 * Set the tv black/white mode
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_BLACKWHITE_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_tv_blackwhite_mode(dxr2_t* instance, char* buffer)
{
  int format;
  int status;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(format, &(argBuffer->arg)) < 0) {
    

    return(-EFAULT);
  }
  
  // check parameters
  if ((format < DXR2_BLACKWHITE_OFF) || 
      (format > DXR2_BLACKWHITE_ON)) {
    
    return(-EINVAL);
  }

  // set it  
  status = bt865_set_blackwhite_mode(instance->bt865Instance, format);
  
  // if we get here, it is an error
  return(status);
}



/**
 *
 * Set the tv output pixel mode
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_PIXEL_*

 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_tv_pixel_mode(dxr2_t* instance, char* buffer)
{
  int format;
  int status;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(format, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check parameters
  if ((format < DXR2_PIXEL_CCIR601) || 
      (format > DXR2_PIXEL_SQUARE)) {
    
    return(-EINVAL);
  }

  // set it  
  status = bt865_set_pixel_mode(instance->bt865Instance, format);
  
  // if we get here, it is an error
  return(status);
}



/**
 *
 * Set the tv output 7.5 IRE mode
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_75IRE_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_tv_75IRE_mode(dxr2_t* instance, char* buffer)
{
  int format;
  int status;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(format, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check parameters
  if ((format < DXR2_75IRE_OFF) || 
      (format > DXR2_75IRE_ON)) {
    
    return(-EINVAL);
  }

  // set it  
  status = bt865_set_75IRE_mode(instance->bt865Instance, format);
  
  // if we get here, it is an error
  return(status);
}



/**
 *
 * Set the source video format
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_threeArg_t. 
 *               arg1 should be one of DXR2_SRC_VIDEO_FREQ_30, DXR2_SRC_VIDEO_FREQ_25
 *               arg2 should be x resolution
 *               arg3 should be y resolution
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_source_video_format(dxr2_t* instance, char* buffer)
{
  dxr2_threeArg_t* argBuffer = (dxr2_threeArg_t*) buffer;
  int freq;
  int xres;
  int yres;

  // get the frequency. if it's <0 => error
  if (get_user(freq, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if ((freq < DXR2_SRC_VIDEO_FREQ_30) || (freq > DXR2_SRC_VIDEO_FREQ_25)) {
    
    return(-EINVAL);
  }
  
  // get the x & y resolutions
  if (get_user(xres, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(yres, &(argBuffer->arg3)) < 0) {
    
    return(-EFAULT);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // store them
  instance->currentSourceVideoFrequency = freq;
  instance->currentSourceVideoXRes = xres;
  instance->currentSourceVideoYRes = yres;
  
  // set it on the ziva
  zivaDS_set_source_video_frequency(instance->zivaDSInstance, freq);
  
  // OK, reset the Ziva
  if (instance->currentBitstreamType == DXR2_BITSTREAM_TYPE_MPEG_VOB) {
    
    if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
      
      pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEON);
    }
    zivaDS_reset(instance->zivaDSInstance);
    if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
      
      pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEOFF);
    }
  }

  // set the bitstream type
  zivaDS_set_bitstream_type(instance->zivaDSInstance, instance->currentBitstreamType);
  
  // new play mode
  zivaDS_new_play_mode(instance->zivaDSInstance);

  // OK, set everything again
  if (instance->currentBitstreamType == DXR2_BITSTREAM_TYPE_MPEG_VOB) {

    // select subpicture stream
    zivaDS_select_stream(instance->zivaDSInstance, 
			 ZIVADS_STREAM_SUBPICTURE, instance->currentSubPictureStream);
    instance->hliFlag = 1;
    
    // select audio stream
    zivaDS_select_stream(instance->zivaDSInstance, 
			 instance->currentAudioStreamType,
			 instance->currentAudioStream);
  }

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);

  // OK
  return(0);
}

/**
 *
 * Get device capabilities. This doesn't actually do anything yet, since
 * I'm not quite sure what sort of things should be returned.
 *
 * @param instance Instance of the dxr2 to use
 * @param buffer buffer to put capabilities in.
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_get_capabilities(dxr2_t* instance, char* buffer)
{
  return(-EOPNOTSUPP);
}



/**
 *
 * Clear ziva video screen
 *
 * @param instance dxr2 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_clear_video(dxr2_t* instance)
{
  int status=0;

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // clear the video
  status = zivaDS_clear_video(instance->zivaDSInstance);
  
  // reenable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // OK
  return(status);
}



/**
 *
 * Pause the current video stream if not already paused or stopped
 *
 * @param instance DXR2 instance to use
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_pause(dxr2_t* instance)
{
  int status= 0;

  // are we already paused/stopped... if so, just return instantly
  if ((instance->currentPlayMode == DXR2_PLAYMODE_PAUSED) ||
      (instance->currentPlayMode == DXR2_PLAYMODE_STOPPED)) {
    
    return(0);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
  
    dxr2_queue_deferred(DXR2_QUEUE_PAUSED, 0, 0, 0);
  }
  else {
    
    status = zivaDS_pause(instance->zivaDSInstance);
  }
  
  // set the play mode
  instance->currentPlayMode = DXR2_PLAYMODE_PAUSED;
  
  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}


/**
 *
 * Set the audio volume
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be volume to set 
 *               (0<=arg<=19, where 0 is min volume)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_audio_volume(dxr2_t* instance, char* buffer)
{
  int status = 0;
  int volume;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the format. if it's <0 => error
  if (get_user(volume, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check parameters
  if ((volume < 0) || (volume > 19)) {
    
    return(-EINVAL);
  }
  
  // is audio muted?
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_ON) {
    
    return(0);
  }

  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
  
    dxr2_queue_deferred(DXR2_QUEUE_SETVOLUME, volume, 0, 0);
  }
  else {

    // OK, set the volume	
    status = zivaDS_set_audio_volume(instance->zivaDSInstance, volume);
  }

  // remember volume for later
  instance->currentAudioVolume = volume;
  
  // return status
  return(status);
}


/**
 *
 * Set the output aspect ratio
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_ASPECTRATIO_4_3 or DXR2_ASPECTRATIO_16_9
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_output_aspect_ratio(dxr2_t* instance, char* buffer)
{
  int aspect;
  int status = 0;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;

  // get the aspect. if it's <0 => error
  if (get_user(aspect, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check it
  if ((aspect < DXR2_ASPECTRATIO_4_3) || (aspect > DXR2_ASPECTRATIO_16_9 )) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);  

  // do it!
  status = zivaDS_set_output_aspect_ratio(instance->zivaDSInstance, aspect);

  // remember it
  instance->currentOutputAspectRatio = aspect;

  // enable the IRQ	
  DXR2_EXIT_CRITICAL(instance);  

  // return status	
  return(status);
}



/**
 *
 * Abort playback
 *
 * @param instance DXR2 instance to use
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_abort(dxr2_t* instance)
{
  int status=0;

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // issue the ABORT command
  status = zivaDS_abort(instance->zivaDSInstance);
  
  // flush the BM stuff
  vxp524_bm_flush(instance->vxp524Instance);

  // set the play mode
  instance->currentPlayMode = DXR2_PLAYMODE_STOPPED;
  
  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Stop playback
 *
 * @param instance DXR2 instance to use
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_stop(dxr2_t* instance)
{
  int status=0;

  // are we already stopped... if so, just return instantly
  if (instance->currentPlayMode == DXR2_PLAYMODE_STOPPED) {
    
    return(0);
  }

  // mute the DAC if it isn't already
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {

    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEON);
  }
  
  // issue the ABORT command
  status = zivaDS_abort(instance->zivaDSInstance);
  
  // flush the BM stuff
  vxp524_bm_flush(instance->vxp524Instance);

  // turn bit 4 of ASIC off
  dxr2_asic_set_bits(instance, 0x10, 0);

  // set the play mode
  instance->currentPlayMode = DXR2_PLAYMODE_STOPPED;
  
  // unmute the DAC, if it wasn't before
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {

    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEOFF);
  }
  
  // return status
  return(status);
}



/**
 * 
 * Enable subpicture
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg should be one of DXR2_SUBPICTURE_OFF or
 *               DXR2_SUBPICTURE_ON
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_enable_subpicture(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_SUBPICTURE_OFF) || (value > DXR2_SUBPICTURE_ON)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
  
    dxr2_queue_deferred(DXR2_QUEUE_ENABLESUBPICTURE, value, 0, 0);
  }
  else {

    // OK, enable it
    status = zivaDS_enable_subpicture(instance->zivaDSInstance, value);
  }

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}




/**
 *
 * Slow Forwards
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_PLAYRATE_2x, or DXR2_PLAYRATE_3x, 
 *               DXR2_PLAYRATE_4x, DXR2_PLAYRATE_5x, DXR2_PLAYRATE_6x
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_slow_forwards(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_PLAYRATE_2x) || (value > DXR2_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_slow_forwards(instance->zivaDSInstance, value);

  // remember values
  instance->currentSlowRate = value;
  instance->currentPlayMode = DXR2_PLAYMODE_SLOWFORWARDS;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Slow Backwards
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_PLAYRATE_2x, or DXR2_PLAYRATE_3x, 
 *               DXR2_PLAYRATE_4x, DXR2_PLAYRATE_5x, DXR2_PLAYRATE_6x
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_slow_backwards(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_PLAYRATE_2x) || (value > DXR2_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // OK, abort playback
  zivaDS_abort(instance->zivaDSInstance);

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_slow_backwards(instance->zivaDSInstance, value);

  // remember values
  instance->currentSlowRate = value;
  instance->currentPlayMode = DXR2_PLAYMODE_SLOWBACKWARDS;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Fast Forwards
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_PLAYRATE_2x, or DXR2_PLAYRATE_3x, 
 *               DXR2_PLAYRATE_4x, DXR2_PLAYRATE_5x, DXR2_PLAYRATE_6x
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_fast_forwards(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;

  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_PLAYRATE_2x) || (value > DXR2_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
  
    dxr2_queue_deferred(DXR2_QUEUE_FASTBACKWARDS, value, 0, 0);
  }
  else {
    
    status = zivaDS_fast_forwards(instance->zivaDSInstance, value);
  }
  
  // remember things
  instance->currentPlayMode = DXR2_PLAYMODE_FASTFORWARDS;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Fast Backwards
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_PLAYRATE_2x, or DXR2_PLAYRATE_3x, 
 *               DXR2_PLAYRATE_4x, DXR2_PLAYRATE_5x, DXR2_PLAYRATE_6x
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_fast_backwards(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_PLAYRATE_2x) || (value > DXR2_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // OK, abort playback
  zivaDS_abort(instance->zivaDSInstance);

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
  
    dxr2_queue_deferred(DXR2_QUEUE_FASTFORWARDS, value, 0, 0);
  }
  else {
    
    status = zivaDS_fast_backwards(instance->zivaDSInstance, value);
  }
  
  // remember things
  instance->currentPlayMode = DXR2_PLAYMODE_FASTBACKWARDS;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**

 *
 * Set source aspect ratio
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of DXR2_ASPECTRATIO_4_3, or DXR2_ASPECTRATIO_16_9
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_source_aspect_ratio(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_ASPECTRATIO_4_3) || (value > DXR2_ASPECTRATIO_16_9)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_set_source_aspect_ratio(instance->zivaDSInstance, value);

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 * 
 * Set aspect ratio mode
 *
 * @param instance DXR2 instance to use

 * @param buffer instance of dxr2_oneArg_t
 *                  arg is one of DXR2_ASPECTRATIOMODE_NORMAL,
 *                                DXR2_ASPECTRATIOMODE_PAN_SCAN,
 *                                DXR2_ASPECTRATIOMODE_LETTERBOX
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_aspect_ratio_mode(dxr2_t* instance, char* buffer)
{
  int status=0;
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get the value. if it's <0 => error
  if (get_user(value, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check param
  if ((value < DXR2_ASPECTRATIOMODE_NORMAL) || (value > DXR2_ASPECTRATIOMODE_LETTERBOX)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_set_aspect_ratio_mode(instance->zivaDSInstance, value);

  // remember it
  instance->currentAspectRatioMode = value;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Single step the picture
 *
 * @param instance DXR2 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_single_step(dxr2_t* instance)
{
  int status=0;


  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_single_step(instance->zivaDSInstance);

  // remember it
  instance->currentPlayMode = DXR2_PLAYMODE_SINGLESTEP;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);

  
  // return status
  return(status);
}



/**
 *
 * Reverse play
 *
 * @param instance DXR2 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_reverse_play(dxr2_t* instance)
{
  int status=0;


  // if we're playing currently, ABORT
  if (instance->currentPlayMode == DXR2_PLAYMODE_PLAY) {
    
    status = dxr2_ioc_abort(instance);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_reverse_play(instance->zivaDSInstance);

  // remember it
  instance->currentPlayMode = DXR2_PLAYMODE_REVERSEPLAY;

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Set the subpicture palette
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_palette_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_subpicture_palettes(dxr2_t* instance, char* buffer)
{
  int status=0;
  char* paletteData[16*4];
  //changed by zulli
//  int i;

  // copy the data from the user
  if (copy_from_user(paletteData, buffer, 16*4) > 0) {
    
    return(-EFAULT);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // OK, do it
  status = zivaDS_set_subpicture_palettes(instance->zivaDSInstance, (int*) paletteData);

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}


/**
 *
 * Get the challenge key from the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_challengeKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_get_challenge_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[10];
  int status = 0;
  dxr2_challengeKey_t* keyBuffer = (dxr2_challengeKey_t*) buffer;


  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_get_challenge_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_get_challenge_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EIO);
  }
  
  // if status != OK => return now
  if (status != 0) {
    
    return(status);
  }

  // OK, copy data to user
  if (copy_to_user(&(keyBuffer->key), tmpBuf, 10) > 0) {
    
    return(-EFAULT);
  }
  
  // OK
  return(0);
}



/**
 *
 * Send the challenge key to the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_challengeKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_send_challenge_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[10];
  int status = 0;
  dxr2_challengeKey_t* keyBuffer = (dxr2_challengeKey_t*) buffer;
  // changed by zulli
//  int i;

  // get data from user
  if (copy_from_user(tmpBuf, &(keyBuffer->key), 10) > 0) {
    
    return(-EFAULT);
  }

  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_send_challenge_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_send_challenge_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EIO);
  }

  // return status
  return(status);
}



/**
 *
 * Get the response key from the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_responseKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_get_response_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[5];
  int status = 0;
  dxr2_responseKey_t* keyBuffer = (dxr2_responseKey_t*) buffer;


  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_get_response_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_get_response_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EFAULT);
  }
  
  // if status != OK => return now
  if (status != 0) {
    
    return(status);
  }

  // OK, copy data to user
  if (copy_to_user(&(keyBuffer->key), tmpBuf, 5) > 0) {
    
    return(-EIO);
  }
  
  // OK
  return(0);
}



/**
 *
 * Send the response key to the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_responseKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_send_response_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[5];
  int status = 0;
  dxr2_responseKey_t* keyBuffer = (dxr2_responseKey_t*) buffer;


  // get data from user
  if (copy_from_user(tmpBuf, &(keyBuffer->key), 5) > 0) {
    
    return(-EFAULT);
  }

  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_send_response_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_send_response_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EIO);
  }

  // return status
  return(status);
}



/**
 *
 * Send the disc key to the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_discKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_send_disc_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[0x800];
  int status = 0;
  dxr2_discKey_t* keyBuffer = (dxr2_discKey_t*) buffer;


  // get data from user
  if (copy_from_user(tmpBuf, &(keyBuffer->key), 0x800) > 0) {
    
    return(-EFAULT);
  }

  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_send_disc_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = dxr2_zivaDS_send_disc_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EIO);
  }

  // return status
  return(status);
}



/**
 * UNSURE.... why the tc6807af has a 6 byte key, but the ziva has a 5 byte one (ignores byte0)
 * Send the title key to the DXR2
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_titleKey_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_send_title_key(dxr2_t* instance, char* buffer)
{
  unsigned char tmpBuf[6];
  int status = 0;
  dxr2_titleKey_t* keyBuffer = (dxr2_titleKey_t*) buffer;


  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:

    // get data from user
    if (copy_from_user(tmpBuf, &(keyBuffer->cgmsFlags), 6) > 0) {
      
      return(-EFAULT);
    }
    
    status = dxr2_tc6807af_send_title_key(instance->tc6807afInstance, tmpBuf);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    // get data from user (ignoring the CGMS flags)
    if (copy_from_user(tmpBuf, &(keyBuffer->key), 5) > 0) {
      
      return(-EFAULT);
    }

    status = zivaDS_send_title_key(instance->zivaDSInstance, tmpBuf);
    break;
    
  default:
    return(-EIO);
  }

  // return status
  return(status);
}



/**
 * UNSURE
 * Set decryption mode
 *
 * @param instance Instance of the DXR2 to use
 * @param buffer instance of dxr2_oneArg_t
 *               arg is one of DXR2_CSSDECRMODE_OFF, DXR2_CSSDECRMODE_ON
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_decryption_mode(dxr2_t* instance, char* buffer)
{
  int mode;
  int status = 0;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // get data from user
  if (get_user(mode, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check params
  if ((mode < DXR2_CSSDECRMODE_OFF) || (mode > DXR2_CSSDECRMODE_ON)) {
    
    return(-EINVAL);
  }

  // OK, do it
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_set_decryption_mode(instance->tc6807afInstance, mode);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_set_decryption_mode(instance->zivaDSInstance, mode);
    break;
    
  default:
    return(-EIO);
  }

  // return status
  return(status);
}



/**
 *
 * Load ucode into the Ziva & initialise it
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_uCode_t
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_init_zivaDS(dxr2_t* instance, char* buffer)
{
  int status = 0;
  char* tmpBuffer;
  int endTime;
  int bufLength;
  dxr2_uCode_t* argBuffer = (dxr2_uCode_t*) buffer;


  // OK, get buffer size
  if (get_user(bufLength, &(argBuffer->uCodeLength)) < 0) {
    
    return(-EFAULT);
  }

  // get some memory
  if ((tmpBuffer = (char*) vmalloc(bufLength)) == NULL) {
    
    return(-ENOMEM);
  }

  // get buffer from user
  copy_from_user(tmpBuffer, argBuffer + sizeof(int), bufLength);

  // mute the DAC
  pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEON);
  
  // reset bus mastering
  vxp524_bm_reset(instance->vxp524Instance);

  // some ASIC twiddling (Set bit 4)
  dxr2_asic_set_bits(instance, 0x10, 0x10);
  udelay(2);

  // disable auto-increment on ziva
  zivaDS_set_bits(instance->zivaDSInstance, ZIVADS_REGCONTROL, 4, 4);
  zivaDS_set_bits(instance->zivaDSInstance, ZIVADS_REGCONTROL, 4, 0);

  // some more ASIC twiddling
  dxr2_asic_set_bits(instance, 1, 0);
  dxr2_asic_set_bits(instance, 1, 1);
  dxr2_asic_set_bits(instance, 8, 0);
  dxr2_asic_set_bits(instance, 8, 8);
  dxr2_asic_set_bits(instance, 0x20, 0);
  dxr2_asic_set_bits(instance, 0x20, 0x20);

  // OK, call the Ziva setup routine
  if ((status = zivaDS_init(instance->zivaDSInstance, tmpBuffer)) < 0) {

    vfree(tmpBuffer);
    return(status);
  }
  
  // loop for 5 centisecs (ish)
  endTime = jiffies + ((5*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();
  }

  // set the ASIC to 0xC3
  dxr2_asic_set_reg(instance, 0xC3);

  // loop for 20 centisecs (ish)
  endTime = jiffies + ((20*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();
  }

  // set bits 3,4&5 of ASIC
  dxr2_asic_set_bits(instance, 0x38, 0x38);

  // unmute the DAC
  pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEOFF);

  // free the memory again
  vfree(tmpBuffer);
  
  // OK, ziva is now initialised
  instance->zivaDSInitialised = 1;

  // can now get zivaDS video and audio sizes
  instance->videoBufferSize = zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_VIDEO_EMPTINESS);
  instance->audioBufferSize = zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_AUDIO_EMPTINESS);  

  // return status
  return(status);
}
  



/**
 *
 * Set bt865 Macrovision mode
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t, arg = one of:
 *             DXR2_MACROVISION_*
 *
 */

extern int dxr2_ioc_set_tv_macrovision_mode(dxr2_t* instance, char* buffer)
{
  int mode;
  // changed by zulli
//  int status = 0;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(mode, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check parameters
  if ((mode < DXR2_MACROVISION_OFF) || (mode > DXR2_MACROVISION_AGC_4COLOURSTRIPE)) {
    
    return(-EINVAL);
  }
  
  // do it!
  return(bt865_set_macrovision_mode(instance->bt865Instance, mode));
}




/**
 *
 * Reset DXR2
 *
 * @param instance DXR2 instance to use
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_reset(dxr2_t* instance)
{
  int status=0;
  // changed by zulli
//  int tmp;
  // changed by zulli
//  dxr2_oneArg_t tmpParams;

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  //  reset
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
    
    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEON);
  }
  zivaDS_reset(instance->zivaDSInstance);
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
    
    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEOFF);
  }
  
  // flush the BM stuff
  vxp524_bm_reset(instance->vxp524Instance);

  // turn bit 4 of ASIC off
  dxr2_asic_set_bits(instance, 0x10, 0);

  // set the play mode
  instance->currentPlayMode = DXR2_PLAYMODE_STOPPED;

  // turn CSS off.
  switch(instance->zivaDSInstance->zivaDSType) {
  case ZIVADS_TYPE_1:
    
    status = dxr2_tc6807af_set_decryption_mode(instance->tc6807afInstance, TC6807AF_CSSDECRMODE_OFF);
    break;
    
  case ZIVADS_TYPE_2:
  case ZIVADS_TYPE_3:
  case ZIVADS_TYPE_4:

    status = zivaDS_set_decryption_mode(instance->zivaDSInstance, ZIVADS_CSSDECRMODE_OFF);
    break;
    
  default:
    return(-EIO);
  }
  
  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Set bitstream type
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t
 *          arg is one of DXR2_BITSTREAM_TYPE_MPEG_VOB,
 *                        DXR2_BITSTREAM_TYPE_CDROM_VCD, DXR2_BITSTREAM_TYPE_MPEG_VCD,
 *                        DXR2_BITSTREAM_TYPE_CDDA, DXR2_BITSTREAM_TYPE_4
 * 
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_bitstream_type(dxr2_t* instance, char* buffer)
{
  int status=0;
  // changed by zulli
//  int tmp;
  int type;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(type, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check params
  if ((type < DXR2_BITSTREAM_TYPE_MPEG_VOB) || (type > DXR2_BITSTREAM_TYPE_4)) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // set the aspect ratio mode to 1 for some reason.
  if (instance->currentBitstreamType == DXR2_BITSTREAM_TYPE_MPEG_VOB) {
    
    // only if the new bitstream is type MPEG_VCD or CDROM_VCD
    if ((type == DXR2_BITSTREAM_TYPE_MPEG_VCD) || (type == DXR2_BITSTREAM_TYPE_CDROM_VCD)) {

      // set the aspect ratio mode
      zivaDS_set_aspect_ratio_mode(instance->zivaDSInstance, DXR2_ASPECTRATIOMODE_NORMAL);
      instance->currentAspectRatioMode = DXR2_ASPECTRATIOMODE_NORMAL;
    }
  }

  // remember it
  instance->currentBitstreamType = type;

  // do it
  zivaDS_set_bitstream_type(instance->zivaDSInstance, type);

  // set source TV format
  zivaDS_set_source_video_frequency(instance->zivaDSInstance, instance->currentSourceVideoFrequency);
  
  // reset
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
    
    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEON);
  }
  zivaDS_reset(instance->zivaDSInstance);
  if (instance->currentAudioMuteStatus == DXR2_AUDIO_MUTE_OFF) {
    
    pcm1723_set_mute_mode(instance->pcm1723Instance, PCM1723_MUTEOFF);
  }

  // new play mode
  zivaDS_new_play_mode(instance->zivaDSInstance);
  
  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // return status
  return(status);
}



/**
 *
 * Play
 *
 * @param instance DXR2 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_play(dxr2_t* instance) 
{
  int playType=0;
  int playStarted =0;


  // if we're in scan mode, stop
  if ((instance->currentPlayMode = DXR2_PLAYMODE_FASTFORWARDS) ||
      (instance->currentPlayMode = DXR2_PLAYMODE_FASTBACKWARDS)) {
    
    dxr2_ioc_stop(instance);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // if we're already playing, but paused, or slow forwards...
  // just "continue play"
  if ((instance->currentPlayMode == DXR2_PLAYMODE_PAUSED) ||
      (instance->currentPlayMode == DXR2_PLAYMODE_SLOWFORWARDS)) {
    
    zivaDS_resume(instance->zivaDSInstance);
    playStarted = 0;
  }
  else {
    
    if ((instance->currentBitstreamType == DXR2_BITSTREAM_TYPE_CDROM_VCD) &&
	(instance->currentVideoStream > 0) &&
	(instance->currentVideoStream != 255)) {
      
      playType = ZIVADS_PLAYTYPE_STILLSTOP;
    }
    else {
      
      playType = ZIVADS_PLAYTYPE_NORMAL;
    }
    
    zivaDS_play(instance->zivaDSInstance, playType);
    playStarted = 1;
  }
   
  // setup ziva's audio DAC
  zivaDS_setup_audio_dac(instance->zivaDSInstance, instance->currentZivaAudioDACMode);

  // if play has just started, turn bit 4 of ASIC on
  if (playStarted) {
    
    dxr2_asic_set_bits(instance, 0x10, 0x10);
  }
  
  // set play mode
  instance->currentPlayMode = DXR2_PLAYMODE_PLAY;  

  // enable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // OK
  return(0);
}


/**
 * 
 * Get System Time Clock
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t, arg will be set to result
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_get_stc(dxr2_t* instance, char* buffer)
{
  // both changed by zulli
//  int mode;
//  int status = 0;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;

  
  // if we're using the TC6087AF and are in the middle of bus mastering...
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
    
    put_user(0, &(argBuffer->arg));
    return(0);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // if we're playing, get the pic_stc
  if (instance->currentPlayMode == DXR2_PLAYMODE_PLAY) {
    
    put_user(zivaDS_get_mrc_pic_stc(instance->zivaDSInstance), &(argBuffer->arg));
  }
  else {
    
    put_user(zivaDS_get_mrc_pic_pts(instance->zivaDSInstance), &(argBuffer->arg));
  }

  // reenable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // OK
  return(0);
}



/**
 *
 * Set audio sample frequency
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *    arg: DXR2_AUDIO_FREQ_441, DXR2_AUDIO_FREQ_96, DXR2_AUDIO_FREQ_24, 
 *         DXR2_AUDIO_FREQ_2205, DXR2_AUDIO_FREQ_32
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_audio_sample_freqency(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check 
  if ((instance->zivaDSInstance->zivaDSType >= ZIVADS_TYPE_3) && (param == DXR2_AUDIO_FREQ_96)) {
    
    param = DXR2_AUDIO_FREQ_48;
  }
  
  // remember it
  instance->currentAudioSampleFrequency = param;

  // do it & return status
  return(pcm1723_set_sample_frequency(instance->pcm1723Instance, param));
}



/**
 *
 * Set audio data width
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *   arg= DXR2_AUDIO_WIDTH_16, DXR2_AUDIO_WIDTH_24, DXR2_AUDIO_WIDTH_32
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_audio_data_width(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;

  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
    
  // check 
  if (instance->zivaDSInstance->zivaDSType >= ZIVADS_TYPE_3) {
    
    param = DXR2_AUDIO_WIDTH_16;
  }

  // remember it
  instance->currentAudioInputWidth = param;

  // do it & return
  return(pcm1723_set_input_width(instance->pcm1723Instance, param));
}



    
/**
 * 
 * Sets the IEC-958 output mode (either decoded AC3, or encoded AC3)
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *     arg= DXR2_IEC958_DECODED, DXR2_IEC958_ENCODED
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_iec958_output_mode(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check
  if ((param < DXR2_IEC958_DECODED) || (param > DXR2_IEC958_ENCODED)) {
    
    return(-EINVAL);
  }

  // do it & return
  return(zivaDS_set_iec958_output_mode(instance->zivaDSInstance, param));
}



/**
 * 
 * Set the AC3 mode... probably for Karaoke... Mmmmm... how useful ;)
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *   arg= DXR2_AC3MODE_LR_STEREO, DXR2_AC3MODE_LR_STEREO_PROLOGIC,
 *        DXR2_AC3MODE_LR_MONOR
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_AC3_mode(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }

  // check
  if ((param < DXR2_AC3MODE_LR_STEREO) || (param > DXR2_AC3MODE_LR_MONOR)) {
    
    return(-EINVAL);
  }

  // if it's not an AC3 audio stream => error
  if (instance->currentAudioStreamType != DXR2_STREAM_AUDIO_AC3) {
    
    return(-EINVAL);
  }

  // do it & return
  return(zivaDS_set_AC3_mode(instance->zivaDSInstance, 
			     instance->currentZivaAudioDACMode,
			     param));
}



/**
 *
 * Selects AC3 voice, either to NONE, or V1V2. This is for karaoke
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *         arg= DXR2_AC3VOICE_NONE, DXR2_AC3VOICE_V1V2
 *
 * @return 0 on success, <0 on failure
 *

 */

extern int dxr2_ioc_select_AC3_voice(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check
  if ((param < DXR2_AC3VOICE_NONE) || (param > DXR2_AC3VOICE_V1V2)) {
    
    return(-EINVAL);
  }

  // if it's not an AC3 audio stream => error
  if (instance->currentAudioStreamType != DXR2_STREAM_AUDIO_AC3) {
    
    return(-EINVAL);
  }

  // do it & return
  return(zivaDS_select_AC3_voice(instance->zivaDSInstance, param));
}




/**
 *
 * Mute/unmute audio
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *         arg= DXR2_AUDIO_MUTE_ON, DXR2_AUDIO_MUTE_OFF
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_audio_mute(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;
  int status = 0;

  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
    
  // check params
  if ((param != DXR2_AUDIO_MUTE_ON) && (param != DXR2_AUDIO_MUTE_OFF)) {
    
    return(-EINVAL);
  }

  // remember it
  instance->currentAudioMuteStatus = param;
    
  // if it's muted, turn audio off on ziva
  if (param == DXR2_AUDIO_MUTE_ON) {
    
    zivaDS_set_audio_attenuation(instance->zivaDSInstance, 0x60);
  }
  else { // otherwise, restore old volume
    
    // are we using the tc6807af?
    if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
	(instance->vxp524Instance->bmInUse)) {
      
      dxr2_queue_deferred(DXR2_QUEUE_SETVOLUME, instance->currentAudioVolume, 0, 0);
    }
    else {
      
      // OK,  set the volume	
      status = zivaDS_set_audio_volume(instance->zivaDSInstance, instance->currentAudioVolume);
    }
  }
  
  // now, deal with the pcm1723
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {
    
    dxr2_queue_deferred(instance, DXR2_QUEUE_SETMUTESTATUS, param, 0, 0);
  }
  else {
    
    pcm1723_set_mute_mode(instance->pcm1723Instance, param);
  }

  // OK
  return(status);
}




/**
 *
 * Set stereo mode
 *
 * @param instance DXR2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *    arg=DXR2_AUDIO_STEREO_NORMAL, DXR2_AUDIO_STEREO_MONOL, DXR2_AUDIO_STEREO_MONOR, 
 *        DXR2_AUDIO_STEREO_REVERSE
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_stereo_mode(dxr2_t* instance, char* buffer)
{
  int param;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get args
  if (get_user(param, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // check
  if ((param < DXR2_AUDIO_STEREO_NORMAL) || (param > DXR2_AUDIO_STEREO_REVERSE)) {
    
    return(-EINVAL);
  }

  // do it & return
  return(pcm1723_set_stereo_mode(instance->pcm1723Instance, param));
}




/**
 * FIXME... what to do with MPEG1 audio streams...
 * Select stream
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_twoArg_t.
 *     arg1 = stream type: DXR2_STREAM_VIDEO, DXR2_STREAM_SUBPICTURE,
 *                         DXR2_STREAM_AUDIO_AC3, DXR2_STREAM_AUDIO_MPEG, 
 *                         DXR2_STREAM_AUDIO_LPCM, DXR2_STREAM_AUDIO_5
 *     arg2 = stream to select
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_select_stream(dxr2_t* instance, char* buffer)
{
  int streamType;
  int stream;
  dxr2_twoArg_t* argBuffer = (dxr2_twoArg_t*) buffer;
  int tmp;
  int zivaClock;
  int pcmClock;


  // OK, get args
  if (get_user(streamType, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(stream, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  
  // check params
  if ((streamType < DXR2_STREAM_VIDEO) || 
      (streamType > DXR2_STREAM_AUDIO_5)) {
    
    return(-EINVAL);
  }

  // check stream max value
  if (stream != 255) {

    tmp = (instance->currentBitstreamType * 6) + streamType;
    if (stream >= maxStreamTable[tmp]) {
      
      return(-EINVAL);
    }
  }

  // if stream == 255 => stream = 0xffff
  if (stream == 255) {
    
    stream = 0xffff;
  }

  // remember values for later
  switch(streamType) {
  case DXR2_STREAM_VIDEO:
    
    instance->currentVideoStream = stream;
    break;
    
  case DXR2_STREAM_SUBPICTURE:

    instance->currentSubPictureStream = stream;
    break;

  case DXR2_STREAM_AUDIO_AC3:

    instance->currentAudioStream = stream;
    zivaDS_set_audio_clock_frequency(instance->zivaDSInstance, ZIVADS_CLKFREQ256);
    pcm1723_set_clock_frequency(instance->pcm1723Instance, PCM1723_CLKFREQ256);
    break;

  case DXR2_STREAM_AUDIO_MPEG:

    instance->currentAudioStream = stream;
    zivaDS_set_audio_clock_frequency(instance->zivaDSInstance, ZIVADS_CLKFREQ256);
    pcm1723_set_clock_frequency(instance->pcm1723Instance, PCM1723_CLKFREQ256);
    break;

  case DXR2_STREAM_AUDIO_LPCM:
    
    instance->currentAudioStream = stream;
    if (clockTable[instance->currentAudioInputWidth + (instance->currentAudioSampleFrequency*3)] == 1) {
      
      pcmClock = PCM1723_CLKFREQ256;
      zivaClock = ZIVADS_CLKFREQ256;
    }
    else {
      
      pcmClock = PCM1723_CLKFREQ384;
      zivaClock = ZIVADS_CLKFREQ384;
    }
    
    if ((instance->zivaDSInstance->zivaDSType < ZIVADS_TYPE_3) &&
	(instance->currentAudioSampleFrequency == DXR2_AUDIO_FREQ_96)) {
      
      pcmClock = PCM1723_CLKFREQ256;
      zivaClock = PCM1723_CLKFREQ384;
    }

    zivaDS_set_audio_clock_frequency(instance->zivaDSInstance, zivaClock);
    pcm1723_set_clock_frequency(instance->pcm1723Instance, pcmClock);
    break;

  case DXR2_STREAM_AUDIO_5:

    instance->currentAudioStream = stream;

    // FIXME... don't know what parameters to set here... these are a guess
    zivaDS_set_audio_clock_frequency(instance->zivaDSInstance, ZIVADS_CLKFREQ384);
    pcm1723_set_clock_frequency(instance->pcm1723Instance, PCM1723_CLKFREQ384);
    break;
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);

  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {

    dxr2_queue_deferred(DXR2_QUEUE_SELECTSTREAM, streamType, stream, 0, 0);
  }
  else {
    
    zivaDS_select_stream(instance->zivaDSInstance, streamType, stream);
  }


  // reenable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // OK
  return(0);
}


/**
 * FIXME.. don't know what all of these do
 * Highlight a button 
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_twoArg_t.
 *     arg1 = button to highlight (1-36), or DXR2_BUTTON_NONE,
 *        DXR2_BUTTON_UP, DXR2_BUTTON_DOWN, DXR2_BUTTON_LEFT, 
 *        DXR2_BUTTON_RIGHT
 *     arg2 = action to perform on button
 *        DXR2_BUTTONACTION_SELECT, DXR2_BUTTONACTION_UNHIGHLIGHT,
 *        DXR2_BUTTONACTION_ACTIVATE, DXR2_BUTTONACTION_ACTIVATE_SELECTED, 
 *        DXR2_BUTTONACTION_ (4-8)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_highlight(dxr2_t* instance, char* buffer)
{
  int buttonIndex;
  int action;
  dxr2_twoArg_t* argBuffer = (dxr2_twoArg_t*) buffer;
  int status = 0;


  // OK, get args
  if (get_user(buttonIndex, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(action, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  
  // check params
  if (action > DXR2_BUTTONACTION_8) {
    
    return(-EINVAL);
  }

  // disable the IRQ
  DXR2_ENTER_CRITICAL(instance);
  
  // wait till HLI int finished?
  if (instance->hliFlag) {
    
    zivaDS_wait_for_HLI_int(instance->zivaDSInstance);
    instance->hliFlag = 0;
  }

  // are we using the tc6807af?
  if ((instance->zivaDSInstance->zivaDSType == ZIVADS_TYPE_1) &&
      (instance->vxp524Instance->bmInUse)) {

    status = dxr2_queue_deferred(DXR2_QUEUE_HIGHLIGHT, buttonIndex, action, 0, 0);
  }
  else {
    
    status = zivaDS_highlight(instance->zivaDSInstance, buttonIndex, action);
  }

  // reenable the IRQ
  DXR2_EXIT_CRITICAL(instance);
  
  // OK
  return(status);
}


/**
 *
 * Set the overlay colour
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_sixArg_t.
 *   arg1= Lower bound for colour
 *   arg2= Upper bound for colour
 *   arg3= Lower bound for colour
 *   arg4= Upper bound for colour
 *   arg5= Lower bound for colour
 *   arg6= Upper bound for colour
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_colour(dxr2_t* instance, char* buffer)
{
  int params[6];


  // copy the parameters from the user
  if (copy_from_user(params, buffer, sizeof(int) * 6) > 0) {  

    return(-EFAULT);
  }
  
  // do it
  return(anp82_set_overlay_colour(instance->anp82Instance,
				  params[0], params[1],
				  params[2], params[3],
				  params[4], params[5]));
}


/**
 *
 * Set the colour gain
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_fourArg_t.
 *   arg1= Common gain (0 - 0x3f)
 *   arg2= red gain (0- 63)
 *   arg3= green gain (0- 63)
 *   arg4= blue gain (0- 63)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_gain(dxr2_t* instance, char* buffer)
{
  dxr2_fourArg_t* argBuffer = (dxr2_fourArg_t*) buffer;
  int common;
  int red;
  int green;
  int blue;


  // copy the parameters from the user
  if (get_user(common, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(red, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(green, &(argBuffer->arg3)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(blue, &(argBuffer->arg4)) < 0) {
    
    return(-EFAULT);
  }
  
  // do it
  return(anp82_set_gain(instance->anp82Instance,
			common, red, green, blue));
}



/**
 *
 * Sets the "in delay" value
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *   arg= In delay value (0-3)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_in_delay(dxr2_t* instance, char* buffer)
{
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // copy the parameters from the user
  if (get_user(value, &(argBuffer->arg)) < 0) {

    return(-EFAULT);
  }
  
  // do it
  return(anp82_set_in_delay(instance->anp82Instance, value));
}



/**
 *
 * Set overlay mode
 *
 * @param instance dxr2 instance
 * @param buffer instance of dxr2_oneArg_t.
 *    arg= One of DXR2_OVERLAY_*
 *
 */

extern int dxr2_ioc_set_overlay_mode(dxr2_t* instance, char* buffer)
{
  int value;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;
  int status=0;
  
  // copy the parameters from the user
  if (get_user(value, &(argBuffer->arg)) < 0) {

    return(-EFAULT);
  }
  
  // check 
  if ((value < DXR2_OVERLAY_DISABLED) ||
      (value > DXR2_OVERLAY_WINDOW_COLOUR_KEY)) {
    
    return(-EINVAL);
  }

  // set the anp82 overlay status
  if ((status = anp82_set_overlay_mode(instance->anp82Instance, value)) < 0) {

    // disable vxp524 overlay on error!
    anp82_set_overlay_mode(instance->anp82Instance, ANP82_OVERLAY_DISABLED);
    vxp524_set_overlay_mode(instance->vxp524Instance, VXP524_OVERLAY_DISABLED);
    return(status);
  }
  
  // set the vxp524 overlay status
  if (value == DXR2_OVERLAY_DISABLED) {

    status = vxp524_set_overlay_mode(instance->vxp524Instance, 
				     VXP524_OVERLAY_DISABLED);
  }
  else { 

    status = vxp524_set_overlay_mode(instance->vxp524Instance, 
				     VXP524_OVERLAY_ENABLED);
  }

  // return status
  return(status);
}



/**
 *
 * Sets the cropping values
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_fourArg_t.
 *    arg1= Left cropping in pixels
 *    arg2= Right cropping in pixels
 *    arg3= Top cropping in pixels
 *    arg4= Bottom cropping in pixels
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_cropping(dxr2_t* instance, char* buffer)
{
  int left;
  int right;
  int top;
  int bottom;
  dxr2_fourArg_t* argBuffer = (dxr2_fourArg_t*) buffer;


  // copy the parameters from the user
  if (get_user(left, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(right, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(top, &(argBuffer->arg3)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(bottom, &(argBuffer->arg4)) < 0) {
    
    return(-EFAULT);
  }

  // do it
  return(vxp524_set_overlay_cropping(instance->vxp524Instance,
				     left, right, top, bottom));
}



/**
 *
 * Sets the dimension of the overlay window
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_twoArg_t.
 *    arg1= Width in pixels
 *    arg1= Height in pixels
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_dimension(dxr2_t* instance, char* buffer)
{
  int width;
  int height;
  int status;
  dxr2_twoArg_t* argBuffer = (dxr2_twoArg_t*) buffer;

  // copy the parameters from the user
  if (get_user(width, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(height, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }

  // do it
  status = vxp524_set_overlay_dimension(instance->vxp524Instance,
					width, height);
  return(status);
}



/**
 *
 * Sets the overlay position
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_twoArg_t.
 *    arg1= Width in pixels
 *    arg1= Height in pixels
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_position(dxr2_t* instance, char* buffer)
{
  int xpos;
  int ypos;
  dxr2_twoArg_t* argBuffer = (dxr2_twoArg_t*) buffer;


  // copy the parameters from the user
  if (get_user(xpos, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(ypos, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }


  // do it
  return(vxp524_set_overlay_position(instance->vxp524Instance,
				     xpos, ypos));
}


/**
 *
 * Set the x ratio, for fine tuning display ratios
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *    arg= X-Ratio adjust (1-2500)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_ratio(dxr2_t* instance, char* buffer)
{
  int ratio;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;

  // copy the parameter from the user
  if (get_user(ratio, &(argBuffer->arg)) < 0) {
    
    return(-EFAULT);
  }
  
  // do it
  return(vxp524_set_overlay_ratio(instance->vxp524Instance, ratio));
}



/**
 *
 * Calculates necessary VGA parameters
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_vgaParams_t. The screenWidth parameter 
 *   should be initalised to the width of the screen in pixels
 *
 * @return 0 on success, or error if <0
 *
 */

extern int dxr2_ioc_calculate_vga_parameters(dxr2_t* instance, char* buffer)
{
  dxr2_vgaParams_t* vgaBuffer = (dxr2_vgaParams_t*) buffer;
  int hsyncPol;
  int vsyncPol;
  int hOffset;
  int vOffset;
  int blankStart;
  int blankWidth;
  int screenWidth;
  int hOffWinKey;
  int vOffWinKey;
  int ratio;


  // verify user supplied data buffer
  if (verify_area(VERIFY_WRITE, (void *) buffer, sizeof(dxr2_vgaParams_t)) < 0) {
    
    return(-EFAULT);
  }
  
  // get sync polarities
  hsyncPol = anp82_measure_hsync_polarity(instance->anp82Instance);
  vsyncPol = anp82_measure_vsync_polarity(instance->anp82Instance);
  put_user(hsyncPol, &(vgaBuffer->hsyncPol));
  put_user(vsyncPol, &(vgaBuffer->vsyncPol));

  // set sync polarities
  anp82_set_sync_polarities(instance->anp82Instance, hsyncPol, vsyncPol);

  // get video blank
  anp82_measure_video_blank(instance->anp82Instance, &blankStart, &blankWidth);
  put_user(blankStart, &(vgaBuffer->blankStart));
  put_user(blankWidth, &(vgaBuffer->blankWidth));

  // set video blank
  anp82_set_video_blank(instance->anp82Instance, blankStart, blankWidth);

  // get offsets
  get_user(hOffWinKey, &(vgaBuffer->hOffWinKey));
  get_user(vOffWinKey, &(vgaBuffer->vOffWinKey));
  hOffset = anp82_measure_horizontal_offset(instance->anp82Instance, hOffWinKey); 
  vOffset = anp82_measure_vertical_offset(instance->anp82Instance, vOffWinKey);

  put_user(hOffset, &(vgaBuffer->hOffset));
  put_user(vOffset, &(vgaBuffer->vOffset));

  // get horizontal ratio
  get_user(screenWidth, &(vgaBuffer->xScreen));
  ratio = anp82_measure_horizontal_ratio(instance->anp82Instance, screenWidth);
  put_user(ratio, &(vgaBuffer->ratio));

  // OK
  return(0);
}



/**
 *
 * Sets VGA parameters to supplied values
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t
 *               arg= Destination for timer value
 * 
 * @return timer 0 on succes, or <0 on failure
 *
 */

extern int dxr2_ioc_set_vga_parameters(dxr2_t* instance, char* buffer)
{
  dxr2_vgaParams_t* vgaBuffer = (dxr2_vgaParams_t*) buffer;
  int hsyncPol;
  int vsyncPol;
  int blankStart;
  int blankWidth;
  int hOffset;
  int vOffset;
  int ratio;
  int status = 0;


  // verify user supplied data buffer
  if (verify_area(VERIFY_READ, (void*) buffer, sizeof(dxr2_vgaParams_t)) < 0) {
    
    return(-EFAULT);
  }  
  
  // set sync polarities
  get_user(hsyncPol, &(vgaBuffer->hsyncPol));
  get_user(vsyncPol, &(vgaBuffer->vsyncPol));
  if ((status = anp82_set_sync_polarities(instance->anp82Instance, hsyncPol, vsyncPol)) < 0) {
    
    return(status);
  }
  
  // set video blank
  get_user(blankStart, &(vgaBuffer->blankStart));
  get_user(blankWidth, &(vgaBuffer->blankWidth));
  if ((status = anp82_set_video_blank(instance->anp82Instance, blankStart, blankWidth)) < 0) {
    
    return(status);
  }
  
  // set offsets
  get_user(hOffset, &(vgaBuffer->hOffset));
  get_user(vOffset, &(vgaBuffer->vOffset));

  if ((status = vxp524_set_overlay_offsets(instance->vxp524Instance, hOffset, vOffset)) < 0) {
    
    return(status);
  }

  // set horizontal ratio (also updates video parameters)
  get_user(ratio, &(vgaBuffer->ratio));
  if ((status = vxp524_set_overlay_ratio(instance->vxp524Instance, ratio)) < 0) {
    
    return(status);
  }

  // OK
  return(0);
}




/**
 *
 * Sets the overlay picture controls
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_fourArg_t.
 *    arg1= Gamma control value (0 to 96)
 *    arg2= Contrast control value (-128 to 127)
 *    arg3= Brightness control value (-128 to 127)
 *    arg4= Saturation control value (-128 to 127)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_set_overlay_picture_controls(dxr2_t* instance,
						 char* buffer)
{
  int gamma;
  int contrast;
  int brightness;
  int saturation;
  dxr2_fourArg_t* argBuffer = (dxr2_fourArg_t*) buffer;


  // copy the parameters from the user
  if (get_user(gamma, &(argBuffer->arg1)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(contrast, &(argBuffer->arg2)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(brightness, &(argBuffer->arg3)) < 0) {
    
    return(-EFAULT);
  }
  if (get_user(saturation, &(argBuffer->arg4)) < 0) {
    
    return(-EFAULT);
  }

  // do it
  return(vxp524_set_overlay_picture_controls(instance->vxp524Instance,
					     gamma, contrast,
					     brightness, saturation));
}



/**
 *
 * Checks if the dxr2's buffers are empty yte (i.e. stream data has
 * finished being processed)
 *
 * @param instance dxr2 instance to use
 * @param buffer instance of dxr2_oneArg_t.
 *    arg= Set to 1 if the buffers ARE empty, else 0
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_ioc_buffers_empty(dxr2_t* instance, char* buffer)
{

  int status;
  int vSize;
  int aSize;
  dxr2_oneArg_t* argBuffer = (dxr2_oneArg_t*) buffer;


  // OK, get the current buffer sizes
  vSize = zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_VIDEO_EMPTINESS);
  aSize = zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_AUDIO_EMPTINESS);  

  // set results appropriately
  status = 0;
  if ((vSize == instance->videoBufferSize) &&
      (aSize == instance->audioBufferSize)) {

    status = 1;
  }
  
  // return data to user
  put_user(status, &(argBuffer->arg));

  // OK!
  return(0);
}
