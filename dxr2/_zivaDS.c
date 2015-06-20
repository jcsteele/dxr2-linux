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
 * Low level zivaDS functions
 *
 */

#include <dxr2modver.h>
#include <zivaDS.h>
#include <dxr2.h>
#include <asm/io.h>
#include <asm/uaccess.h>


static int dxr2_zivaDS_get_reg (zivaDS_t* instance, int reg);
static int dxr2_zivaDS_set_reg (zivaDS_t* instance, int reg, int val);
static int dxr2_zivaDS_get_mem (zivaDS_t* instance, int addr);
static int dxr2_zivaDS_set_mem (zivaDS_t* instance, int reg, int val);
static int dxr2_zivaDS_enable_int (zivaDS_t* instance, int flag);
static int dxr2_zivaDS_set_bits(zivaDS_t* instance, int reg, int bitmask, int valuemask);



zivaDS_ops_t zivaDS_ops_table = {

  DXR2_LOGNAME,
  dxr2_zivaDS_get_reg,
  dxr2_zivaDS_set_reg,
  dxr2_zivaDS_get_mem,
  dxr2_zivaDS_set_mem,
  dxr2_zivaDS_enable_int
};




/**
 *
 * Get a *register* from the ziva
 *
 * @param instance Instance of the ziva to use
 * @param reg Register to get
 *
 * @return Value on success, <0 on failure
 *
 */

static int dxr2_zivaDS_get_reg (zivaDS_t* instance, int reg)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;

  // read & return value
  return(readb(dxr2Inst->zivaDSBase + (reg<<2)));
}


/**
 *
 * Set a *register* on the ziva
 *
 * @param instance Instance of the ziva to use
 * @param reg Register to set
 * @param val Value to set
 *
 * @return 0 on success, <0 on failure
 *
 */

static int dxr2_zivaDS_set_reg (zivaDS_t* instance, int reg, int val)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;

  // write byte out
  writeb(val, dxr2Inst->zivaDSBase + (reg<<2));
  
  // OK
  return(0);
}


/**
 *
 * Get a memory location from the ziva DS
 *
 * @param instance instance of the ziva to use
 * @param addr Address on the ziva to get
 *
 * @return value on success, <0 on failure
 *
 */

static int dxr2_zivaDS_get_mem (zivaDS_t* instance, int addr)
{
  // changed by zulli
//  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  u32 value;

  // clear bit 3 of ZiVA register 7
  dxr2_zivaDS_set_bits(instance, ZIVADS_REGCONTROL, 0x8, 0);

  // write the address out
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS, addr & 0xff); addr >>= 8;
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS+1, addr & 0xff); addr >>= 8;
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS+2, addr & 0xff);

  // read reg3 ONCE
  dxr2_zivaDS_get_reg(instance, ZIVADS_REGDATA+3);
  

  // read in the data values
  value =  (dxr2_zivaDS_get_reg(instance, ZIVADS_REGDATA+3) << 24);
  value |= (dxr2_zivaDS_get_reg(instance, ZIVADS_REGDATA+2) << 16);
  value |= (dxr2_zivaDS_get_reg(instance, ZIVADS_REGDATA+1) << 8);
  value |=  dxr2_zivaDS_get_reg(instance, ZIVADS_REGDATA+0);

  // return it
  return(value);
}



/**
 *
 * Set a memory location on the ziva DS
 *
 * @param instance instance of the ziva to use
 * @param addr Address on the ziva to set
 * @param val Value to set
 *
 * @return 0 on success, <0 on failure
 *
 */

static int dxr2_zivaDS_set_mem(zivaDS_t* instance, int addr, int val)
{
  // changed by zulli
//  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;


  // clear bit 3 of ZiVA register 7
  dxr2_zivaDS_set_bits(instance, ZIVADS_REGCONTROL, 0x8, 0);

  // write the address out
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS, addr & 0xff); addr >>= 8;
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS+1, addr & 0xff); addr >>= 8;
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGADDRESS+2, addr & 0xff);

  // write out the data values
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGDATA+3, (val & 0xff000000) >> 24);
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGDATA+2, (val & 0xff0000) >> 16);
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGDATA+1, (val & 0xff00) >> 8);
  dxr2_zivaDS_set_reg(instance, ZIVADS_REGDATA+0, (val & 0xff));
  
  // OK
  return(0);
}


/**
 *
 * Enable/disable ziva interrupt
 *
 * @param instance zivaDS instance
 * @param flag 0=> disable, 1=> enable
 *
 * @return 0 on success, <0 on failure
 *
 */

static int dxr2_zivaDS_enable_int(zivaDS_t* instance, int flag)
{
  if (!flag) {
    
    dxr2_asic_set_bits(instance->data, 0x40, 0x40);
  }
  else {
    
    dxr2_asic_set_bits(instance->data, 0x40, 0);
  }

  // OK 
  return(0);
}
  


/**
 *
 * Set specified bits of an (8bit) register on zivaDS
 *
 * @param instance Instance of the vxp524 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 * @return 0 on success, <0 on failure
 *
 */

static int dxr2_zivaDS_set_bits(zivaDS_t* instance, int reg, int bitmask, int valuemask)
{

  // get the current register value
  // changed by zulli deleted 'int value ='
  // int value = dxr2_zivaDS_get_reg(instance, reg);
  dxr2_zivaDS_get_reg(instance, reg);

  // set it on the hardware
  dxr2_zivaDS_set_reg(instance, reg, (valuemask & (~bitmask)) | valuemask);

  // OK
  return(0);
}



/**
 *
 * Send disc key to the Ziva DS
 *
 * @param instance The instance of zivaDS to use
 * @param discKey 0x800 byte array containing the disc key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_zivaDS_send_disc_key(zivaDS_t* instance, char* discKey) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  int status=0;
  int endTime;
  int loopFlag;
  // changed by zulli
//  int i;

  // check BM is not in use
  if ((status = vxp524_bm_check_status(dxr2Inst->vxp524Instance)) < 0) {
    
    return(status);
  }

  // copy supplied buffer to DMA buffer
  if (!memcpy((void*) dxr2Inst->buffer[0], (void*) discKey, 0x800)) {
    
    return(-EFAULT);
  }

  // do some ASIC twiddling
  dxr2_asic_set_bits(dxr2Inst, 0x10, 0x10);
  dxr2_asic_set_bits(dxr2Inst, 0x20, 0x20);

  // get the ziva "warmed up"
  if ((status = zivaDS_send_disc_key_part1(instance)) < 0) {
    
    return(status);
  }

  // send the data
  // remembering to convert VIRTUAL address into BUS address...
  vxp524_bm_send_data(dxr2Inst->vxp524Instance, virt_to_bus((void*) dxr2Inst->buffer[0]), 0x800, 0);
  
  // wait for BM TX to finish
  loopFlag=0;
  endTime = jiffies + ((50*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // wait till DMA complete
    if (vxp524_bm_completed(dxr2Inst->vxp524Instance, 0)) {
      
      loopFlag=1;
      break;
    }
  }
  
  // disable PCI bus master
  vxp524_set_bits(dxr2Inst->vxp524Instance, VXP524_INPUT_CFG, 0x10, 0);

  // OK, bm no longer in use..
  vxp524_bm_not_in_use(dxr2Inst->vxp524Instance);

  // did it time out?
  if (!loopFlag) {

    return(-ETIMEDOUT);
  }
  
  // finalise
  zivaDS_send_disc_key_part2(instance);

  // OK
  return(0);
}
