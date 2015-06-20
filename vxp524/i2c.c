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
 * vxp524 i2c operations
 *
 */

#include <linux/errno.h>
#include <vxp524.h>



/**
 *
 * Send an ACK on the i2c bus
 *
 * @param instance VXP524 instance to use
 *
 */

static int vxp524_i2s_send_ack(vxp524_t* instance) 
{
  int status = 0;
  
  // set clock line
  vxp524_i2s_set_scl(instance);

  // wait till it is reflected on the input
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) <0) {
    
    status--;
  }

  // clear clock line
  vxp524_i2s_clear_scl(instance);
  
  // return status
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }
  
  // OK
  return(0);
}


/**
 *
 * Wait for acknowledgement from slave i2c device
 *
 * @param instance VXP524 instance to use
 *
 */

static int vxp524_i2s_wait_for_ack(vxp524_t* instance) 
{
  int status=0;
  
  // tristate the data line
  vxp524_i2s_tri_sda(instance);
  
  // wait till sda set
  if (vxp524_i2s_wait_till_sda_set(instance, 5000) <0) {
    
    status--;
  }

  // un-tristate the data line
  vxp524_i2s_untri_sda(instance);
  
  // set clock line
  vxp524_i2s_set_scl(instance);

  // wait till it is reflected on the input
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }

  // clear clock line
  vxp524_i2s_clear_scl(instance);
  
  // check for error 
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }

  // OK
  return(0);
}


/**
 *
 * Send a stop bit
 *
 * @param instance VXP524 instance to use
 *
 */

static int vxp524_i2s_send_stop_bit(vxp524_t* instance) 
{
  int status = 0;

  // clear data line
  vxp524_i2s_clear_sda(instance);

  // set clock line
  vxp524_i2s_set_scl(instance);

  // wait till it is reflected on the input
  status = vxp524_i2s_wait_till_scl_set(instance, 1000);

  // set data line
  vxp524_i2s_set_sda(instance);
  
  // return status
  return(status);
}



/**
 *
 * Send a start bit
 *
 * @param instance VXP524 instance to use
 *
 */

static int vxp524_i2s_send_start_bit(vxp524_t* instance) 
{
  int status=0;

  // set data line
  vxp524_i2s_set_sda(instance);

  // wait till it is reflected on the input
  if (vxp524_i2s_wait_till_sda_set(instance, 1000) <0) {
    
    status--;
  }

  // set clock line
  vxp524_i2s_set_scl(instance);

  // wait till it is reflected on the input
  if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {
    
    status--;
  }

  // clear data line
  vxp524_i2s_clear_sda(instance);

  // clear clock line
  vxp524_i2s_clear_scl(instance);
  
  // return status
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }
  
  // OK
  return(0);
}



/**
 *
 * Read a (8bit) byte from the i2c interface
 *
 * @param instance VXP524 instance to use
 *
 * @return The value (>=0), or <0 on error
 *
 */

static int vxp524_i2s_read_byte(vxp524_t* instance) 
{
  int value=0;
  int status=0;
  int i;

  // tri state the SDA read line
  vxp524_i2s_tri_sda(instance);

  // read in each bit MSB first
  for(i=0; i<8; i++) {
    
    // set clock
    vxp524_i2s_set_scl(instance);

    // wait for it to be reflected on input lines
    if (vxp524_i2s_wait_till_scl_set(instance, 1000) < 0) {

      status--;
    }
    
    // get the next bit
    value <<=1; // shift acc left a bit
    value |= vxp524_i2s_get_sda(instance); // OR on the next value
    
    // clear clock
    vxp524_i2s_clear_scl(instance);
  }

  // tri state the SDA read line
  vxp524_i2s_untri_sda(instance);

  // return the status if there was an error
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }

  // return the value if everything was OK
  return(value);
}



/**
 *
 * Write an (8bit) byte to the i2c interface
 *
 * @param instance VXP524 instance to use
 * @param val Value to write
 *
 * @return 0 on success, or <0 on error
 *
 */

static int vxp524_i2s_write_byte(vxp524_t* instance, int val) 
{
  int curBit = 0x80;
  int status=0;
  int i;

  // write out each bit MSB first
  for(i=0; i<8; i++) {
    
    // set the SDA line appropriately
    if (val & curBit) {
      
      vxp524_i2s_set_sda(instance);
    }
    else {

      vxp524_i2s_clear_sda(instance);
    }

    // set clock
    vxp524_i2s_set_scl(instance);

    // wait for it to be reflected on input lines
    if (vxp524_i2s_wait_till_scl_set(instance, 1000) <0) {

      status --;
    }

    // clear clock
    vxp524_i2s_clear_scl(instance);

    // move curBit right one
    curBit >>= 1;
  }

  // tri state the SDA read line
  if (vxp524_i2s_wait_for_ack(instance) < 0) {
    
    return(-ETIMEDOUT);
  }

  // return the status if there was an error
  if (status < 0) {
    
    return(-ETIMEDOUT);
  }

  // return the value if everything was OK
  return(0);
}



/**
 *
 * Read a value from a register on a device connected to the i2c bus
 *
 * @param instance VXP524 instance to use
 * @param devId Device id to read from
 * @param reg Register on the device to read from
 *
 * @return Value on success (>=0), or <0 on error
 *
 */

extern int vxp524_i2c_read_reg(vxp524_t* instance, int devId, int reg)
{
  int value = 0;
  int status = 0;
  
  // initialise I2S bus
  vxp524_i2s_init(instance);
  
  // send start bit
  if (vxp524_i2s_send_start_bit(instance) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // write device ID
  if (vxp524_i2s_write_byte(instance, devId) < 0) {
    
    status = -ETIMEDOUT;
    goto error;
  }

  // write register
  if (vxp524_i2s_write_byte(instance, reg) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }
  
  // send start bit
  if (vxp524_i2s_send_start_bit(instance) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // write device ID (set bit1... ie. READ from device)
  if (vxp524_i2s_write_byte(instance, devId | 1) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // read register
  if ((value = vxp524_i2s_read_byte(instance)) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // send ACK
  if (vxp524_i2s_send_ack(instance) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }
  
  // send stop bit
  if (vxp524_i2s_send_stop_bit(instance) < 0) {

    status = -ETIMEDOUT;
  }

 error:

  // shutdown i2c bus
  vxp524_i2s_close(instance);
  
  // if it's an error, return it
  if (status <0) {
    
    return(status);
  }
  
  // return the read value
  return(value);
}
  


/**
 *
 * Write a value to a register on a device connected to the i2c bus
 *
 * @param instance VXP524 instance to use
 * @param devId Device id to write to
 * @param reg Register on the device to write to
 * @param val Value to write
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int vxp524_i2c_write_reg(vxp524_t* instance, int devId, int reg, int val)
{
  int status=0;

  // initialise I2S bus
  vxp524_i2s_init(instance);
  
  // send start bit
  if (vxp524_i2s_send_start_bit(instance) < 0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // write device ID
  if (vxp524_i2s_write_byte(instance, devId) <0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // write register
  if (vxp524_i2s_write_byte(instance, reg) <0) {

    status = -ETIMEDOUT;
    goto error;
  }

  // write value
  if (vxp524_i2s_write_byte(instance, val) <0) {

    status = -ETIMEDOUT;
    goto error;
  }
 
  // send stop bit
  if (vxp524_i2s_send_stop_bit(instance) <0) {
 
    status = -ETIMEDOUT;
    goto error;
  }

 error:

  // shutdown i2c bus
  vxp524_i2s_close(instance);

  // OK
  return(status);
}
