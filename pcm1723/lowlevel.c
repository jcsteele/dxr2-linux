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
 * Register get/set functions
 *
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <pcm1723.h>



/**
 *
 * Get register from the PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 *
 * @return the value, or <0 on error
 *
 */

extern int pcm1723_get_reg(pcm1723_t* instance, int reg)
{
  // check params
  if ((reg < 0) || (reg >= PCM1723_REGISTERCOUNT)) {
    
    return(-EINVAL);
  }

  // retrieve it from the instance buffer (PCM1723 is WRITE ONLY)
  return(instance->registerValues[reg] & 0x1ff);
}


/**
 *
 * Set register on the PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int pcm1723_set_reg(pcm1723_t* instance, int reg, int val)
{
  // check params
  if ((reg < 0) || (reg >= PCM1723_REGISTERCOUNT)) {
    
    return(-EINVAL);
  }

  // remember the register value for later (PCM1723 is WRITE ONLY)
  instance->registerValues[reg] = val & 0x1ff;
  
  // set it on the actual hardware
  return((*instance->ops->set_reg) (instance, ((reg & 3) << 9) | (val & 0x1ff)));
}



/**
 *
 * Get specified bitmask of a register from PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues, or <0 on error
 *
 */

extern int pcm1723_get_bits(pcm1723_t* instance, int reg, int bitmask)
{
  return (pcm1723_get_reg(instance, reg) & bitmask);
}



/**
 *
 * Set specified bits of a register on PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int pcm1723_set_bits(pcm1723_t* instance, int reg, int bitmask, int valuemask)
{
  int value;

  // get the current register value
  if ((value = pcm1723_get_reg(instance, reg)) < 0) {
    
    return(-EINVAL);
  }
  
  // set it on the hardware
  return(pcm1723_set_reg(instance, reg, (value & (~bitmask)) | valuemask));
}

