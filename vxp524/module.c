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
 * Driver for the Auravision VxP524 Video processor chip
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

#include <vxp524.h>
#include <mod_lic.h>

/**
 *
 * Create new vxp524 driver instance
 *
 * @param pci_dev PCI device structure
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

vxp524_t* vxp524_new (vxp524_ops_t* ops, void *data)
{
  vxp524_t* instance;
  struct pci_dev* pci_dev = NULL;
  u32 base_address;



  // validate ops
  if (ops == NULL)
    return NULL;

  // create new structure
  instance = (vxp524_t*) vmalloc (sizeof(vxp524_t));
  if (!instance)
    return NULL;

  // copy data across
  instance->pci_dev = pci_dev;
  instance->ops = ops;
  instance->data = data;

  // OK, probe for the device
  if (!pci_present() ||
      !(instance->pci_dev = pci_find_device (PCI_VENDOR_ID_AURAVISION,
					     PCI_DEVICE_ID_AURAVISION_VXP524,
					     pci_dev))) {
    vfree(instance);
    printk(KERN_ERR VXP524_LOGNAME ": PCI not present, or VxP524 not detected\n");
    return(NULL);
  }

  // OK, remap the thing
  //   instance->mem = instance->pci_dev->base_address[0] & PCI_BASE_ADDRESS_MEM_MASK;
  pci_read_config_dword(instance->pci_dev, PCI_BASE_ADDRESS_0, &base_address);
  base_address &= PCI_BASE_ADDRESS_MEM_MASK;
  instance->mem = base_address;

  instance->memlen = VXP524_MEMRANGE; 
  if (!(instance->base = (unsigned long) ioremap(instance->mem, instance->memlen))) {

    vfree(instance);
    printk (KERN_ERR VXP524_LOGNAME ": unable to remap I/O\n");
    return(NULL);
  }

  // Bus mastering not currently in use
  instance->bmInUse = 0;

  // enable PCI memory mapping & IRQ
  vxp524_enable_mem(instance);

  // init the chip
  vxp524_init(instance);

  // nothing is disabled
  instance->inputDisabledCount = 0;
  instance->outputDisabledCount = 0;
  instance->vploadDisabledCount = 0;
  
  // setup extra fields in the device structure
  instance->raw_width = 720;
  instance->raw_height = 609;
  instance->cropped_width = 200;
  instance->cropped_height = 200;
  instance->xpos = 100;
  instance->ypos = 100;
  instance->xoffset = 0;
  instance->yoffset = 0;
  instance->dw_width = 100;
  instance->dw_width = 100;
  instance->xratio = 1000;

  vxp524_set_overlay_cropping(instance, 20, 20, 20, 20);
  vxp524_set_overlay_position(instance, 100, 100);
  vxp524_set_overlay_dimension(instance, 100, 100);

  // success message!
  printk (KERN_INFO VXP524_LOGNAME ": [%s] mem 0x%lx-0x%lx, irq %d\n",
	  instance->ops->name,
	  instance->mem, 
	  instance->mem + instance->memlen - 1,
	  instance->pci_dev->irq);
  
  // another module use
  MOD_INC_USE_COUNT;

  // return instance
  return instance;
}



/**
 *
 * Destroy a vxp524 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void vxp524_free(vxp524_t* instance)
{
  char* tmpName = instance->ops->name;

  // disable overlay
  vxp524_set_overlay_mode(instance, VXP524_OVERLAY_DISABLED);

  // disable mem
  vxp524_disable_mem(instance);

  // unmap HW address
  iounmap((char *) instance->base);

  // free instance
  vfree(instance);

  // one less module use
  MOD_DEC_USE_COUNT;

  // log it
  printk (KERN_INFO VXP524_LOGNAME ": [%s] instance destroyed.\n", 
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




EXPORT_SYMBOL(vxp524_new);
EXPORT_SYMBOL(vxp524_free);
EXPORT_SYMBOL(vxp524_disable_mem);
EXPORT_SYMBOL(vxp524_enable_mem);
EXPORT_SYMBOL(vxp524_get_reg);
EXPORT_SYMBOL(vxp524_set_reg);
EXPORT_SYMBOL(vxp524_get_reg16);
EXPORT_SYMBOL(vxp524_set_reg16);
EXPORT_SYMBOL(vxp524_get_reg24);
EXPORT_SYMBOL(vxp524_set_reg24);
EXPORT_SYMBOL(vxp524_get_reg32);
EXPORT_SYMBOL(vxp524_set_reg32);
EXPORT_SYMBOL(vxp524_get_bits);
EXPORT_SYMBOL(vxp524_set_bits);
EXPORT_SYMBOL(vxp524_i2s_init);
EXPORT_SYMBOL(vxp524_i2s_close);
EXPORT_SYMBOL(vxp524_i2s_set_sda);
EXPORT_SYMBOL(vxp524_i2s_clear_sda);
EXPORT_SYMBOL(vxp524_i2s_tri_sda);
EXPORT_SYMBOL(vxp524_i2s_untri_sda);
EXPORT_SYMBOL(vxp524_i2s_get_sda);
EXPORT_SYMBOL(vxp524_i2s_set_scl);
EXPORT_SYMBOL(vxp524_i2s_clear_scl);
EXPORT_SYMBOL(vxp524_i2s_wait_till_sda_set);
EXPORT_SYMBOL(vxp524_i2s_wait_till_scl_set);
EXPORT_SYMBOL(vxp524_i2c_read_reg);
EXPORT_SYMBOL(vxp524_i2c_write_reg);
EXPORT_SYMBOL(vxp524_bm_send_data);
EXPORT_SYMBOL(vxp524_bm_flush);
EXPORT_SYMBOL(vxp524_bm_reset);
EXPORT_SYMBOL(vxp524_bm_completed);
EXPORT_SYMBOL(vxp524_bm_check_status);
EXPORT_SYMBOL(vxp524_bm_not_in_use);
EXPORT_SYMBOL(vxp524_init);
EXPORT_SYMBOL(vxp524_init_video);
EXPORT_SYMBOL(vxp524_set_overlay_cropping);
EXPORT_SYMBOL(vxp524_set_overlay_dimension);
EXPORT_SYMBOL(vxp524_set_overlay_position);
EXPORT_SYMBOL(vxp524_set_overlay_ratio);
EXPORT_SYMBOL(vxp524_set_overlay_mode);
EXPORT_SYMBOL(vxp524_set_overlay_offsets);
EXPORT_SYMBOL(vxp524_set_overlay_picture_controls);
