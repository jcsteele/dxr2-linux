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
 * Driver for the Toshiba TC6807AF CSS decode chip
 * Register get/set functions
 *
 */

#include <tc6807af.h>


/**
 *
 * Get location from the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param loc Location to retrieve
 * @return The location's value (or negative on error)
 *
 */

extern int tc6807af_get_loc(tc6807af_t* instance, int loc)
{
  // set register number
  (*instance->ops->set_reg) (instance, TC6807AF_REGADDRESS, loc);

  // get & return value
  return((*instance->ops->get_reg) (instance, TC6807AF_REGDATA));
}


/**
 *
 * Set location on the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param reg Location to set
 * @param val Value to set
 *
 */

extern int tc6807af_set_loc(tc6807af_t* instance, int loc, int val)
{
  // register number
  (*instance->ops->set_reg) (instance, TC6807AF_REGADDRESS, loc);

  // set value
  (*instance->ops->set_reg) (instance, TC6807AF_REGDATA, val);

  // OK
  return(0);
}


