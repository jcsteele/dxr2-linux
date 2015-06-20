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
 * Driver for the C-Cube Ziva-DS MPEG decoder chip
 * Register get/set functions.
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <zivaDS.h>



/**
 *
 * Get register from the ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 *
 * @return The register's value, or <0 on failure
 *
 */

extern int zivaDS_get_reg(zivaDS_t* instance, int reg)
{
  return (*instance->ops->get_reg) (instance, reg);
}


/**
 *
 * Set register on the ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void zivaDS_set_reg(zivaDS_t* instance, int reg, int val)
{
  (*instance->ops->set_reg) (instance, reg, val);
}



/**
 *
 * Get specified bitmask of a register from ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues, or <0 on failure
 *
 */

extern int zivaDS_get_bits(zivaDS_t* instance, int reg, int bitmask)
{
  return (zivaDS_get_reg(instance, reg) & bitmask);
}



/**
 *
 * Set specified bits of a register on ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void zivaDS_set_bits(zivaDS_t* instance, int reg, int bitmask, int valuemask)
{

  // get the current register value
  int value = zivaDS_get_reg(instance, reg);
  
  // set it on the hardware
  zivaDS_set_reg(instance, reg, (value & (~bitmask)) | valuemask);
}


/**
 *
 * Read from a ZivaDS memory location
 *
 * @param instance Instance of Ziva DS to read from
 * @param addr Address to read from in that memory area
 *
 * @return Memory value
 *
 */

extern int zivaDS_get_mem (zivaDS_t* instance, int addr)
{
  // return the value
  return (*instance->ops->get_mem) (instance, addr);
}



/**
 *
 * Write to a ZivaDS memory location
 *
 * @param instance Instance of Ziva DS to read from
 * @param addr Address to write to in that memory area
 * @param val Value to write
 *
 */

extern void zivaDS_set_mem (zivaDS_t* instance, int addr, int val)
{
  // set the value
  (*instance->ops->set_mem) (instance, addr, val);
}






/**
 *
 * Issues a command to the Ziva chip
 *
 * @param instance Instance of the ZivaDS to use
 * @param command Ziva command to send
 * @param arg0 Argument 0 for command
 * @param arg1 Argument 1 for command
 * @param arg2 Argument 2 for command
 * @param arg3 Argument 3 for command
 * @param arg4 Argument 4 for command
 * @param arg5 Argument 5 for command
 * @param intMask Ziva Int Mask to set (0 for don't bother)
 * @param statusToWaitFor Ziva Status to wait for (0 for don't bother)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_command(zivaDS_t* instance,
			  int command,
			  int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
			  int intMask, int statusToWaitFor)
{
  int oldIntFlag;
  int oldIntMask;
  // changed by zulli
//  int oldIntStatus;
  int endTime;
  int status = 0;
  int statusAddress;
  zivaDS_int_src_t intStatus;
  int loopFlag;


  // if the int mask isn't zero, set supplied int status on ziva
  if (intMask != 0) {
    
    // get old interrupt enabled flag
    oldIntFlag = zivaDS_is_int_enabled(instance);
    
    // disable ziva interrupts
    zivaDS_enable_int(instance, 0);

    // get old int mask & status
    oldIntMask = zivaDS_get_mem(instance, ZIVADS_INT_MASK);

    // clear any pending interrupts
    zivaDS_get_int_status(instance, &intStatus);
    
    // write supplied int mask
    zivaDS_set_mem(instance, ZIVADS_INT_MASK, intMask);
  }

  // loop until ziva status address is set
  loopFlag=0;
  endTime = jiffies + ((40*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();
    
    // get status addres
    if (zivaDS_get_mem(instance, ZIVADS_STATUS_ADDRESS)) {
      
      loopFlag=1;
      break;
    }
  }
  
  // has loop elapsed? if so => error
  if (!loopFlag) {
    
    return(-ETIMEDOUT);
  }
  
  // OK, issue command to the ziva
  zivaDS_set_mem(instance, ZIVADS_COMMAND, command);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_1, arg0);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_2, arg1);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_3, arg2);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_4, arg3);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_5, arg4);
  zivaDS_set_mem(instance, ZIVADS_PARAMETER_6, arg5);

  // OK, ziva status address = 0.... does this start the command going?
  zivaDS_set_mem(instance, ZIVADS_STATUS_ADDRESS, 0);
  
  // OK, if the intMask != 0 => wait until interrupt occurs
  if (intMask != 0) {
    
    // loop until ziva status address is set
    loopFlag =0;
    endTime = jiffies + ((30*HZ)/100);
    while(jiffies < endTime) {
      
      // let other things in
      schedule();
      
      // is the interrupt set?
      if (zivaDS_get_int_status(instance, &intStatus) & intMask) {

	loopFlag=1;
	break;
      }
    }
    
    // restore old int mask
    zivaDS_set_mem(instance, ZIVADS_INT_MASK, oldIntMask);

    // turn interrupts back on if oldIntFlag says so.
    if (oldIntFlag != 0) {

      zivaDS_enable_int(instance, 1);
    }

    // did timer elapse?
    if (!loopFlag) {
      
      status = -ETIMEDOUT;
    }
  }
  
  // right, if statusToWaitFor != 0 => wait for it...
  if (statusToWaitFor != 0) {
    
    // loop until ziva status is what we're looking for, or timeout
    loopFlag=0;
    endTime = jiffies + ((80*HZ)/100);
    while(jiffies < endTime) {
      
      // let other things in
      schedule();
      
      // get the status address
      // changed by zulli inserted ( )
      if ( (statusAddress = zivaDS_get_mem(instance, ZIVADS_STATUS_ADDRESS)) ) {
	
	if (zivaDS_get_mem(instance, statusAddress) == statusToWaitFor) {
	  
	  loopFlag=1;
	  break;
	}
      }
    }
    
    // did timer elapse?
    if (!loopFlag) {
      
      status = -ETIMEDOUT;
    }
  }
  // return status
  return(status);
}



/**
 *
 * Determines whether the ziva Int is currently enabled
 *
 * @param instance zivaDS instance
 *
 * @return 0 => int NOT enabled, 1 => int enabled
 *
 */

extern int zivaDS_is_int_enabled(zivaDS_t* instance)
{
  
  return(instance->intEnabledFlag);
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

extern int zivaDS_enable_int(zivaDS_t* instance, int flag) 
{
  
  // OK, do the hardware specifics
  (*instance->ops->enable_int) (instance, flag);

  // remember status
  instance->intEnabledFlag = flag;

  // OK
  return(0);
}



/**
 *
 * Get ziva int status
 *
 * @param instance zivaDS instance
 * @param itnSrcBuf buffer to hold intsrcs
 * 
 * @return int status bitmap from ziva
 *
 */

extern int zivaDS_get_int_status(zivaDS_t* instance, zivaDS_int_src_t* intSrcBuf)
{
  int intStatus;


  // read the int status.. if it's 0, exit now
  intStatus = zivaDS_get_mem(instance, ZIVADS_INT_STATUS);

  // OK, do stuff if some ints ARE marked as set
  if (intStatus != 0) {

    // ??	
    zivaDS_set_mem(instance, 0x800000, 0x1002);

    // OK, read int srcs
    if (intStatus & 0x80000) {
      
      intSrcBuf->HLI_int_src = zivaDS_get_mem(instance, ZIVADS_HLI_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_HLI_INT_SRC, 0);
    }
    if (intStatus & 0x10000) {
      
      intSrcBuf->BUF_int_src = zivaDS_get_mem(instance, ZIVADS_BUFF_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_BUFF_INT_SRC, 0);    
    }
    if (intStatus & 0x100) {
      
      intSrcBuf->UND_int_src = zivaDS_get_mem(instance, ZIVADS_UND_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_UND_INT_SRC, 0);
    }
    if (intStatus & 0x80) {
      
      intSrcBuf->AOR_int_src = zivaDS_get_mem(instance, ZIVADS_AOR_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_AOR_INT_SRC, 0);
    }
    if (intStatus & 0x8000) {
      
      intSrcBuf->AEE_int_src = zivaDS_get_mem(instance, ZIVADS_AEE_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_AEE_INT_SRC, 0);
    }
    if (intStatus & 1) {
      
      intSrcBuf->ERR_int_src = zivaDS_get_mem(instance, ZIVADS_ERR_INT_SRC);
      zivaDS_set_mem(instance, ZIVADS_ERR_INT_SRC, 0);
    }
  
    // set int status to 0 (i.e. the interrupt has been dealt with)
    zivaDS_set_mem(instance, ZIVADS_INT_STATUS, 0);
  }
  
  // return status
  return(intStatus);
}


/**
 *
 * Checks if the ziva is ZIVADS_TYPE_1 or not (i.e. doesn't have onboard CSS)
 *
 * @param instance zivaDS instance
 *
 * @return 0 => NOT ZIVADS_TYPE_1, 1=> it IS ZIVADS_TYPE_1
 *
 */

extern int zivaDS_check_type_1(zivaDS_t* instance)
{
  // if this bit is set => it is!!
  if (zivaDS_get_mem(instance, 0x800000) & 0x40000) {
    
    return(1);
  }

  // failed
  return(0);
}


/**
 *
 * Check the ziva is there
 *
 * @param instance zivaDS instance
 *
 * @return 0 if it is, <0 on failure
 *
 */

extern int zivaDS_validate(zivaDS_t* instance)
{
  // write to ziva reg
  zivaDS_set_reg(instance, ZIVADS_REGADDRESS, 0x65);
  
  // check it is still that value
  if (zivaDS_get_reg(instance, ZIVADS_REGADDRESS) != 0x65) {
    
    return(-ENODEV);
  }
  
  // OK!
  return(0);
}

