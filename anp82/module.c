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
 * Driver for the SkyTune/Auravision AnP82 VGA overlay chip
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

#include <anp82.h>
#include <mod_lic.h>

/**
 *
 * Create new AnP82 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern anp82_t* anp82_new (anp82_ops_t* ops, void* data)
{
  anp82_t* instance;
  int val;
  
  // validate ops
  if (ops == NULL)
    return NULL;
  
  // create new structure
  instance = (anp82_t*) vmalloc(sizeof(anp82_t));
  if (!instance)
    return NULL;

  // copy necessary data in
  instance->ops = ops;
  instance->data = data;

  // check the chip is there and is OK
  if ((val = anp82_get_reg(instance, 1)) < 0) {

    printk (KERN_ERR ANP82_LOGNAME ": [%s]: initialisation error!.\n",
	    instance->ops->name);
    vfree(instance);
    return(NULL);
  }

  // initialise it
  anp82_init(instance);

  // report OK
  printk (KERN_INFO ANP82_LOGNAME ": [%s] instance created (anp%i revision %c).\n",
	  instance->ops->name, 
	  instance->chip_id + 80,
	  'A' + (val >>6));
  
  // OK, another module usage
  MOD_INC_USE_COUNT;
  
  // return new structure
  return instance;
}



/**
 *
 * Destroy an Anp82 driver instance
 *
 * @param instance The instance to destroy
 *
 */

void anp82_free (anp82_t* instance)
{
  char* tmpName = instance->ops->name;

  // overlay off
  anp82_set_overlay_mode(instance, ANP82_OVERLAY_DISABLED);
  
  // shutdown anp82
  anp82_shutdown(instance);
  
  // free instance
  vfree(instance);
  
  // one less module usage
  MOD_DEC_USE_COUNT;

  // log it
  printk (KERN_INFO ANP82_LOGNAME ": [%s] instance destroyed.\n", 
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


EXPORT_SYMBOL(anp82_new);
EXPORT_SYMBOL(anp82_free);
EXPORT_SYMBOL(anp82_init);
EXPORT_SYMBOL(anp82_get_reg);
EXPORT_SYMBOL(anp82_set_reg);
EXPORT_SYMBOL(anp82_get_bits);
EXPORT_SYMBOL(anp82_set_bits);
EXPORT_SYMBOL(anp82_set_overlay_colour);
EXPORT_SYMBOL(anp82_set_gain);
EXPORT_SYMBOL(anp82_set_in_delay);
EXPORT_SYMBOL(anp82_set_overlay_mode);
EXPORT_SYMBOL(anp82_measure_horizontal_offset);
EXPORT_SYMBOL(anp82_measure_vertical_offset);
EXPORT_SYMBOL(anp82_measure_hsync_polarity);
EXPORT_SYMBOL(anp82_measure_vsync_polarity);
EXPORT_SYMBOL(anp82_measure_video_blank);
EXPORT_SYMBOL(anp82_measure_horizontal_ratio);
EXPORT_SYMBOL(anp82_set_video_blank);
EXPORT_SYMBOL(anp82_set_sync_polarities);
