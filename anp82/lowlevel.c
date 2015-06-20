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
 * Driver for the SkyTune/Auravision AnP82 VGA overlay chip
 * Register get/set functions
 *
 */

#include <anp82.h>




/**
 *
 * Get register from the AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 *
 * @return The register's value (or negative on error)
 *
 */

extern int anp82_get_reg (anp82_t* instance, int reg)
{
  return (*instance->ops->get_reg) (instance, reg);
}


/**
 *
 * Set register on the AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void anp82_set_reg(anp82_t* instance, int reg, int val)
{
  (*instance->ops->set_reg) (instance, reg, val);
}



/**
 *
 * Get specified bitmask of a register from AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int anp82_get_bits(anp82_t* instance, int reg, int bitmask)
{
  return (anp82_get_reg(instance, reg) & bitmask);
}


/**
 *
 * Set specified bits of a register on AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void anp82_set_bits(anp82_t* instance, int reg, int bitmask, int valuemask)
{
  // get the current register value
  int value = anp82_get_reg(instance, reg);
  
  // set it on the hardware
  anp82_set_reg(instance, reg, (value & (~bitmask)) | valuemask);
}

