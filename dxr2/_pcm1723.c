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
 * Low level pcm1723 functions
 *
 */


#include <dxr2.h>
#include <pcm1723.h>
#include "asic.h"


static int dxr2_pcm1723_set_reg(pcm1723_t* instance, int val);
static int dxr2_pcm1723_write_bits(pcm1723_t* instance, int val);


pcm1723_ops_t pcm1723_ops_table = {

  DXR2_LOGNAME,
  dxr2_pcm1723_set_reg 
};



/**
 *
 * Set register on the pcm1723
 *
 * @param instance instance of the pcm1723 to use
 * @param val value to set (already combined with the register to set)
 *
 * @return 0 on success, <0 on failure
 *
 */

static int dxr2_pcm1723_set_reg(pcm1723_t* instance, int val)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  

  // set pcm1723 mode
  dxr2_asic_set_i2c_pcm1723_mode(dxr2Inst);

  // output the bits
  dxr2_pcm1723_write_bits(instance, val);

  // back to normal mode
  dxr2_asic_set_i2c_normal_mode(dxr2Inst);
  
  // OK
  return(0);
}


/**
 *
 * Writes bits to the pcm1723 (on the vxp i2c bus)
 *
 * @param instance instance of the pcm1723 to use
 * @param val value/register to write
 *
 * @return 0 on success, <0 on failure
 *
 */
 
static int dxr2_pcm1723_write_bits(pcm1723_t* instance, int val) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  vxp524_t* vxp524Inst =(vxp524_t*) dxr2Inst->vxp524Instance;
  int i;

  // write each bit out
  for(i=15; i>=0; i--) {
    
    // clear clock
    vxp524_i2s_clear_scl(vxp524Inst);

    // send data bit	
    if (val & (1<<i)) {
      
      vxp524_i2s_set_sda(vxp524Inst);
    }
    else {
      
      vxp524_i2s_clear_sda(vxp524Inst);
    }

    // set clock
    vxp524_i2s_set_scl(vxp524Inst);
  }

  // OK
  return(0);
}
  
