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
 * Low level AnP82 functions
 *
 */


#include <linux/errno.h>
#include <dxr2.h>
#include <anp82.h>
#include <vxp524.h>



static int dxr2_anp82_get_reg(anp82_t* instance, int reg);
static int dxr2_anp82_set_reg(anp82_t* instance, int reg, int val);
static int dxr2_anp82_send_byte(vxp524_t* instance, int data);
static int dxr2_anp82_read_byte(vxp524_t* instance);

anp82_ops_t anp82_ops_table = {

  DXR2_LOGNAME,
  dxr2_anp82_get_reg,
  dxr2_anp82_set_reg,
};



/**
 *
 * Get register from the anp82
 *
 * @param instance anp82 instance to use
 * @param reg Register to read
 *
 * @return value (-1 => error)
 *
 */

static int dxr2_anp82_get_reg(anp82_t* instance, int reg)
{
  int data;
  int status = 0;
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  vxp524_t* vxp524Inst = dxr2Inst->vxp524Instance;


  // open i2c bus
  vxp524_i2s_init(vxp524Inst);
  vxp524_set_bits(vxp524Inst, VXP524_I2C, 0x80, 0x80);

  // Send control byte
  if ((status = dxr2_anp82_send_byte(vxp524Inst, ANP82_CHIPID | 0x80)) < 0) {
    
    vxp524_i2s_close(vxp524Inst);
    return(status);
  }

  // Send index
  if ((status = dxr2_anp82_send_byte (vxp524Inst, reg)) < 0) {
    
    vxp524_i2s_close(vxp524Inst);
    return(status);
  }
  
  // Send data
  if ((data = dxr2_anp82_read_byte (vxp524Inst)) < 0) {

    vxp524_i2s_close(vxp524Inst);    
    return(data);
  }
  
  // turn i2c off again
  vxp524_set_bits(vxp524Inst, VXP524_I2C, 0x80, 0x0);
  vxp524_i2s_close(vxp524Inst);

  // return
  return data;
}



/**
 *
 * Set register on the anp82
 *
 * @param instance anp82 instance to use
 * @param reg Register to write
 * @param reg Value to write
 *
 * @return status (-1 => error)
 *
 */

static int dxr2_anp82_set_reg(anp82_t* instance, int reg, int val)
{
  int status = 0;
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  vxp524_t* vxp524Inst = dxr2Inst->vxp524Instance;

  // open i2c bus
  vxp524_i2s_init(vxp524Inst);
  vxp524_set_bits(vxp524Inst, VXP524_I2C, 0x80, 0x80);

  // Send control byte
  if (dxr2_anp82_send_byte(vxp524Inst, ANP82_CHIPID) < 0) {
	  
    vxp524_i2s_close(vxp524Inst);
    return(-ETIMEDOUT);
  }

  // Send register
  if ((status = dxr2_anp82_send_byte(vxp524Inst, reg)) < 0) {
	 
    vxp524_i2s_close(vxp524Inst);
    return(status);
  }

  // Send data
  if ((status = dxr2_anp82_send_byte(vxp524Inst, val)) < 0) {
	  
    vxp524_i2s_close(vxp524Inst);
    return(status);
  }

  // turn i2c off again
  vxp524_set_bits(vxp524Inst, VXP524_I2C, 0x80, 0x0);
  vxp524_i2s_close(vxp524Inst);

  // OK
  return(0);
}


/**
 *
 * Send a byte to the AnP82
 *
 * @param instance VxP524 instance
 * @param data Data to send
 *
 * @return status (0=>success, -1=>error)
 *
 */

static int dxr2_anp82_send_byte (vxp524_t* instance, int data)
{
  int i, j;
  int status = 0;

  // start bit
  vxp524_i2s_set_sda(instance);
  if (vxp524_i2s_wait_till_sda_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_clear_sda(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_clear_scl(instance);

  // Clock the bits out, MSB to LSB 
  for (i = 7; i >= 0; --i) {

    // set data bit appropriately
    if (data & (1<<i)) {

      vxp524_i2s_set_sda(instance);
      if (vxp524_i2s_wait_till_sda_set(instance, 1000) < 0) {
	
	status--;
      }
    }
    else {

      vxp524_i2s_clear_sda(instance);
    }
    
    // Clock pulse
    vxp524_i2s_set_scl(instance);
    if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
      
      status--;
    }
    vxp524_i2s_clear_scl(instance);
  }
  
  // Send stop bit
  vxp524_i2s_set_sda(instance);
  if (vxp524_i2s_wait_till_sda_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  
  // get ACK
  vxp524_i2s_tri_sda(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  i = vxp524_i2s_get_sda(instance);
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  j = vxp524_i2s_get_sda(instance);
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_untri_sda(instance);

  // check status
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }

  // Check ack
  if (!i && j)
    return(0);
  
  // erk! not got an ACK... just say we've timed out
  return(-ETIMEDOUT);
}



/**
 *
 * Read a byte from the AnP82
 *
 * @param instance VxP524 instance
 *
 * @return data (-1=>error)
 *
 */

static int dxr2_anp82_read_byte (vxp524_t* instance)
{
  int i, data = 0;
  int status=0;


  // Clock start bit, with data tri-stated
  vxp524_i2s_tri_sda(instance);
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  if (vxp524_i2s_wait_till_sda_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_set_scl(instance);  
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  
  // Read the bits in, MSB to LSB 
  for (i = 0; i < 8; i++) {

    // shift old value left 1
    data <<= 1;

    // Read the bit
    if (vxp524_i2s_get_sda(instance)) {
      
      data |= 1;
    }

    // clock up
    vxp524_i2s_set_scl(instance);
    if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
      
      status--;
    }

    // Clock down
    vxp524_i2s_clear_scl(instance);
  }
  
  // Clock stop bit
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  
  // Send ack
  vxp524_i2s_clear_sda(instance);
  vxp524_i2s_untri_sda(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_sda(instance);
  if (vxp524_i2s_wait_till_sda_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);
  vxp524_i2s_set_scl(instance);
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }
  vxp524_i2s_clear_scl(instance);

  // check status
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }

  // OK, return the data
  return(data);
}


