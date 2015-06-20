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
 * Low level tc6807af functions
 *
 */


#include <dxr2modver.h>
#include <linux/types.h>
#include <dxr2.h>
#include <tc6807af.h>
#include <zivaDS.h>
#include <asm/io.h>
#include <asm/uaccess.h>


static int dxr2_tc6807af_get_reg(tc6807af_t* instance, int reg);
static int dxr2_tc6807af_set_reg(tc6807af_t* instance, int reg, int val);


tc6807af_ops_t tc6807af_ops_table = {
  
  DXR2_LOGNAME,
  dxr2_tc6807af_get_reg,
  dxr2_tc6807af_set_reg
};


/**
 *
 * Get register from the tc6807af
 *
 * @param instance instance of the tc6807af to use
 * @param reg register to get
 *
 * @return register value, or <0 on error
 *
 */

static int dxr2_tc6807af_get_reg(tc6807af_t* instance, int reg)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  
  // read & return it
  return(readb((u32) dxr2Inst->tc6807afBase + (reg<<2)));
}



/**
 *
 * Set register on the tc6807af
 *
 * @param instance instance of the tc6807af to use
 * @param reg register to set
 * @param val Value to set
 *
 * @return 0 on success, or <0 on error
 *
 */

static int dxr2_tc6807af_set_reg(tc6807af_t* instance, int reg, int val) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  
  // write it
  writeb(val, (u32) dxr2Inst->tc6807afBase + (reg<<2));

  // OK
  return(0);
}



/**
 *
 * Get challenge key from tc6807af
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array to recieve the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_get_challenge_key(tc6807af_t* instance, char* key) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_get_challenge_key(instance, key);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}


/**
 *
 * Send challenge key to TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array containing the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_send_challenge_key(tc6807af_t* instance, char* key) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_send_challenge_key(instance, key);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}


/**
 *
 * Get the response key from the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array to receive the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_get_response_key(tc6807af_t* instance, char* key) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_get_response_key(instance, key);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}


/**
 *
 * Send the response key to the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_send_response_key(tc6807af_t* instance, char* key) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_send_response_key(instance, key);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}


/**
 *
 * Send the disc title key
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 6 byte char array containing the key to send (incl CGMS flags in byte 0)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_send_title_key(tc6807af_t* instance, char* key) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_send_title_key(instance, key);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}


/**
 *
 * Set tc6807af decryption mode
 *
 * @param instance The instance of TC6807AF to use
 * @param mode CSS mode (one of TC6807AF_CSSDECMODEX)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_set_decryption_mode(tc6807af_t* instance, int mode) 
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status;

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // set Css mode
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);
  
  // do the tc6807af function
  status = tc6807af_set_decryption_mode(instance, mode);

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}



/**
 *
 * Send disc key to the TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 * @param key 0x800 byte array containing the disc key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_tc6807af_send_disc_key(tc6807af_t* instance, char* discKey)
{
  dxr2_t* dxr2Inst = (dxr2_t*) instance->data;
  zivaDS_cssFlags_t oldFlags;
  int status=0;
  int endTime;
  int loopFlag;

  // disable IRQs
  DXR2_ENTER_CRITICAL(dxr2Inst);

  // check BM is not in use
  if ((status = vxp524_bm_check_status(dxr2Inst->vxp524Instance)) < 0) {
    
    DXR2_EXIT_CRITICAL(dxr2Inst);
    return(status);
  }

  // copy supplied buffer to DMA buffer
  if (!memcpy((void*) dxr2Inst->buffer[0], (void*) discKey, 0x800)) {

    DXR2_EXIT_CRITICAL(dxr2Inst);
    return(-EFAULT);
  }

  // read the old ziva flags
  zivaDS_get_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // do the command
  zivaDS_new_play_mode(dxr2Inst->zivaDSInstance);

  // reset bus mastering
  vxp524_bm_reset(dxr2Inst->vxp524Instance);

  // do some ASIC twiddling
  dxr2_asic_set_bits(dxr2Inst, 0x10, 0);
  dxr2_asic_set_bits(dxr2Inst, 0x10, 0x10);
  dxr2_asic_set_bits(dxr2Inst, 0x20, 0);
  dxr2_asic_set_bits(dxr2Inst, 0x20, 0x20);

  // tell the tc6807af to get ready
  status = tc6807af_send_disc_key_part1(dxr2Inst->tc6807afInstance);

  // send the data
  // remembering to convert VIRTUAL address into BUS address...
  vxp524_bm_send_data(dxr2Inst->vxp524Instance, virt_to_bus((void*) dxr2Inst->buffer[0]), 0x800, 0);

  // set Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 1);

  // wait for BM TX to finish
  loopFlag=0;
  endTime = jiffies + ((40*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // wait till DMA complete
    if (vxp524_bm_completed(dxr2Inst->vxp524Instance, 0)) {
      
      loopFlag=1;
      break;
    }
  }
  
  // clear bit4 of ASIC
  dxr2_asic_set_bits(dxr2Inst, 0x10, 0);
  
  // disable PCI bus master
  vxp524_set_bits(dxr2Inst->vxp524Instance, VXP524_INPUT_CFG, 0x10, 0);

  // OK, bm no longer in use..
  vxp524_bm_not_in_use(dxr2Inst->vxp524Instance);

  // enable IRQs
  DXR2_EXIT_CRITICAL(dxr2Inst);

  // ok, tell the tc6807af to process it, if the timer didn't elapse
  if (loopFlag) {

    // OK, wait for tc6807af to process it
    status = tc6807af_send_disc_key_part2(instance);
  }
  else {
    
    status = -ETIMEDOUT;
  }

  // set non-Css mode 
  zivaDS_set_css_mode(dxr2Inst->zivaDSInstance, 0);

  // restore the old flags
  zivaDS_restore_css_flags(dxr2Inst->zivaDSInstance, &oldFlags);

  // return status
  return(status);
}







