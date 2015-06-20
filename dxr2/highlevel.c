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
 * High level misc functions
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <dxr2.h>
#include <vxp524.h>
#include <zivaDS.h>
#include <pcm1723.h>
#include <bt865.h>
#include <tc6807af.h>
#include "highlevel.h"
#include "asic.h"



/**
 *
 * Table of bitstream-specific transfer parameters. These are determined by hand
 * The size of each allocated bm buffer MUST be geater then or equal to
 * each of these
 *
 */

static int thresholdsTable[] = {
  
  0x8000,  // DVD transfer size
  10000,   // DVD min video buffer size
  10000,   // DVD min audio buffer size

  0x2000,  // VCD (CDROM) transfer size
  40000,   // VCD min video buffer size
  10000,   // VCD min audio buffer size

  0x2000,  // VCD (DECODED) transfer size
  40000,   // VCD min video buffer size
  10000,   // VCD min audio buffer size

  0x1000,  // CDDA transfer size
  40000,   // CDDA min video buffer size
  10000,   // CDDA min audio buffer size

  0x1000,   // ??
  40000,   // ??
  10000    // ??
};


/**
 *
 * Sub IRQ handler....  the buit which actually does stuff. Exported so it can also be called
 * from non-irq code when starting a TX. Starts a bm transfer from the writeBuffer.
 *
 * @param instance DXR2 instance
 */

extern void dxr2_sub_irq_handler(void* instance);


/**
 *
 * Actually starts a BM transfer going (if it can)
 *
 * @param instance DXR2 instance to use
 *
 * @return 1 if a BM was started, 0 if not
 *
 */

extern int dxr2_do_bm_transfer(dxr2_t* instance);



/**
 *
 * Extracts hardware type
 *
 * @param instance DXR2 instance to use
 *
 * @return -ENODEV on validation failure, or device type (>=0) on success
 *
 */

extern int dxr2_zivaDS_get_hardware_type(dxr2_t* instance)
{
  int rom0;
  int rom1;

  // read in the two WORDS from the eeprom
  rom0 = dxr2_eeprom_read_byte(instance->vxp524Instance, 0) << 8;
  rom0 |= dxr2_eeprom_read_byte(instance->vxp524Instance, 1);

  rom1 = dxr2_eeprom_read_byte(instance->vxp524Instance, 0xA) << 8;
  rom1 |= dxr2_eeprom_read_byte(instance->vxp524Instance, 0xB);

  // this seems to be some form of validity check
  if (rom0 != 0xA55A) {
    
    return(-ENODEV);
  }

  // OK, this works out what kind of hardware we have (I think)
  if (rom1 == 0) {

    return(ZIVADS_TYPE_2);
  }
  else if (rom1 == 0x100D) {
    
    return(ZIVADS_TYPE_3);
  }
  else if (rom1 == 0x100C) {
    
    return(ZIVADS_TYPE_4);
  }

  // erk! hardware not valid!
  return(-ENODEV);
}



/**
 *
 * IRQ handler function
 *
 * You may wonder why we don't do the BM transfer in here, or in a bottom half
 * The reason is that even though this IRQ indicates the last BM operation has
 * finished, this DOESN'T necessarily mean that the ZiVA is ready for more
 * data yet. The ziva doesn't cause an IRQ when it is ready for data....
 * we have to POLL for this. So, we have to do a big loop at the start of 
 * dxr2_do_bm_transfer(), to wait until the ziva can cope with more data. 
 * This is a problem. Theoretically, the ziva could take up to 1/2 second to 
 * be ready for more data, so the delay loop would have to wait this long.
 *
 * There are four options
 * 1) Busy loop in the IRQ handler. This is obviosuly NOT feasible.
 * 2) Loop in a bottom half (i.e. schedule something on tq_immediate)
 *    We tried this, BUT we still can't call the schedule() function in a bottom half, 
 *    so we'd have to have a busy loop.... for 1/2 sec, which is NOT nice.
 * 3) Stick it on tq_scheduler. This only gets executed when the scheduler is run... which isn't
 *    deterministic enough for us. (believe me, we tried it)
 * 4) The user's call to dxr2_io_write() call goes to sleep for a bit after starting a BM transfer.
 *    The IRQ handler to wakes this up again when the BM finishes, so dxr2_io_write() can start
 *    another transfer. Since we're not in any IRQ code, dxr2_io_write() call can use schedule() 
 *    in it's polling loop to wait for the ziva to become ready... thus avoiding a nasty busy loop.
 * 
 * Solution 4, although not quite as clean as we'd like it to be, turned out to be the only 
 * viable solution.
 *
 * @param irq IRQ number
 * @param vInstance DXR2 instance pointer
 * @param regs Processor regs
 *
 */

extern void dxr2_irq_handler(int irq, void* vInstance, struct pt_regs* regs) 
{
  dxr2_t* instance = (dxr2_t*) vInstance;


  // is the IRQ for us?
  if (!vxp524_bm_completed(instance->vxp524Instance, 1)) {
    
    return;
  }

  // OK, if we've got this, then the BM operation must have finished,
  // so mark it as such
  vxp524_bm_not_in_use(instance->vxp524Instance);

  // wake up waiting task
  wake_up_interruptible(&(instance->waitQueue));
}



/**
 *
 * Actually starts a BM transfer going (if it can)
 *
 * @param instance DXR2 instance to use
 *
 * @return number of bytes sent if a BM was started, 0 if no BM started
 *
 */

extern int dxr2_do_bm_transfer(dxr2_t* instance) 
{
  int tmp;
  int status = 0;
  int size;
  int endTime;

  // offset into the thresholds table
  tmp = (instance->currentBitstreamType * 3);

  // check buffers on ziva.... if there's no space => wait until there is some,
  // or timeout
  endTime = jiffies + ((60*HZ)/100);
  while (((zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_VIDEO_EMPTINESS) < thresholdsTable[tmp+1]) ||
	 (zivaDS_get_mem(instance->zivaDSInstance, ZIVADS_AUDIO_EMPTINESS) < thresholdsTable[tmp+2]) ||
	 (zivaDS_get_reg(instance->zivaDSInstance, ZIVADS_REGCONTROL) & 0x80)) &&
	 (jiffies < endTime)) {

    schedule();
  }

  // if the above elapsed, the ziva ain't going to become ready, so return 0 -- no new BM transfer started
  if (jiffies >= endTime) {
    
    return(0);
  }

  // swap buffers over... so the BM buffer is the one that was just written in to
  tmp = instance->bmBuffer;
  instance->bmBuffer = instance->writeBuffer;
  instance->writeBuffer = tmp;
  instance->bufferCount[instance->writeBuffer] = 0;

  // if there is some data in the buffer, send it
  if (instance->bufferCount[instance->bmBuffer] > 0) {

    vxp524_bm_send_data(instance->vxp524Instance,
			virt_to_bus((void*) instance->buffer[instance->bmBuffer]),
			instance->bufferCount[instance->bmBuffer],
			1);
    status = instance->bufferCount[instance->bmBuffer];
  }

  // OK, copy next load of data into the write buffer
  size = instance->userBufferSize - instance->userBytesTransferred;
  if (size > 0) {
    
    tmp = (instance->currentBitstreamType * 3);
    if (size > thresholdsTable[tmp]) {
      
      size = thresholdsTable[tmp];
    }
    copy_from_user((void*) instance->buffer[instance->writeBuffer], 
		   (void*) instance->userBuffer + instance->userBytesTransferred,
		   size);
    instance->bufferCount[instance->writeBuffer] = size;

    // update the number of bytes we've transferred
    instance->userBytesTransferred += size;
  }
    
  // return the status
  return(status);
}

  
/**
 *
 * Handles user's writes to the device
 *
 * 
 */

extern ssize_t dxr2_io_write(struct file* filp, const char* buf, size_t count, 
			     loff_t* offset) 
{
  dxr2_t* instance = (dxr2_t*) filp->private_data;
  int size;
  int tmp;
  int sentBytes = 0;

  // if there's no supplied data, exit now
  if (count == 0) {
    
    return(count);
  }
  
  // OK, prime inital buffer
  size = count;
  tmp = (instance->currentBitstreamType * 3);
  if (size > thresholdsTable[tmp]) {
   
    size = thresholdsTable[tmp];
  }
  copy_from_user((void*) instance->buffer[instance->writeBuffer], 
		 (void*) buf, size);
  instance->bufferCount[instance->writeBuffer] = size;

  // remember the data the user supplied for later... they may have supplied more than the 
  // threshold, and we'll have to send that in a bit as well
  instance->userBuffer = (char*) buf;
  instance->userBytesTransferred = size;
  instance->userBufferSize = count;

  // OK, loop until we run out of data
  while(1) {
    
    // if the BM is still in use, don't do anything 
    // (we must have timed out before the operation had finished)
    if (!instance->vxp524Instance->bmInUse) {
      
      // OK, enter critical section
      DXR2_ENTER_CRITICAL(instance);
      
      // issue any queued commands
      dxr2_process_deferred_queue(instance);

      // if a new BM transfer wasn't started, we'ev run out of data,
      // or the ZiVA refused to accept any new (e.g. user has paused it),
      // so we can return to the user
      if (!(tmp = dxr2_do_bm_transfer(instance))) {
	
	DXR2_EXIT_CRITICAL(instance);
	break;
      }

      // increment the number of bytes actually sent
      sentBytes+= tmp;

      // critical section done.
      DXR2_EXIT_CRITICAL(instance);
    }
    
    // sleep for a bit. Notice that we have a timeout. This is because
    // when we first wrote this, we were on a PII-400, and it worked 
    // perectly. However, when we later tried it on a K6-233, we 
    // had nasty hanging problems, because the IRQ would sometimes 
    // occur while we were still in the dxr2_do_bm_transfer() above. 
    // This meant the IRQ called wake_up_interruptible on us, BEFORE 
    // we had actually gone to sleep. We then happily slept.... forever, 
    // because we had missed the wake_up call. After about 8 hours of 
    // trying different architectures, we came to the conclusion that 
    // using a timeout was the only 100% certain way of avoiding such problems.

    // this comment is so detailed because every so often we kept on
    // thinking "ah ha!" I can remove that timeout.. followed about 15
    // minutes later by realising why not... gah!
    interruptible_sleep_on_timeout(&(instance->waitQueue), 1);
  }

  // OK, return the bytes transmitted.
  return(sentBytes);
}

