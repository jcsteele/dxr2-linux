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
 * Driver for the Brooktree/Rockwell/Conexant BT865 TV encoder chip
 * Register get/set functions
 *
 */


#include <linux/errno.h>
#include <bt865.h>



/**
 *
 * Get register from the BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int bt865_get_reg (bt865_t* instance, int reg)
{
  // if the register == -1 => do readback
  if (reg == -1) {
    
    return (*instance->ops->get_reg) (instance, 0xfe);
  }
  
  // check register valid
  if ((reg <0) || (reg > 255)) {

    return(-EINVAL);
  }

  // return stored values
  return(instance->regValues[reg]);
}


/**
 *
 * Set register on the BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void bt865_set_reg (bt865_t* instance, int reg, int val)
{
  if ((reg < 0) || (reg > 255)) {
    
    return;
  }
  
  // write it
  (*instance->ops->set_reg) (instance, reg, val);
  
  // remember it
  instance->regValues[reg] = val;
}


/**
 *
 * Get specified bitmask of a register from BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int bt865_get_bits(bt865_t* instance, int reg, int bitmask)
{
  return (bt865_get_reg(instance, reg) & bitmask);
}


/**
 *
 * Set specified bits of a register on BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void bt865_set_bits(bt865_t* instance, int reg, int bitmask, int valuemask)
{
  // get the current register value
  int value = bt865_get_reg(instance, reg);
  
  // set it on the hardware
  bt865_set_reg(instance, reg, (value & (~bitmask)) | valuemask);
}
