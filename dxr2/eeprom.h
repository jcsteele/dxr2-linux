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
 * Functions for accessing the Atmel Serial EEPROM
 *
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <vxp524.h>


#define EEPROM_CHIPID 0xA0


/**
 *
 * Read a byte from the EEPROM
 *
 * @param instance VXP524 instance to use
 * @param addr Addresst to read from
 *
 * @return value (>=0) on success, or error (<0)
 *
 */

extern int dxr2_eeprom_read_byte(vxp524_t* instance, int addr);

/**
 *
 * Read a byte from the EEPROM
 *
 * @param instance VXP524 instance to use
 * @param addr Addresst to use
 * @param val Value to write
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int dxr2_eeprom_write_byte(vxp524_t* instance, int addr, int val);


#endif
