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
 * Driver for the Toshiba TC6807AF CSS decode chip
 * Driver Maintenance Functions
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

#include <tc6807af.h>
#include <mod_lic.h>

/**
 *
 * Create new tc6807af driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern tc6807af_t* tc6807af_new (tc6807af_ops_t* ops, void* data)
{
  tc6807af_t* instance;
  // changed by zulli
//  int val;
  int status;
  
  // validate supplied ops
  if (ops == NULL)
    return NULL;
  
  // allocate
  instance = (tc6807af_t*) vmalloc (sizeof (tc6807af_t));
  if (!instance)
    return NULL;
  
  // setup
  instance->ops = ops;
  instance->data = data;

  // init the device
  if ((status = tc6807af_init(instance)) < 0) {
    
    vfree(instance);
    printk(KERN_INFO TC6807AF_LOGNAME ": [%s] Unable to initialise.\n",
	   instance->ops->name);
    return(NULL);
  }
  
  // ok, module has one more usage
  MOD_INC_USE_COUNT;

  // log creation
  printk (KERN_INFO TC6807AF_LOGNAME ": [%s] instance created.\n", 
	  instance->ops->name);
  
  // return it!
  return(instance);
}



/**
 *
 * Destroy a tc6807af driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void tc6807af_free (tc6807af_t* instance)
{
  char* tmpName = instance->ops->name;

  // free instance
  vfree (instance);

  // one less usage of module
  MOD_DEC_USE_COUNT;

  // log destruction
  printk (KERN_INFO TC6807AF_LOGNAME ": [%s] instance destroyed.\n", 
	  tmpName);
}


#ifdef MODULE
int init_module(void)
{

  // OK
  return(0);
}

void cleanup_module(void)
{
}
#endif




EXPORT_SYMBOL(tc6807af_new);
EXPORT_SYMBOL(tc6807af_free);
EXPORT_SYMBOL(tc6807af_get_loc);
EXPORT_SYMBOL(tc6807af_set_loc);
EXPORT_SYMBOL(tc6807af_init);
EXPORT_SYMBOL(tc6807af_set_decryption_mode);
EXPORT_SYMBOL(tc6807af_send_challenge_key);
EXPORT_SYMBOL(tc6807af_get_challenge_key);
EXPORT_SYMBOL(tc6807af_send_response_key);
EXPORT_SYMBOL(tc6807af_get_response_key);
EXPORT_SYMBOL(tc6807af_send_disc_key_part1);
EXPORT_SYMBOL(tc6807af_send_disc_key_part2);
EXPORT_SYMBOL(tc6807af_send_title_key);

