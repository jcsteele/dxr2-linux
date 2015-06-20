/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     December 18, 2001    Scott Bucholtz       Include mod_lic.h
  *
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
 * Driver for the Burr-Brown PCM1723 DAC
 * Driver maintenance functions
 *
 */


#define EXPORT_SYMTAB

#include <dxr2modver.h>
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/cdrom.h>
#include <linux/videodev.h>
#include <asm/byteorder.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,10)
 #include <linux/spinlock.h>
#else
 #include <asm/spinlock.h>
#endif

#include <pcm1723.h>
#include <mod_lic.h>

/**
 *
 * Create new pcm1723 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 * @return new instance (or NULL on error)
 *
 */

extern pcm1723_t* pcm1723_new (pcm1723_ops_t* ops, void* data)
{
  pcm1723_t* instance;
  // changed by zulli
//  int val;
  int i;
  
  // validate supplied ops
  if (ops == NULL)
    return NULL;
  
  // allocate
  instance = (pcm1723_t*) vmalloc (sizeof (pcm1723_t));
  if (!instance)
    return NULL;
  
  // setup
  instance->ops = ops;
  instance->data = data;

  // zero the registerValues
  for(i=0; i < PCM1723_REGISTERCOUNT; i++) {

    instance->registerValues[i] = 0;
  }
  
  // initialise the device
  if (pcm1723_init(instance) < 0) {
    
    printk (KERN_INFO PCM1723_LOGNAME ": [%s] Cannot initialise pcm1723.\n",
	    instance->ops->name);
    vfree(instance);
    return(NULL);
  }
  
  // ok, module has one more usage
  MOD_INC_USE_COUNT;

  // log creation
  printk (KERN_INFO PCM1723_LOGNAME "(%s): instance created.\n", 
	  instance->ops->name);
  
  // return it!
  return(instance);
}


/**
 *
 * Destroy a Pcm1723 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void pcm1723_free (pcm1723_t* instance)
{
  char* tmpName = instance->ops->name;

  // free instance
  vfree (instance);

  // one less usage of module
  MOD_DEC_USE_COUNT;

  // log destruction
  printk (KERN_INFO PCM1723_LOGNAME ": [%s] instance destroyed.\n", 
	  tmpName);
}


#ifdef MODULE
int init_module(void)
{
  return(0);
}

void cleanup_module(void)
{
}
#endif

EXPORT_SYMBOL(pcm1723_new);
EXPORT_SYMBOL(pcm1723_free);
EXPORT_SYMBOL(pcm1723_init);
EXPORT_SYMBOL(pcm1723_set_sample_frequency);
EXPORT_SYMBOL(pcm1723_set_input_width);
EXPORT_SYMBOL(pcm1723_set_mute_mode);
EXPORT_SYMBOL(pcm1723_set_stereo_mode);
EXPORT_SYMBOL(pcm1723_set_clock_frequency);
EXPORT_SYMBOL(pcm1723_get_reg);
EXPORT_SYMBOL(pcm1723_set_reg);
EXPORT_SYMBOL(pcm1723_get_bits);
EXPORT_SYMBOL(pcm1723_set_bits);
