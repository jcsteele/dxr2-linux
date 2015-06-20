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
 * vxp524 Bus mastering routines
 *
 */

#include <asm/atomic.h>
#include <linux/errno.h>
#include <vxp524.h>



/**
 *
 * Start bus master TX to VXP524
 *
 * @param instance VXP524 instance to use
 * @param buffer Buffer start address (this is the BUS ADDRESS of it BTW)
 * @param count Number of bytes to TX
 * @param irqEndFlag 1=> cause IRQ on tx end, 0=>no IRQ on tx end
 *
 */

extern void vxp524_bm_send_data(vxp524_t* instance, 
			      unsigned long buffer, unsigned long count,
			      int irqEndFlag)
{
  // set address
  vxp524_set_reg32(instance, VXP524_PCI_DMA_ADDR0, buffer);

  // set count
  vxp524_set_reg24(instance, VXP524_PCI_DMA_CNT0, count);

  // set PCI burst size to 32 dwords
  vxp524_set_reg(instance, VXP524_PCI_BURST_SIZE, 0xff);

  // setup FIFO thresholds 
  vxp524_set_reg(instance, VXP524_PCI_FIFO_THRESH, 0xf);

  // all IRQs off
  vxp524_set_reg(instance, VXP524_INTR_STATUS, 0);

  // setup interrupt on TX end status
  if (irqEndFlag) {

    // IRQ on TX end
    vxp524_set_reg(instance, VXP524_INTR_CTRL, 0x10);
  }
  else {

    // no IRQs
    vxp524_set_reg(instance, VXP524_INTR_CTRL, 0);
  }
    
  // start TX!
  vxp524_set_reg(instance, VXP524_INPUT_CFG, 0x12);

  // OK! BM is now in use
  instance->bmInUse =1;
}


/**
 *
 * Flush the vxp BM system
 *
 * @param instance Vxp524 instance to use
 *
 */

extern void vxp524_bm_flush(vxp524_t* instance)
{
  // enable demux
  vxp524_set_bits(instance, VXP524_INPUT_CFG, 2, 2);
  
  // all IRQs off
  vxp524_set_reg(instance, VXP524_INTR_STATUS, 0);

  // Master FIFO flush On
  vxp524_set_bits(instance, VXP524_PCI_GLOBAL_REGS, 0x10, 0x10);

  // Master FIFO flush Off
  vxp524_set_bits(instance, VXP524_PCI_GLOBAL_REGS, 0x10, 0);

  // BM no longer in use
  instance->bmInUse = 0;
}


/**
 *
 * Reset the Vxp524 BM system
 *
 * @param instance Vxp524 instance to use
 *
 */

extern void vxp524_bm_reset(vxp524_t* instance)
{
  // disable PCI master cycle
  vxp524_set_bits(instance, VXP524_INPUT_CFG, 0x10, 0);  

  // flush BM stuff
  vxp524_bm_flush(instance);
  
  // all interrupts off
  vxp524_set_reg(instance, VXP524_INTR_STATUS, 0); 
} 



/**
 *
 * Check whether the BM operation has completed yet?
 *
 * @param instance Vxp524 instance to use
 * @param checkType 0 => see if the PCI master cycle has finished yet...
 *                  1 => see if the IRQ has occurred yet, and clear it if so
 *
 * @return 1 if the operation HAS completed, 0 if not
 *
 */

extern int vxp524_bm_completed(vxp524_t* instance, int checkType)
{
  switch(checkType) {
  case 0:
    
    // if the PCI master cycle is no longer ENABLED, then the TX HAS finished
    if (!(vxp524_get_reg(instance, VXP524_INPUT_CFG) & 0x10)) {
      
      return(1);
    }
    
    // otherwise, return 0
    return(0);
    
  case 1:
    
    // if IRQ on PCI burst end is not enabled... return(0)
    if (!(vxp524_get_reg(instance, VXP524_INTR_CTRL) & 0x10)) {

      return(0);
    }
    
    // if IRQ on burst end status != 1, return(0)
    if (!(vxp524_get_reg(instance, VXP524_INTR_STATUS) & 0x10)) {
      
      return(0);
    }
    
    // clear bus master IRQ status
    vxp524_set_bits(instance, VXP524_INTR_STATUS, 0x10, 0);

    // OK! DMA HAD finished!
    return(1);
    
  default:
    
    return(0);
  }
}



/**
 *
 * Checks if BM in use, and sets BM in use flag if it wasn't
 *
 * @param instance vxp524 instance to use
 *
 * @return 0 on success, -EBUSY if BM is already in use
 *
 */

extern int vxp524_bm_check_status(vxp524_t* instance)
{
  // changed by zulli
//  unsigned long flags;
  int status = -EBUSY;
  
  // if BM isn't in use, mark it as such & set status to successful
  if (!instance->bmInUse) {
    
    instance->bmInUse = 1;
    status = 0;
  }

  // return status
  return(status);
}



/**
 *
 * Mark BM as no longer in use
 *
 */

extern int vxp524_bm_not_in_use(vxp524_t* instance)
{
  instance->bmInUse = 0;
  
  // OK
  return(0);
}
