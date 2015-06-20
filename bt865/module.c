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
 * Driver for the Brooktree/Rockwell/Conexant BT865 TV encoder chip
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

#include <bt865.h>
#include <mod_lic.h>

/**
 *
 * Create new BT865 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern bt865_t* bt865_new (bt865_ops_t* ops, void *data)
{
  bt865_t* instance;
  int val;
  
  // validate ops
  if (ops == NULL)
    return NULL;
  
  // create new structure 
  instance = (bt865_t*) vmalloc (sizeof(bt865_t));
  if (!instance)
    return NULL;

  // copy necessary data in
  instance->ops = ops;
  instance->data = data;

  // zero the register values
  memset(instance->regValues, 0, 256);

  // try and detect the chip
  val = bt865_get_reg(instance, BT865_READBACK);
  // changed by zulli : inserted ( )
  if ( (val < 0) ||
      ( (((val >> 5) != 0x05)) && ((val >> 5) != 0x04)) ) {
    
    printk (KERN_INFO BT865_LOGNAME ": [%s] no BT864, or BT865 detected!\n",
	    instance->ops->name);
    vfree(instance);
    return(NULL);
  }

  // keep note of chip type
  instance->chipType = val >> 5;

  // initialise the chip
  bt865_init(instance);

  // report OK
  printk (KERN_INFO BT865_LOGNAME ": [%s] %s instance created.\n",
	  instance->ops->name, (instance->chipType == 5) ? "BT865" : "BT864" );

  // OK, another module usage
  MOD_INC_USE_COUNT;
  
  // return new structure
  return(instance);
}


/**
 *
 * Destroy a BT865 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void bt865_free (bt865_t* instance)
{
  char* tmpName = instance->ops->name;

  bt865_set_bits(instance, 0xCE, 2, 0);

  // free instance
  vfree (instance);

  // one less usage of module
  MOD_DEC_USE_COUNT;
 
  // log it
  printk (KERN_INFO BT865_LOGNAME ": [%s] instance destroyed.\n", 
	  tmpName);
}


#ifdef MODULE
int init_module (void)
{
  return 0;
}

void cleanup_module (void)
{
}
#endif


EXPORT_SYMBOL (bt865_new);
EXPORT_SYMBOL (bt865_free);
EXPORT_SYMBOL (bt865_init);
EXPORT_SYMBOL (bt865_set_output_mode);
EXPORT_SYMBOL (bt865_set_blackwhite_mode);
EXPORT_SYMBOL (bt865_set_interlaced_mode);
EXPORT_SYMBOL (bt865_set_75IRE_mode);
EXPORT_SYMBOL (bt865_get_reg);
EXPORT_SYMBOL (bt865_set_reg);
EXPORT_SYMBOL (bt865_get_bits);
EXPORT_SYMBOL (bt865_set_bits);
EXPORT_SYMBOL (bt865_set_macrovision_mode);
EXPORT_SYMBOL (bt865_set_pixel_mode);
