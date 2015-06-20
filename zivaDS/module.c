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
 * Driver for the C-Cube Ziva-DS MPEG decoder chip
 * Driver Maintenance functions
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

#include <zivaDS.h>
#include <mod_lic.h>

// internal driver functions that are in different files
extern int zivaDS_validate(zivaDS_t* instance);


/**
 *
 * Create new zivaDS driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern zivaDS_t* zivaDS_new(zivaDS_ops_t* ops, void *data)
{
  zivaDS_t* instance;
  
  // validate ops
  if (ops == NULL)
    return NULL;
  
  // create new structure
  instance = (zivaDS_t*) vmalloc (sizeof(zivaDS_t));
  if (!instance)
    return NULL;
  
  // copy necessary data
  instance->ops = ops;
  instance->data = data;

  // validate the ziva... check it is there
  if (zivaDS_validate(instance) < 0) {
    
    vfree(instance);
    return(NULL);
  }
  
  // unknown ziva chip type
  instance->zivaDSType = -1;

  // ints are not enabled
  instance->intEnabledFlag = 0;

  // report OK
  printk (KERN_INFO ZIVADS_LOGNAME "(%s): instance created.\n",
	  instance->ops->name);

  // OK, another module usage
  MOD_INC_USE_COUNT;

  // return new structure
  return instance;
}



/**
 *
 * Destroy a zivaDS driver instance
 *
 * @param instance The instance to destroy
 *
 */

void zivaDS_free (zivaDS_t* instance)
{
  char* tmpName = instance->ops->name;
  
  // free instance
  vfree(instance);
  
  // one less module usage
  MOD_DEC_USE_COUNT;

  // log it
  printk (KERN_INFO ZIVADS_LOGNAME "(%s): instance destroyed.\n", 
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


EXPORT_SYMBOL(zivaDS_get_reg);
EXPORT_SYMBOL(zivaDS_set_reg);
EXPORT_SYMBOL(zivaDS_get_bits);
EXPORT_SYMBOL(zivaDS_set_bits);
EXPORT_SYMBOL(zivaDS_get_mem);
EXPORT_SYMBOL(zivaDS_set_mem);
EXPORT_SYMBOL(zivaDS_set_decryption_mode);
EXPORT_SYMBOL(zivaDS_send_challenge_key);
EXPORT_SYMBOL(zivaDS_get_challenge_key);
EXPORT_SYMBOL(zivaDS_send_response_key);
EXPORT_SYMBOL(zivaDS_get_response_key);
EXPORT_SYMBOL(zivaDS_send_disc_key_part1);
EXPORT_SYMBOL(zivaDS_send_disc_key_part2);
EXPORT_SYMBOL(zivaDS_send_title_key);
EXPORT_SYMBOL(zivaDS_free);
EXPORT_SYMBOL(zivaDS_new);
EXPORT_SYMBOL(zivaDS_get_css_flags);
EXPORT_SYMBOL(zivaDS_restore_css_flags);
EXPORT_SYMBOL(zivaDS_set_css_mode);
EXPORT_SYMBOL(zivaDS_detect);
EXPORT_SYMBOL(zivaDS_enable_subpicture);
EXPORT_SYMBOL(zivaDS_abort);
EXPORT_SYMBOL(zivaDS_set_output_aspect_ratio);
EXPORT_SYMBOL(zivaDS_set_source_aspect_ratio);
EXPORT_SYMBOL(zivaDS_set_audio_volume);
EXPORT_SYMBOL(zivaDS_pause);
EXPORT_SYMBOL(zivaDS_clear_video);
EXPORT_SYMBOL(zivaDS_slow_forwards);
EXPORT_SYMBOL(zivaDS_slow_backwards);
EXPORT_SYMBOL(zivaDS_set_aspect_ratio_mode);
EXPORT_SYMBOL(zivaDS_fast_forwards);
EXPORT_SYMBOL(zivaDS_fast_backwards);
EXPORT_SYMBOL(zivaDS_single_step);
EXPORT_SYMBOL(zivaDS_reverse_play);
EXPORT_SYMBOL(zivaDS_set_subpicture_palettes);
EXPORT_SYMBOL(zivaDS_init);
EXPORT_SYMBOL(zivaDS_select_stream);
EXPORT_SYMBOL(zivaDS_play);
EXPORT_SYMBOL(zivaDS_resume);
EXPORT_SYMBOL(zivaDS_reset);
EXPORT_SYMBOL(zivaDS_new_play_mode);
EXPORT_SYMBOL(zivaDS_set_bitstream_type);
EXPORT_SYMBOL(zivaDS_set_source_video_frequency);
EXPORT_SYMBOL(zivaDS_get_mrc_pic_stc);
EXPORT_SYMBOL(zivaDS_get_mrc_pic_pts);
EXPORT_SYMBOL(zivaDS_set_iec958_output_mode);
EXPORT_SYMBOL(zivaDS_set_AC3_mode);
EXPORT_SYMBOL(zivaDS_select_AC3_voice);
EXPORT_SYMBOL(zivaDS_set_audio_attenuation);
EXPORT_SYMBOL(zivaDS_setup_audio_dac);
EXPORT_SYMBOL(zivaDS_command);
EXPORT_SYMBOL(zivaDS_is_int_enabled);
EXPORT_SYMBOL(zivaDS_enable_int);
EXPORT_SYMBOL(zivaDS_get_int_status);
EXPORT_SYMBOL(zivaDS_highlight);
EXPORT_SYMBOL(zivaDS_wait_for_HLI_int);
EXPORT_SYMBOL(zivaDS_check_type_1);
EXPORT_SYMBOL(zivaDS_set_audio_clock_frequency);
