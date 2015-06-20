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
 * CSS functions
 *
 */


#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <tc6807af.h>


/**
 *
 * Initialise the TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_init(tc6807af_t* instance)
{
  // initialisation
  tc6807af_set_loc(instance, TC6807AF_LOC2, 8);
  tc6807af_set_loc(instance, TC6807AF_LOC1, 0x10);
  tc6807af_set_loc(instance, TC6807AF_LOC2, 0x18);
  tc6807af_set_loc(instance, TC6807AF_LOC4, 0);
  tc6807af_set_loc(instance, TC6807AF_LOC5, 0);
  tc6807af_set_loc(instance, TC6807AF_LOC1, 3);
  tc6807af_set_loc(instance, TC6807AF_LOC2, 0x37);

  //changed by zulli
  return(0); // success
}
