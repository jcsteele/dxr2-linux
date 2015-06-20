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
 * Driver for the C-Cube Ziva-DS MPEG decoder chip
 * High level functions.
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/byteorder/generic.h>
#include <zivaDS.h>



/**
 *
 * Table of attenuation values for zivaDS_setAudioVolume
 *
 */
static int volumeTable[] = {

  0x60, 0x20, 0x1c, 0x18, 0x16, 0x14, 0x12,
  0x10, 0x0e, 0x0c, 0x0a, 9, 8, 7, 6, 5, 4, 3, 2, 1
};


/**
 *
 * Table of values for slow forwards
 *
 */
static int slowForwardsTable[] = {
  
  4, 6, 8, 0xa, 0xc
};



/**
 *
 * Table of values for slow backwards
 *
 */
static int slowBackwardsTable[] = {
  
  4, 8, 0xa, 0xc, 0x14
};




/**
 *
 * Bitstream type table
 *
 */
static int bitStreamTypeTable[] = {
  
  0,2,1,0x11,0x10
};


/**
 *
 * This detects the Ziva chip. You can't just reset the thing, because
 * you need the firmware uploaded first (I think)... 
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_detect(zivaDS_t* instance) {
  
  // set register to 0xa5
  zivaDS_set_reg(instance, ZIVADS_REGADDRESS, 0xA5); // is this right?

  // try and read it again
  if (zivaDS_get_reg(instance, ZIVADS_REGADDRESS) != 0xA5) {
    
    return(-ENODEV);
  }
 
  // OK!
  return(0);
}



/**
 *
 * Get the CSS flags from the ziva and store them in supplied structure
 *
 * @param instance Ziva instance to use
 * @param flags Where to store the flags
 *
 */

extern void zivaDS_get_css_flags(zivaDS_t* instance, zivaDS_cssFlags_t* flags) 
{
  // OK, get these memory locations
  flags->flag1 = zivaDS_get_mem(instance, 0x80001F);
  flags->flag2 = zivaDS_get_mem(instance, 0x80001C);
}
  


/**
 *
 * Restore the CSS flags to the ziva from the supplied structure
 *
 * @param instance Ziva instance to use
 * @param flags Where to store the flags
 *
 */

extern void zivaDS_restore_css_flags(zivaDS_t* instance, zivaDS_cssFlags_t* flags) 
{
  // OK, restore 'em
  zivaDS_set_mem(instance, 0x80001F, flags->flag1);
  zivaDS_set_mem(instance, 0x80001C, flags->flag2);
}
  


/**
 *
 * Set the CSS mode... used for the tc6807af functions
 * You call this with mode=1 before doing anything with the tc6807af, and then with
 * mode=0 after you have finished.
 *
 * @param instance Ziva instance to use
 * @param mode Mode to set (0 or 1).
 *
 */

extern void zivaDS_set_css_mode(zivaDS_t* instance, int mode) 
{
  switch(mode) {
  case 0:
    
      zivaDS_set_mem(instance, 0x80001C, 8);
      break;
      
  case 1:

    zivaDS_set_mem(instance, 0x80001F, 3);
    zivaDS_set_mem(instance, 0x80001C, 0xC);
    break;
  }
}



/**
 *
 * Enable/disable subpicture
 *
 * @param instance instance to use
 * @param flag one of ZIVADS_SUBPICTURE_OFF, ZIVADS_SUBPICTURE_ON
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_enable_subpicture(zivaDS_t* instance, int flag) 
{
  if ((flag < ZIVADS_SUBPICTURE_OFF) || (flag > ZIVADS_SUBPICTURE_ON)) {
    
    return(-EINVAL);
  }

  // do it
  zivaDS_set_mem(instance, ZIVADS_ENABLE_SUBPICTURE, flag);
  
  // OK
  return(0);
}



/**
 *
 * Abort playback
 *
 * @param instance zivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_abort(zivaDS_t* instance) 
{
  return(zivaDS_command(instance, ZIVADS_CMD_ABORT,
			0,2,0,0,0,0,
			0x200,4));
}



/**
 *
 * Set the output aspect ratio
 *
 * @param instance zivaDS instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of ZIVADS_ASPECTRATIO_4_3 or ZIVADS_ASPECTRATIO_16_9
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_output_aspect_ratio(zivaDS_t* instance, int ratio)
{

  // check parameters
  if ((ratio != ZIVADS_ASPECTRATIO_4_3) &&
      (ratio != ZIVADS_ASPECTRATIO_16_9)) {
    
    return(-EINVAL);
  }

  // set it!
  zivaDS_set_mem(instance, ZIVADS_DISPLAY_ASPECT_RATIO, ratio);

  // OK
  return(0);
}


/**
 *
 * Set source aspect ratio
 *
 * @param instance ZivaDS instance
 * @param ratio one of ZIVADS_ASPECTRATIO_4_3, ZIVADS_ASPECTRATIO_16_9
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_source_aspect_ratio(zivaDS_t* instance, int ratio) 
{
  int value;

  // check parameters
  switch(ratio) {
  case ZIVADS_ASPECTRATIO_4_3:
    
    value = 0;
    break;
    
  case ZIVADS_ASPECTRATIO_16_9:
    
    value = 3;
    break;

  default:

    return(-EINVAL);
  }

  // set it!
  zivaDS_set_mem(instance, ZIVADS_SOURCE_ASPECT_RATIO, value);
  
  // OK
  return(0);
}


/**
 *
 * Set audio volume
 *
 * @param instance ZivaDS instance to use
 * @param volume Volume to to (0-19), 0 =min, 19=max
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_audio_volume(zivaDS_t* instance, int volume) 
{
  int value;
  
  // check parameters
  if ((volume < 0) || (volume > 19)) {
    
    return(-EINVAL);
  }

  // OK, work out REAL value
  value = volumeTable[volume];

  // set it!
  zivaDS_set_mem(instance, ZIVADS_AUDIO_ATTENUATION, value);
  
  // OK
  return(0);
}


/**
 *
 * Pause playback
 *
 * @param instance ZivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_pause(zivaDS_t* instance)
{
  // pause, with repeated even field
  return(zivaDS_command(instance, 
			ZIVADS_CMD_PAUSE,
			1,0,0,0,0,0,
			0x2000, 4));
}


/**
 *
 * Clear video
 *
 * @param instance ZivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_clear_video(zivaDS_t* instance) 
{
  // fill screen with colour 0x108080
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SETFILL,
			0xffffffff,0,0,0,0x108080,0,
			0, 0));
}



/**
 *
 * Slow forwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_slow_forwards(zivaDS_t* instance, int rate) 
{
  int value;

  // check parameters
  if ((rate < ZIVADS_PLAYRATE_2x) || (rate > ZIVADS_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // OK, work out REAL value
  value = slowForwardsTable[rate];
  
  // slow forwards, with repeated even field
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SLOWFORWARDS,
			value,1,0,0,0,0,
			0x100000, 0));
}



/**
 *
 * Slow backwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_slow_backwards(zivaDS_t* instance, int rate) 
{
  int value;

  // check parameters
  if ((rate < ZIVADS_PLAYRATE_2x) || (rate > ZIVADS_PLAYRATE_6x)) {
    
    return(-EINVAL);
  }

  // OK, work out REAL value
  value = slowBackwardsTable[rate];
  
  // do it!
  return(zivaDS_command(instance, 
			ZIVADS_CMD_BACKWARDS,
			value,3,4,0,0,0,
			0x100000, 0));
}



/**
 * 
 * Set aspect ratio mode
 *
 * @param instance ZivaDS instance to use
 * @param mode Aspect ratio mode (one of ZIVADS_ASPECTRATIOMODE_NORMAL,
 *                                ZIVADS_ASPECTRATIOMODE_PAN_SCAN,
 *                                ZIVADS_ASPECTRATIOMODE_LETTERBOX)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_aspect_ratio_mode(zivaDS_t* instance, int mode)
{
  // check parameters
  if ((mode < ZIVADS_ASPECTRATIOMODE_NORMAL) || 
      (mode > ZIVADS_ASPECTRATIOMODE_LETTERBOX)) {
    
    return(-EINVAL);
  }
  
  // set it!
  zivaDS_set_mem(instance, ZIVADS_ASPECT_RATIO_MODE, mode);
  
  // OK
  return(0);
}



/**
 *
 * Fast forwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_fast_forwards(zivaDS_t* instance, int rate) 
{
  int scanmode;
  int skip;

  // work out real values
  switch(rate) {    
  case ZIVADS_PLAYRATE_2x:
    
    scanmode = 1;
    skip = 0;
    break;

  case ZIVADS_PLAYRATE_3x:
    
    scanmode = 0;
    skip = 0;
    break;

  case ZIVADS_PLAYRATE_4x:
    
    scanmode = 0;
    skip = 1;
    break;

  case ZIVADS_PLAYRATE_5x:
    
    scanmode = 0;
    skip = 2;
    break;

  case ZIVADS_PLAYRATE_6x:
    
    scanmode = 0;
    skip = 6;
    break;
    
  default:
    
    return(-EINVAL);    
  }
  
  // slow forwards, with repeated even field
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SCAN,
			scanmode,skip,4,0,0,0,
			0x100000, 0));
}



/**
 *
 * Fast backwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_fast_backwards(zivaDS_t* instance, int rate) 
{
  int scanmode;
  int skip;


  // work out real values
  switch(rate) {    
  case ZIVADS_PLAYRATE_2x:
    
    scanmode = 1;
    skip = 0;
    break;

  case ZIVADS_PLAYRATE_3x:
    
    scanmode = 0;
    skip = 0;
    break;

  case ZIVADS_PLAYRATE_4x:
    
    scanmode = 0;
    skip = 1;
    break;

  case ZIVADS_PLAYRATE_5x:
    
    scanmode = 0;
    skip = 2;
    break;

  case ZIVADS_PLAYRATE_6x:
    
    scanmode = 0;
    skip = 6;
    break;
    
  default:
    
    return(-EINVAL);    
  }

  // do it!
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SCAN,
			scanmode,skip,4,0,0,0,
			0x100000, 0));
}


/**
 * 
 * Single Step
 *
 * @param instance ZivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_single_step(zivaDS_t* instance)
{
  // do it!
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SINGLESTEP,
			4,0,0,0,0,0,
			0x100000, 0));
}



/**
 * 
 * Reverse play
 *
 * @param instance ZivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_reverse_play(zivaDS_t* instance)
{
  // do it!
  return(zivaDS_command(instance, 
			ZIVADS_CMD_BACKWARDS,
			2,3,4,0,0,0,
			0x100000, 0));
}



/**
 * 
 * Set subpicture palettes
 *
 * @param instance ZivaDS instance to use	
 * @param palette (16x32bit palette entries)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_subpicture_palettes(zivaDS_t* instance, u32 palettes[])
{
  int palStart;
  int i;

  // get start of palette data in ziva memory
  palStart = zivaDS_get_mem(instance, ZIVADS_SUBPICTURE_PALETTE_START);
  
  // write out each palette entry
  for(i=0; i< 16; i++) {
    
    zivaDS_set_mem(instance, palStart + (i*4), palettes[i]);
  }
  
  // inform Ziva that new palette data is present
  zivaDS_set_mem(instance, ZIVADS_NEW_SUBPICTURE_PALETTE, 1);
  
  // OK
  return(0);
}



/**
 *
 * Initialise the ziva
 *
 * @param instance instance to use
 * @param uCode ucode data to upload
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_init(zivaDS_t* instance, char* uCode)
{
  u32 dataLength;
  u32 destAddress;
  u32 tmpLength;
  u32 tmpAddress;
  u32 tmpValue;
  int i;
  int loopFlag;
  int endTime;
  char* oldUcode;
  char* tmpUcode;
  char* block1End;
  
  // skip over file header
  uCode += 4 + 12;
  
  // OK, get the data length & address
  dataLength = le32_to_cpup(uCode); uCode +=4;
  destAddress = le32_to_cpup(uCode); uCode +=4;
  uCode += 4; // ignore the next 4 bytes
  
  // keep note of where we are
  oldUcode = uCode;
  
  // set host control register to ziva normal mode
  zivaDS_set_mem(instance, 0x800000, 0x1000);

  // Don't know what these do.... set some internal register shite
  zivaDS_set_mem(instance, 0x800022, 0xf);
  zivaDS_set_mem(instance, 0x800023, 0x14ec);
  zivaDS_set_mem(instance, 0x800022, 0x11);
  zivaDS_set_mem(instance, 0x800023, 0);
  
  // OK, skip over 0xBFC bytes
  tmpUcode = oldUcode + 0xBFC;
  
  // get length of data to write
  tmpLength = be32_to_cpup((u32*) tmpUcode); tmpUcode-=4;

  // OK, write out data from this table
  while(tmpLength > 0) {

    // get address & value
    tmpAddress = be32_to_cpup((u32*) tmpUcode); tmpUcode -=4;
    tmpValue = be32_to_cpup((u32*) tmpUcode); tmpUcode -=4;
    
    // output them
    zivaDS_set_mem(instance, tmpAddress, tmpValue);
    
    // decrement counter
    tmpLength--;
  }

  // remember where we are
  block1End = tmpUcode;
  
  // OK, move to next table start
  tmpUcode = oldUcode + 0x800;
  
  // write out table number 2
  for(i=0; i<255; i++) {

    // get value
    tmpValue = be32_to_cpup((u32*) tmpUcode); tmpUcode+=4;

    // output them... dunno what this is 
    zivaDS_set_mem(instance, 0x800036, i);
    zivaDS_set_mem(instance, 0x800034, tmpValue);
  }

  // ok, back to start of ucode
  tmpUcode = oldUcode;
  
  // write out all the Ucode now.
  for(i=0; i< dataLength; i+=4) {
    
    // get value
    tmpValue = be32_to_cpup((u32*) tmpUcode); tmpUcode += 4;
    
    // output it
    zivaDS_set_mem(instance, i, tmpValue);
  }
  
  // letterbox mode
  zivaDS_set_mem(instance, ZIVADS_ASPECT_RATIO_MODE, 
		 ZIVADS_ASPECTRATIOMODE_LETTERBOX);
  
  // setup bitstream source & SD mode
  if (instance->zivaDSType == ZIVADS_TYPE_1) {
    
    // not sure what this is, actually... assume it means bitstream
    // comes from host
    zivaDS_set_mem(instance, ZIVADS_BITSTREAM_SOURCE, 0);
    zivaDS_set_mem(instance, ZIVADS_SD_MODE, 0x0d);
  }
  else {

    // bitstream comes from host
    zivaDS_set_mem(instance, ZIVADS_BITSTREAM_SOURCE, 2);

    // DVD REQ bit/REQ pin is active LOW.
    zivaDS_set_mem(instance, ZIVADS_SD_MODE, 0x08);
  }

  // dunno what this does
  zivaDS_set_mem(instance, 0x214, 0xF0);

  // setup the ziva's memory layout.. I assume
  if (instance->zivaDSType == ZIVADS_TYPE_4) {
    
    zivaDS_set_mem(instance, ZIVADS_DRAM_INFO, 0);
    zivaDS_set_mem(instance, ZIVADS_UCODE_MEMORY, 0);
    zivaDS_set_mem(instance, ZIVADS_MEMORY_MAP, 1);
  }
  else {

    // OK, there are 20 MBits of DRAM there
    zivaDS_set_mem(instance, ZIVADS_DRAM_INFO, 1);

    // uCode is in DRAM
    zivaDS_set_mem(instance, ZIVADS_UCODE_MEMORY, 0);
    
    // 20 Mbit DRAM with NTSC memory map.
    zivaDS_set_mem(instance, ZIVADS_MEMORY_MAP, 3);
  }

  // AC3 output mode = 2 forward speakers, 0 rear, L/R
  zivaDS_set_mem(instance, ZIVADS_AC3_OUTPUT_MODE, 0);
  
  // ziva Type = Ziva-DS (as opposed to Ziva-D6)
  zivaDS_set_mem(instance, ZIVADS_IC_TYPE, 1);
  
  // frame based error recovery
  zivaDS_set_mem(instance, ZIVADS_ERR_CONCEALMENT_LEVEL, 0);
  
  // enable ALL host interrupts
  zivaDS_set_mem(instance, ZIVADS_INT_MASK, 0xffffff);
  
  // Ziva is Sync master
  zivaDS_set_mem(instance, ZIVADS_VIDEO_MODE, 2);

  // dunno what this does
  zivaDS_set_mem(instance, 0x80003a, 0x0a);
  zivaDS_set_mem(instance, 0x80003b, 0xc000);
  zivaDS_set_mem(instance, 0x80003a, 0x9);
  zivaDS_set_mem(instance, 0x80003b, 0x0);

  // more ucode writes
  tmpUcode = block1End;
  tmpLength = be32_to_cpup((u32*) tmpUcode); tmpUcode-=4;
  while(tmpLength > 0) {
    
    // get address & value
    tmpAddress = be32_to_cpup((u32*) tmpUcode); tmpUcode -=4;
    tmpAddress |= 0x800000;
    tmpValue = be32_to_cpup((u32*) tmpUcode); tmpUcode -=4;
    
    // output them
    zivaDS_set_mem(instance, tmpAddress, tmpValue);
    
    // decrement counter
    tmpLength--;
  }

  // loop for 50 centisecs (ish)
  loopFlag = 0;
  endTime = jiffies + ((50*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // check ziva state. exit if ziva is in IDLE state
    if (zivaDS_get_mem(instance, ZIVADS_PROC_STATE) == 2) {
      
      loopFlag =1;
      break;
    }
  }

  // enable HLI interrupt
  zivaDS_set_mem(instance, ZIVADS_INT_MASK, 0x80000);
  
  // don't know what this does
  zivaDS_set_mem(instance, 0x204, 0x4);

  // audio config
  if (instance->zivaDSType == ZIVADS_TYPE_1) {
    
    // various audio parameters:
    // output is encoded, IEC-958 on, L/R channels on, I2S output on L/R,
    // output = 48Fs when input = 96Fs
    zivaDS_set_mem(instance, ZIVADS_AUDIO_CONFIG, 0x2E);
  }
  else {

    zivaDS_set_mem(instance, ZIVADS_AUDIO_CONFIG, 0x0E);    
  }

  // indicate audio config changed
  zivaDS_set_mem(instance, ZIVADS_NEW_AUDIO_CONFIG, 1);

  // zetup ziva audio DAC mode:
  // MSB transmitted first on L/R, LRCK is high during L/R output
  i = zivaDS_get_mem(instance, ZIVADS_AUDIO_DAC_MODE);
  zivaDS_set_mem(instance, ZIVADS_AUDIO_DAC_MODE, i & 0xf4);

  // setup ziva audio clock: frequency division = 256Fs
  zivaDS_set_mem(instance, ZIVADS_AUDIO_CLOCK_SELECTION, 2);

  // did it time out?
  if (!loopFlag) {
    
    return(-ETIMEDOUT);
  }

  // OK
  return(0);
}



/**
 *
 * Resume play (e.g. if the device is paused)
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_resume(zivaDS_t* instance)
{
  return(zivaDS_command(instance, 
			ZIVADS_CMD_RESUME, 
			1,0,0,0,0,0,
			0x100000,0));
}



/**
 *
 * Start play (e.g. if the device is stopped)
 *
 * @param instance instance to use
 * @param playType ZIVADS_PLAYTYPE_NORMAL, ZIVADS_PLAYTYPE_STILLSTOP
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_play(zivaDS_t* instance, int playType)
{
  return(zivaDS_command(instance, 
			ZIVADS_CMD_PLAY, 
			playType+1,0,0,0,0,0,
			0x100000,0));
}


/**
 *
 * Select stream
 *
 * @param streamType Type of stream. One of ZIVADS_STREAM_VIDEO, 
 *                   ZIVADS_STREAM_SUBPICTURE, ZIVADS_STREAM_AUDIO_AC3, 
 *                   ZIVADS_STREAM_AUDIO_MPEG, ZIVADS_STREAM_AUDIO_LPCM,
 *                   ZIVADS_STREAM_AUDIO_5
 * @param streamNumber Stream of that type to select
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_select_stream(zivaDS_t* instance, 
				int streamType, int streamNumber)
{
  if ((streamType < ZIVADS_STREAM_VIDEO) || 
      (streamType > ZIVADS_STREAM_AUDIO_5)) {
    
    return(-EINVAL);
  }
  
  // do it
  return(zivaDS_command(instance, 
			ZIVADS_CMD_SELECTSTREAM,
			streamType, streamNumber, 0,0,0,0,
			0,4));
}



/**
 *
 * Reset the Ziva
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_reset(zivaDS_t* instance)
{
  return(zivaDS_command(instance, 
			ZIVADS_CMD_RESET,
			0,0,0,0,0,0,
			0x800000,4));
}



/**
 *
 * Performs a new play mode command on the ziva
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_new_play_mode(zivaDS_t* instance)
{
  return(zivaDS_command(instance, 
			ZIVADS_CMD_NEWPLAYMODE, 
			0,0,0,0,0,0,
			0,4));
}



/**
 *
 * Sets the bitstream type
 *
 * @param instance instance to use
 * @param type Bitstream type, one of ZIVADS_BITSTREAM_TYPE_MPEG_VOB,
 * ZIVADS_BITSTREAM_TYPE_CDROM_VCD, ZIVADS_BITSTREAM_TYPE_MPEG_VCD, 
 * ZIVADS_BITSTREAM_TYPE_CDDA, ZIVADS_BITSTREAM_TYPE_4
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_bitstream_type(zivaDS_t* instance, int type)
{
  int value;


  // check parameters
  if ((type < ZIVADS_BITSTREAM_TYPE_MPEG_VOB) || 
      (type > ZIVADS_BITSTREAM_TYPE_4)) {
    
    return(-EINVAL);
  }
  
  // work out REAL value
  value = bitStreamTypeTable[type];

  // do it
  zivaDS_set_mem(instance, ZIVADS_BITSTREAM_TYPE, value);
  
  // OK
  return(0);
}




/**
 *
 * Sets the source video frequency
 *
 * @param instance instance to use
 * @param freq ZIVADS_SRC_VIDEO_FREQ_30, ZIVADS_SRC_VIDEO_FREQ_25
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_source_video_frequency(zivaDS_t* instance, int freq)
{
  // check parameters
  if ((freq < ZIVADS_SRC_VIDEO_FREQ_30) || 
      (freq > ZIVADS_SRC_VIDEO_FREQ_25)) {
    
    return(-EINVAL);
  }

  // do it
  zivaDS_set_mem(instance, ZIVADS_SOURCE_TV_FORMAT, freq+1);
  
  // OK
  return(0);
}



/**
 *
 * Setup audio DAC.
 *
 * @param instance ZivaDS instance to use
 * @param dacMode Current Audio DAC mode
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_setup_audio_dac(zivaDS_t* instance,
				  int dacMode)
{

  int tmp;

  // get the AC3 output mode
  tmp = zivaDS_get_mem(instance, ZIVADS_AC3_OUTPUT_MODE);
  if (tmp == 1) { // i.e. AC3 is on 1/0, C
    
    // output Right data on both channels
    zivaDS_set_mem(instance, ZIVADS_AUDIO_DAC_MODE, dacMode | 0x20);
  }
  else {

    // leave it as it was
    zivaDS_set_mem(instance, ZIVADS_AUDIO_DAC_MODE, dacMode);
  }

  // OK, new audio config present
  zivaDS_set_mem(instance, ZIVADS_NEW_AUDIO_CONFIG, 1);
  
  // OK
  return(0);
}


/**
 *
 * Get the most significant 32 bits of the STC value
 * this is the System time..
 *
 * @param instance instance to use
 *
 * @return Value on success, <0 on failure
 *
 */

extern int zivaDS_get_mrc_pic_stc(zivaDS_t* instance)
{
  return(zivaDS_get_mem(instance, ZIVADS_MRC_PIC_STC));
}



/**
 * 
 * Get the most significant 32 bits of the PTS value
 * this is the presentation time..
 *
 * @param instance instance to use
 *
 * @return Value on success, <0 on failure
 *
 */

extern int zivaDS_get_mrc_pic_pts(zivaDS_t* instance)
{
  return(zivaDS_get_mem(instance, ZIVADS_MRC_PIC_PTS));
}


/**
 * 
 * Sets the IEC-958 output mode (either decoded AC3, or encoded AC3)
 *
 * @param instance instance to use
 * @param flag (ZIVADS_IEC958_DECODED, ZIVADS_IEC958_ENCODED)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_iec958_output_mode(zivaDS_t* instance, int flag)
{
  int tmp;


  // get old value & set flags
  tmp = zivaDS_get_mem(instance, ZIVADS_AUDIO_CONFIG);
  if (flag == ZIVADS_IEC958_DECODED) {
    
    // IEC-958 off, output decoded audio on IEC-958 channel
    tmp = (tmp & 0xfffffffd) | 1;
  }
  else {
    
    // IEC-958 on, output is encoded
    tmp = (tmp | 2) & 0xfffffffe;
  }

  // set it
  zivaDS_set_mem(instance, ZIVADS_AUDIO_CONFIG, tmp);
  
  // indicate new audio config
  zivaDS_set_mem(instance, ZIVADS_NEW_AUDIO_CONFIG, 1);

  // OK
  return(0);
}
  
  
/**
 * 
 * Set the AC3 mode... probably for Karaoke... Mmmmm... how useful ;)
 *
 * @param instance instance to use
 * @param dacMode Current Audio DAC mode
 * @param param One of ZIVADS_AC3MODE_LR_STEREO, 
 *                     ZIVADS_AC3MODE_LR_STEREO_PROLOGIC,
 *                     ZIVADS_AC3MODE_LR_MONOR
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_AC3_mode(zivaDS_t* instance, 
			       int dacMode,
			       int param)
{
  int ac3Mode;

  switch(param) {
  case ZIVADS_AC3MODE_LR_STEREO:
    
    // 2/0, L/R
    ac3Mode = 2;
    break;
    
  case ZIVADS_AC3MODE_LR_STEREO_PROLOGIC:
      
    // 2/0, L/R (Dolby ProLogic)
    ac3Mode = 0;
    break;
    
  case ZIVADS_AC3MODE_LR_MONOR:
    
    // output right data on both channels
    dacMode |= 0x20;

    // 2/0, L/R
    ac3Mode = 2;
    break;
      
  default:
    
    return(-EINVAL);
  }
  
  // OK, set DAC mode
  zivaDS_set_mem(instance, ZIVADS_AUDIO_DAC_MODE, dacMode);
  zivaDS_set_mem(instance, ZIVADS_NEW_AUDIO_CONFIG, 1);

  // set AC3 mode
  zivaDS_set_mem(instance, ZIVADS_AC3_OUTPUT_MODE, ac3Mode);
  
  // ok
  return(0);
}



/**
 * 
 * Selects AC3 voice, either to NONE, or V1V2. This is for karaoke
 *
 * @param instance instance to use
 * @param voice Voice to select (ZIVADS_AC3VOICE_NONE, ZIVADS_AC3VOICE_V1V2)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_select_AC3_voice(zivaDS_t* instance, int voice)
{
  int ac3Voice;

  // what should we be doing?
  if (voice == ZIVADS_AC3VOICE_NONE) {
    
    ac3Voice = 4;
  }
  else {
    
    ac3Voice = 3;
  }
  
  // set it
  zivaDS_set_mem(instance, ZIVADS_AC3_VOICE_SELECT, ac3Voice);

  // OK
  return(0);
}


/**
 *
 * Set the audio attenuation
 *
 * @param instance instance to use
 * @param attenuation The attenuation
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_audio_attenuation(zivaDS_t* instance, int attenuation) 
{
  // do it
  zivaDS_set_mem(instance, ZIVADS_AUDIO_ATTENUATION, attenuation);

  // OK
  return(0);
}


/**
 *
 * Highlights a button
 *
 * @param instance zivaDS instance
 * @param button Index of button to highlight (1-36), or ZIVADS_BUTTON_NONE,
 *     ZIVADS_BUTTON_UP, ZIVADS_BUTTON_DOWN, ZIVADS_BUTTON_LEFT, 
 *     ZIVADS_BUTTON_RIGHT
 * @param action to perform on button:
 *     ZIVADS_BUTTONACTION_SELECT, ZIVADS_BUTTONACTION_UNHIGHLIGHT,
 *     ZIVADS_BUTTONACTION_ACTIVATE, ZIVADS_BUTTONACTION_ACTIVATE_SELECTED
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_highlight(zivaDS_t* instance, int button, int action)
{
  return(zivaDS_command(instance, ZIVADS_CMD_HIGHLIGHT,
			button, action, 0,0,0,0,
			0,0));
}


/**
 *
 * Waits for the current HLI interrupt to finish??
 *
 * @param instance zivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_wait_for_HLI_int(zivaDS_t* instance) 
{
  zivaDS_int_src_t intSrcs;
  // changed by zulli
//  int status;
  int endTime;
  int loopFlag;

  // loop for 3 centisecs (ish)
  loopFlag =0;
  endTime = jiffies + ((3*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // check ziva state
    if (zivaDS_get_int_status(instance, &intSrcs) & 0x80000) {
      
      if (intSrcs.HLI_int_src == 0xff) {
	
	loopFlag = 1;
	break;
      }
      break;
    }
  }
  
  // did it time out?
  if (!loopFlag) {
    
    return(-ETIMEDOUT);
  }
  
  // OK
  return(0);
}
  

/**
 *
 * Sets the ziva Audio clock frequency
 *
 * @param instance ZivaDS instance
 * @param freq Frequency to set (ZIVADS_CLKFRE256 or ZIVADS_CLKFREQ384)
 *
 */

extern int zivaDS_set_audio_clock_frequency(zivaDS_t* instance, int freq) 
{
  int value;

  // get values
  switch(freq) {
  case ZIVADS_CLKFREQ256:
    
    value = 2;
    break;

  case ZIVADS_CLKFREQ384:
    
    value = 0;
    break;

  default:
    
    return(-EINVAL);
  }
  
  // set it!
  zivaDS_set_mem(instance, ZIVADS_AUDIO_CLOCK_SELECTION, value);  
  
  // OK
  return(0);
}
