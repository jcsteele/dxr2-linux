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
 * Low level bt865 functions
 *
 */


#include <dxr2.h>
#include <bt865.h>
#include <vxp524.h>



// ****************************************************************************
// BT865 operations


static int dxr2_bt865_get_reg(bt865_t* instance, int reg);
static int dxr2_bt865_set_reg(bt865_t* instance, int reg, int val);


bt865_ops_t bt865_ops_table = {

  DXR2_LOGNAME,
  dxr2_bt865_get_reg,
  dxr2_bt865_set_reg
};




/**
 *
 * Get a BT865 register
 *
 * @param instance bt865 instance to use
 * @param reg register to get
 *
 * @return value on success (>=0), <0 o failure
 *
 */

static int dxr2_bt865_get_reg(bt865_t* instance, int reg)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  
  return(vxp524_i2c_read_reg(dxr2Inst->vxp524Instance, BT865_CHIPID, reg));
}

/**
 *
 * Set a BT865 register
 *
 * @param instance bt865 instance to use
 * @param reg register to set
 * @param val value to set
 *
 * @return 0 on success, <0 o failure
 *
 */

static int dxr2_bt865_set_reg(bt865_t* instance, int reg, int val)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;

  return(vxp524_i2c_write_reg(dxr2Inst->vxp524Instance, BT865_CHIPID, reg, val));
}

