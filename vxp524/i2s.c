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
 * Driver for the Auravision VxP524 video processor
 * Low level vxp524 generic i2s operations
 *
 */

#include <linux/errno.h>
#include <vxp524.h>


/**
 *
 * Initialise the i2s bus
 *
 * @param instance VxP524 instance to use
 */

extern void vxp524_i2s_init(vxp524_t* instance) {
  
  vxp524_set_reg(instance, VXP524_I2C, 0x05);
}


/**
 * 
 * Close the i2s bus after use
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_close(vxp524_t* instance) {
  
  vxp524_set_reg(instance, VXP524_I2C, 0x0F);
}


/**
 *
 * Set the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_set_sda(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x4, 0x4);
}


/**
 *
 * Clear the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_clear_sda(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x4, 0);
}


/**
 *
 * Tristate the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_tri_sda(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x8, 0x8);
}


/**
 *
 * Un-Tristate the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_untri_sda(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x8, 0);
}


/**
 *
 * Get the current value of the DATA line on the i2s bus
 *
 * @param instance the VxP524 instance to use
 *
 */

extern int vxp524_i2s_get_sda(vxp524_t* instance) {
  
  // was the bit set?
  if (vxp524_get_bits(instance, VXP524_I2C, 0x20)) {
    
    return(1);
  }
  
  // wasn't set
  return(0);
}


/**
 *
 * Set the CLOCK line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_set_scl(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x1, 0x1);
}


/**
 *
 * Clear the clock line on the i2s bus
 *
 * @param instance the VxP524 instance to use
 *
 */

extern void vxp524_i2s_clear_scl(vxp524_t* instance) {
  
  vxp524_set_bits(instance, VXP524_I2C, 0x1, 0);

  // always read it once after doing this (dunno why)
  vxp524_get_reg(instance, VXP524_I2C);
}


/**
 *
 * Reads the DATA line until it becomes set. Max. count reads
 *
 * @param instance the VxP524 instance to use
 * @param count Maximum number of reads
 *
 * @return 0 on success, -ETIMEDOUT if the DATA line never became set
 *
 */

extern int vxp524_i2s_wait_till_sda_set(vxp524_t* instance, int count) {

  int counter = 0;
  // changed by zulli
//  int value;
  
  // try 1000 times
  while(counter++ < count) {
    
    // was it set?
    if (vxp524_get_bits(instance, VXP524_I2C, 0x20)) {
      
      break;
    }
  }

  // dunno why it does this
  vxp524_get_reg(instance, VXP524_I2C);
  vxp524_get_reg(instance, VXP524_I2C);
  vxp524_get_reg(instance, VXP524_I2C);
  vxp524_get_reg(instance, VXP524_I2C);

  // what happened...
  if (counter == 0) {
    
    return(-ETIMEDOUT);
  }
  
  // OK! we got it!
  return(0);
}
  



/**
 *
 * Reads the CLOCK line until it becomes set. Max. count reads
 *
 * @param instance the VxP524 instance to use
 * @param count Maximum number of reads
 *
 * @return 0 on success, -ETIMEDOUT if the CLOCK line never became set
 *
 */

extern int vxp524_i2s_wait_till_scl_set(vxp524_t* instance, int count) {

  int counter = 0;
  // changed by zulli
//  int value;
  
  // try 1000 times
  while(counter++ < count) {
    
    // was it set?
    if (vxp524_get_bits(instance, VXP524_I2C, 0x10)) {
      
      break;
    }
  }

  // dunno why it does this
  vxp524_get_reg(instance, VXP524_I2C);
  vxp524_get_reg(instance, VXP524_I2C);

  // what happened...
  if (counter == 0) {
    
    return(-ETIMEDOUT);
  }
  
  // OK! we got it!
  return(0);
}
