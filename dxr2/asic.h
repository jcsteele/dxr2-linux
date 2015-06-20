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
 * Functions for accessing the Creative ASIC
 *
 */

#ifndef __ASIC_H__
#define __ASIC_H__

#include <dxr2.h>



/**
 *
 * Initialise the ASIC
 *
 * @param instance dxr2 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_asic_init(dxr2_t* instance);


/**
 *
 * Set the ASIC to the supplied value
 *
 * @param instance DXR2 instance to use
 * @param val value to write
 *
 */

extern void dxr2_asic_set_reg(dxr2_t* instance, int val);



/**
 *
 * Set specified bits of the ASIC
 *
 * @param instance DXR2 instance to use
 * @param bitmask Bitmask of bits to set
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void dxr2_asic_set_bits(dxr2_t* instance, int bitmask, int valuemask);


/**
 *
 * Set the i2c bus into normal mode
 *
 * @param instance DXR2 instance to use
 *
 */

extern void dxr2_asic_set_i2c_normal_mode(dxr2_t* instance);



/**
 *
 * Set the i2c bus into PCM1723 mode
 *
 * @param instance DXR2 instance to use
 *
 */

extern void dxr2_asic_set_i2c_pcm_mode(dxr2_t* instance);


#endif
