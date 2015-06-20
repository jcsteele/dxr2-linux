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
 * Driver for the Burr-Brown PCM1723 DAC
 * High level functions.
 *
 */

#include <linux/errno.h>
#include <linux/types.h>
#include <pcm1723.h>


/**
 *
 * Initialises the DAC. 
 *
 * @param instance Instance of the PCM1723 to use
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_init(pcm1723_t* instance)
{
  // set left & right attenuation to 0dB
  pcm1723_set_reg(instance, PCM1723_REG0, 0xff);
  pcm1723_set_reg(instance, PCM1723_REG1, 0xff);
  pcm1723_set_reg(instance, PCM1723_REG0, 0x1ff);
  pcm1723_set_reg(instance, PCM1723_REG1, 0x1ff);

  // Mute off, Deemphasis off, operational control=0, input resolution=16bits,
  // normal stereo mode enabled.
  pcm1723_set_reg(instance, PCM1723_REG2, 0x120);
  
  // i2c mode, l&r channel data polarity reversed, attenuation applied
  // to each channel seperately, system clock=256fs, sample rate=normal,
  // sampling frequency=32kHz group, infinite zero detect off
  pcm1723_set_reg(instance, PCM1723_REG3, 0x4B);

  // OK
  return(0);
}


/**
 *
 * Sets the DAC Sample frequency.
 *
 * @param instance Instance of the PCM1723 to use
 * @param freqMode. One of the PCM1723_FREQXX defines
 *
 * @return nonzero on failure
 *
 */

int pcm1723_set_sample_frequency(pcm1723_t* instance, int freqMode)
{
  u32 tmp;

  // get the current DAC register 3
  if ((tmp = pcm1723_get_reg(instance, PCM1723_REG3)) < 0) {
    
    return(-EINVAL);
  }

  // set the sampling frequency = 44.1, multiplier =1
  tmp &= 0xFFFFFF0F;

  // set the frequency mode
  switch(freqMode) {
  case PCM1723_FREQ441:
    break;

  case PCM1723_FREQ48:
    tmp |= 0x40;
    break;

  case PCM1723_FREQ96:
    tmp |= 0x50;
    break;
    
  case PCM1723_FREQ2205:
    tmp |= 0x20;
    break;
    
  case PCM1723_FREQ32:
    tmp |= 0x80;
    break;
    
  default:
    return(-EINVAL);
  }

  // set the value
  return(pcm1723_set_reg(instance, PCM1723_REG3, tmp));
}



/**
 *
 * Sets the DAC input width
 *
 * @param instance Instance of the PCM1723 to use
 * @param freqMode. One of the PCM1723_WIDTHXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_input_width(pcm1723_t* instance, int width)
{
  u32 tmp;

  // get the current DAC register 2
  if ((tmp = pcm1723_get_reg(instance, PCM1723_REG2)) < 0) {
    
    return(-EINVAL);
  }

  // set input width = 16 bits
  tmp &= 0xFFFFFFE7;

  // set the input width
  switch(width) {
  case PCM1723_WIDTH16:
    break;

  case PCM1723_WIDTH20:
    tmp |= 8;
    break;

  case PCM1723_WIDTH24:
    tmp |=0x10;
    break;
    
  default:
    return(-EINVAL);
  }

  // set the value
  return(pcm1723_set_reg(instance, PCM1723_REG2, tmp));
}



/**
 *
 * Sets/unsets the mute on the DAC
 *
 * @param instance Instance of the PCM1723 to use
 * @param muteMode. One of the PCM1723_MUTEON or PCM1723_MUTEOFF defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_mute_mode(pcm1723_t* instance, int muteMode)
{
  u32 tmp;

  // get the current DAC register 2
  if ((tmp = pcm1723_get_reg(instance, PCM1723_REG2)) < 0) {
    
    return(-EINVAL);
  }

  // mute off
  tmp &= 0xFFFFFFFE;

  if (muteMode == PCM1723_MUTEON) {
    
    tmp |=1;
  }

  // set the value
  return(pcm1723_set_reg(instance, PCM1723_REG2, tmp));
}



/**
 *
 * Sets stereo mode
 *
 * @param instance Instance of the PCM1723 to use
 * @param muteMode. One of the PCM1723_STEREOXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_stereo_mode(pcm1723_t* instance, int stereoMode)
{
  u32 tmp;

  // get the current DAC register 2
  if ((tmp = pcm1723_get_reg(instance, PCM1723_REG2)) < 0) {
    
    return(-EINVAL);
  }

  // wipe stereo mode bits
  tmp &= 0xFFFFFE1F;

  // set stereo mode
  switch(stereoMode) {
  case PCM1723_STEREONORMAL:
    
    tmp |= 0x120;
    break;

  case PCM1723_STEREOMONOL:
    
    tmp |= 0x140;
    break;
    
  case PCM1723_STEREOMONOR:

    tmp |= 0xA0;
    break;

  case PCM1723_STEREOREVERSE:

    tmp |= 0xC0;
    break;
    
  default:
    
    return(-EINVAL);
  }

  // set the value
  return(pcm1723_set_reg(instance, PCM1723_REG2, tmp));
}



/**
 *
 * Sets clock frequency
 *
 * @param instance Instance of the PCM1723 to use
 * @param clockFrequency. One of the PCM1723_CLKFREQXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_clock_frequency(pcm1723_t* instance, int clockFrequency)
{
  u32 tmp;

  // get the current DAC register 2
  if ((tmp = pcm1723_get_reg(instance, PCM1723_REG3)) < 0) {
    
    return(-EINVAL);
  }

  // clock mode = 384fs
  tmp &= 0xFFFFFFF7;

  // clock mode = 256fs 
  if (clockFrequency == PCM1723_CLKFREQ256) {
    
    tmp |= 8;
  }

  // set the value
  return(pcm1723_set_reg(instance, PCM1723_REG3, tmp));
}
