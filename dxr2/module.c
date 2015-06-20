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
 * Overall driver for the dxr2 card
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
#include <linux/types.h>
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
#include <bt865.h>
#include <tc6807af.h>
#include <anp82.h>
#include <zivaDS.h>
#include <vxp524.h>
#include <dxr2.h>
#include <mod_lic.h>

/**
 *
 * IRQ handler function
 *
 * @param irq IRQ number
 * @param vInstance DXR2 instance pointer
 * @param regs Processor regs
 *
 */

extern void dxr2_irq_handler(int irq, void* vInstance, struct pt_regs* regs);

extern ssize_t dxr2_io_write(struct file* filp, const char* buf, size_t count, loff_t* offset);
extern int dxr2_io_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg);
extern int dxr2_io_open(struct inode* inode, struct file* filp);
extern int dxr2_io_release(struct inode* inode, struct file* filp);


extern vxp524_ops_t vxp524_ops_table;
extern anp82_ops_t anp82_ops_table;
extern bt865_ops_t bt865_ops_table;
extern pcm1723_ops_t pcm1723_ops_table;
extern tc6807af_ops_t tc6807af_ops_table;
extern zivaDS_ops_t zivaDS_ops_table;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
static struct file_operations dxr2_fops = {

  NULL,
  NULL,
  dxr2_io_write,
  NULL,
  NULL,
  dxr2_io_ioctl,
  NULL,
  dxr2_io_open,
  NULL,
  dxr2_io_release,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL };
#else

static struct file_operations dxr2_fops = {
  write:   dxr2_io_write,
  ioctl:   dxr2_io_ioctl,
  open:    dxr2_io_open,
  release: dxr2_io_release,
};
#endif




/**
 *
 * Create new DXR2 device
 *
 */

extern dxr2_t* dxr2_new()
{
  dxr2_t* instance;
  int tmp;


  // create new structure
  instance = (dxr2_t*) vmalloc (sizeof (dxr2_t));
  if (!instance)
    return(NULL);

  // OK, initialise the VxP524
  if (!(instance->vxp524Instance = vxp524_new(&vxp524_ops_table, instance)))
    goto err_vxp524;

  // work out base addresses for all the hardware
  instance->vxp524Base = instance->vxp524Instance->base;
  instance->asicBase = instance->vxp524Base + 0x80040;
  instance->zivaDSBase = instance->vxp524Base + 0x80000;
  instance->tc6807afBase = instance->vxp524Base + 0x80080;

  // initialise the ASIC
  dxr2_asic_init(instance);

  // pre-initialise the ZiVA
  if (!(instance->zivaDSInstance = zivaDS_new (&zivaDS_ops_table, instance)))
    goto err_ziva;

  // this works out the type of ziva chip (initially... can be changed
  // during ziva initialisation)
  tmp = dxr2_zivaDS_get_hardware_type(instance);
  if (tmp > 0) {

    instance->zivaDSInstance->zivaDSType = tmp;
  }
    
  // we've not *actually* initialised the ziva yet... cannot do that until
  // firmware is loaded
  instance->zivaDSInitialised = 0;

  // OK, now we can initialise the rest of the hardware
  // initialise the AnP82
  if (!(instance->anp82Instance = anp82_new (&anp82_ops_table, instance)))
    goto err_anp82;


  // since the AnP82 is now initialised, we can safely initialise the VxP524's
  // video processing pipelines
  vxp524_init_video(instance->vxp524Instance);

  // initialise the pcm1723e
  if (!(instance->pcm1723Instance = pcm1723_new (&pcm1723_ops_table, instance)))
    goto err_pcm1723;
  
  // initialise the Bt865
  if (!(instance->bt865Instance = bt865_new (&bt865_ops_table, instance)))
    goto err_bt865;

  // OK, check if the ziva is ZIVADS_TYPE_1
  if (zivaDS_check_type_1(instance->zivaDSInstance)) {
    
    instance->zivaDSInstance->zivaDSType = ZIVADS_TYPE_1;
    if (!(instance->tc6807afInstance = tc6807af_new(&tc6807af_ops_table, instance)))
      goto err_tc6807af;
  }
  else {

    instance->tc6807afInstance = NULL;
  }

  // is the zivaDS type recognised?
  if (instance->zivaDSInstance->zivaDSType == -1) {
    
    printk(KERN_ERR "Unknown zivaDS Type! Defaulting to ZIVADS_TYPE_3.\n");
    printk(KERN_ERR "If you see this message, this is STILL a problem. Please report it.\n");
    instance->zivaDSInstance->zivaDSType = ZIVADS_TYPE_3;
  }

  // nothing in the deferred queue
  instance->deferredCount = 0;

  // semaphore = *1*... i.e. nothing in the critial section
  atomic_set(&(instance->semaphore), 1);

  // initialise other structure members
  #if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,10)
    init_waitqueue_head(&(instance->waitQueue));
  #else
    instance->waitQueue = NULL;
  #endif

  instance->userBuffer = 0;
  instance->userBytesTransferred = 0;
  instance->userBufferSize = 0;
  instance->hliFlag = DXR2_SUBPICTURE_OFF;
  instance->currentZivaAudioDACMode = 0;
  instance->currentSourceVideoFrequency = DXR2_SRC_VIDEO_FREQ_30;
  instance->currentSourceVideoXRes = 720;
  instance->currentSourceVideoYRes = 480;
  instance->currentPlayMode = 0;
  instance->currentAudioVolume = 19;
  instance->currentOutputAspectRatio = DXR2_ASPECTRATIO_4_3;
  instance->currentSlowRate = 0;
  instance->currentAspectRatioMode = 0;
  instance->currentBitstreamType = DXR2_BITSTREAM_TYPE_MPEG_VOB;
  instance->currentVideoStream = 0;
  instance->currentSubPictureStream = 0;
  instance->currentAudioStream = 0;
  instance->currentAudioStreamType = DXR2_STREAM_AUDIO_AC3;
  instance->currentAudioMuteStatus = 0;

  // bus mastering stuff
  instance->bmBuffer = 1;
  instance->writeBuffer = 0;
  instance->bufferCount[0] = 0;
  instance->bufferCount[1] = 0;

  // get BM buffers
  if (!(instance->buffer[0] = __get_free_pages(GFP_KERNEL, DXR2_PAGE_ORDER))) {
    
    goto err_mem1;
  }
  if (!(instance->buffer[1] = __get_free_pages(GFP_KERNEL, DXR2_PAGE_ORDER))) {
    
    goto err_mem2;
  }
  instance->bufferSize[0] = (2*2*2) * PAGE_SIZE;
  instance->bufferSize[1] = (2*2*2) * PAGE_SIZE;

  // hook the IRQ
  if (request_irq(instance->vxp524Instance->pci_dev->irq,
		  dxr2_irq_handler,
		  SA_INTERRUPT | SA_SHIRQ,
		  DXR2_LOGNAME,
		  instance) != 0) {
    
    goto err_irq;
  }

  printk(KERN_ERR "ZivaDS Type=%i\n", instance->zivaDSInstance->zivaDSType);

  // another module use
  MOD_INC_USE_COUNT;

  // OK!
  return(instance);
  
  // error handling!
 err_irq:
  free_pages(instance->buffer[1], DXR2_PAGE_ORDER);
 err_mem2:
  free_pages(instance->buffer[0], DXR2_PAGE_ORDER);
 err_mem1:
  if (instance->tc6807afInstance != NULL) tc6807af_free(instance->tc6807afInstance);
 err_tc6807af:
  bt865_free (instance->bt865Instance);
 err_bt865:
  pcm1723_free (instance->pcm1723Instance);
 err_pcm1723:
  anp82_free (instance->anp82Instance);
 err_anp82:
  zivaDS_free (instance->zivaDSInstance);
 err_ziva:
  vxp524_free (instance->vxp524Instance);
 err_vxp524:
  vfree (instance);
  return(NULL);
}



/**
 *
 * Destroy a dxr2 device (BURNY BURNY!!!!)
 *
 * @param instance DXR2 instance to use
 *
 */

void dxr2_free (dxr2_t* instance)
{
  // ok, shut everything down...
  zivaDS_reset(instance->zivaDSInstance);
  vxp524_bm_reset(instance->vxp524Instance);
  if (instance->zivaDSInitialised) {

    zivaDS_set_audio_attenuation(instance->zivaDSInstance, 0x60);
    zivaDS_clear_video(instance->zivaDSInstance);
  }
  pcm1723_set_mute_mode(instance->pcm1723Instance, DXR2_AUDIO_MUTE_ON);

  // unhook IRQ
  free_irq(instance->vxp524Instance->pci_dev->irq, instance);

  // free BM buffers
  free_pages(instance->buffer[0], DXR2_PAGE_ORDER);
  free_pages(instance->buffer[1], DXR2_PAGE_ORDER);

  // free all the other drivers
  if (instance->tc6807afInstance != NULL) {

    tc6807af_free (instance->tc6807afInstance);
  }
  pcm1723_free (instance->pcm1723Instance);
  zivaDS_free (instance->zivaDSInstance);
  bt865_free (instance->bt865Instance);
  anp82_free (instance->anp82Instance);
  vxp524_free (instance->vxp524Instance);

  // OK, free ourselves!
  vfree(instance);

  // one less module usage
  MOD_DEC_USE_COUNT;
}


/**
 *
 * IOCTL handler. This is temporary, so kI can get the driver out so people
 * can test it etc...
 *
 */

extern int dxr2_io_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{
  char* buffer = (char*) arg;
  dxr2_t* instance = (dxr2_t*) filp->private_data;
  int status;

  // OK, do it!
  switch(cmd) {
  case DXR2_IOC_GET_REGION_CODE:
      
    status = dxr2_ioc_get_region_code(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_OUTPUT_FORMAT:
    
    status = dxr2_ioc_set_tv_output_format(instance, buffer);
    break;

  case DXR2_IOC_SET_SOURCE_VIDEO_FORMAT:

    status = dxr2_ioc_set_source_video_format(instance, buffer);
    break;

  case DXR2_IOC_GET_CAPABILITIES:
    
    status = dxr2_ioc_get_capabilities(instance, buffer);
    break;
    
  case DXR2_IOC_CLEAR_VIDEO:
    
    status = dxr2_ioc_clear_video(instance);
    break;

  case DXR2_IOC_PAUSE:
    
    status = dxr2_ioc_pause(instance);
    break;
    
  case DXR2_IOC_SET_AUDIO_VOLUME:
    
    status = dxr2_ioc_set_audio_volume(instance, buffer);
    break;
    
  case DXR2_IOC_SET_OUTPUT_ASPECT_RATIO:
    
    status = dxr2_ioc_set_output_aspect_ratio(instance, buffer);
    break;
    
  case DXR2_IOC_ABORT:
    
    status = dxr2_ioc_abort(instance);
    break;
    
  case DXR2_IOC_STOP:
    
    status = dxr2_ioc_stop(instance);
    break;
    
  case DXR2_IOC_ENABLE_SUBPICTURE:
    
    status = dxr2_ioc_enable_subpicture(instance, buffer);
    break;
    
  case DXR2_IOC_SLOW_FORWARDS:
    
    status = dxr2_ioc_slow_forwards(instance, buffer);
    break;

  case DXR2_IOC_SLOW_BACKWARDS:
    
    status = dxr2_ioc_slow_backwards(instance, buffer);
    break;

  case DXR2_IOC_SET_SOURCE_ASPECT_RATIO:
    
    status = dxr2_ioc_set_source_aspect_ratio(instance, buffer);
    break;

  case DXR2_IOC_SET_ASPECT_RATIO_MODE:
    
    status = dxr2_ioc_set_aspect_ratio_mode(instance, buffer);
    break;

  case DXR2_IOC_SINGLE_STEP:
    
    status = dxr2_ioc_single_step(instance);
    break;

  case DXR2_IOC_REVERSE_PLAY:
    
    status = dxr2_ioc_reverse_play(instance);
    break;
    
  case DXR2_IOC_SET_SUBPICTURE_PALETTE:
    
    status = dxr2_ioc_set_subpicture_palettes(instance, buffer);
    break;

  case DXR2_IOC_GET_CHALLENGE_KEY:
    
    status = dxr2_ioc_get_challenge_key(instance, buffer);
    break;

  case DXR2_IOC_SEND_CHALLENGE_KEY:
    
    status = dxr2_ioc_send_challenge_key(instance, buffer);
    break;

  case DXR2_IOC_GET_RESPONSE_KEY:
    
    status = dxr2_ioc_get_response_key(instance, buffer);
    break;

  case DXR2_IOC_SEND_RESPONSE_KEY:
    
    status = dxr2_ioc_send_response_key(instance, buffer);
    break;

  case DXR2_IOC_SEND_DISC_KEY:
    
    status = dxr2_ioc_send_disc_key(instance, buffer);
    break;

  case DXR2_IOC_SEND_TITLE_KEY:
    
    status = dxr2_ioc_send_title_key(instance, buffer);
    break;

  case DXR2_IOC_SET_DECRYPTION_MODE:
    
    status = dxr2_ioc_set_decryption_mode(instance, buffer);
    break;

  case DXR2_IOC_INIT_ZIVADS:
    
    status = dxr2_ioc_init_zivaDS(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_MACROVISION_MODE:
    
    status = dxr2_ioc_set_tv_macrovision_mode(instance, buffer);
    break;

  case DXR2_IOC_RESET:
    
    status = dxr2_ioc_reset(instance);
    break;
    
  case DXR2_IOC_SET_BITSTREAM_TYPE:
    
    status = dxr2_ioc_set_bitstream_type(instance, buffer);
    break;
    
  case DXR2_IOC_PLAY:
    
    status = dxr2_ioc_play(instance);
    break;
    
  case DXR2_IOC_GET_STC:
    
    status = dxr2_ioc_get_stc(instance, buffer);
    break;

  case DXR2_IOC_SET_AUDIO_SAMPLE_FREQUENCY:
    
    status = dxr2_ioc_set_audio_sample_freqency(instance, buffer);
    break;

  case DXR2_IOC_SET_AUDIO_DATA_WIDTH:
    
    status = dxr2_ioc_set_audio_data_width(instance, buffer);
    break;
    
  case DXR2_IOC_IEC958_OUTPUT_MODE:
    
    status = dxr2_ioc_iec958_output_mode(instance, buffer);
    break;
    
  case DXR2_IOC_SET_AC3_MODE:
    
    status = dxr2_ioc_set_AC3_mode(instance, buffer);
    break;
    
  case DXR2_IOC_SELECT_AC3_VOICE:
    
    status = dxr2_ioc_select_AC3_voice(instance, buffer);
    break;
    
  case DXR2_IOC_AUDIO_MUTE:
    
    status = dxr2_ioc_audio_mute(instance, buffer);
    break;

  case DXR2_IOC_SET_STEREO_MODE:
    
    status = dxr2_ioc_set_stereo_mode(instance, buffer);
    break;

  case DXR2_IOC_SELECT_STREAM:
    
    status = dxr2_ioc_select_stream(instance, buffer);
    break;
    
  case DXR2_IOC_HIGHLIGHT:
    
    status = dxr2_ioc_highlight(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_BLACKWHITE_MODE:
    
    status = dxr2_ioc_set_tv_blackwhite_mode(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_INTERLACED_MODE:
    
    status = dxr2_ioc_set_tv_interlaced_mode(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_75IRE_MODE:
    
    status = dxr2_ioc_set_tv_75IRE_mode(instance, buffer);
    break;

  case DXR2_IOC_SET_TV_PIXEL_MODE:
    
    status = dxr2_ioc_set_tv_pixel_mode(instance, buffer);
    break;
    
  case DXR2_IOC_SET_OVERLAY_COLOUR:
    
    status = dxr2_ioc_set_overlay_colour(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_GAIN:
    
    status = dxr2_ioc_set_overlay_gain(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_IN_DELAY:
    
    status = dxr2_ioc_set_overlay_in_delay(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_MODE:
    
    status = dxr2_ioc_set_overlay_mode(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_CROPPING:
    
    status = dxr2_ioc_set_overlay_cropping(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_DIMENSION:
    
    status = dxr2_ioc_set_overlay_dimension(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_POSITION:
    
    status = dxr2_ioc_set_overlay_position(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_RATIO:
    
    status = dxr2_ioc_set_overlay_ratio(instance, buffer);
    break;

  case DXR2_IOC_CALCULATE_VGA_PARAMETERS:
    
    status = dxr2_ioc_calculate_vga_parameters(instance, buffer);
    break;

  case DXR2_IOC_SET_VGA_PARAMETERS:
    
    status = dxr2_ioc_set_vga_parameters(instance, buffer);
    break;

  case DXR2_IOC_SET_OVERLAY_PICTURE_CONTROLS:
    
    status = dxr2_ioc_set_overlay_picture_controls(instance, buffer);
    break;

  case DXR2_IOC_FAST_FORWARDS:
    
    status = dxr2_ioc_fast_forwards(instance, buffer);
    break;

  case DXR2_IOC_FAST_BACKWARDS:
    
    status = dxr2_ioc_fast_backwards(instance, buffer);
    break;

  case DXR2_IOC_BUFFERS_EMPTY:
    
    status = dxr2_ioc_buffers_empty(instance, buffer);
    break;

  default:
    
    status = -EINVAL;
  }
  
  return(status);
}
    

extern int dxr2_io_open(struct inode* inode, struct file* filp) 
{
  dxr2_t* instance;

  // create new DXR2 instance & check (increments module usage automatically)
  if ((instance = dxr2_new()) == NULL) {
    
    return(-EIO);
  }

  // OK, remember instance pointer
  filp->private_data = instance;

  // OK!
  return(0);
}

extern int dxr2_io_release(struct inode* inode, struct file* filp) 
{
  dxr2_t* instance = (dxr2_t*) filp->private_data;
  
  // free the DXR2 instance (decrements module usage automatically)
  dxr2_free(instance);

  // OK
  return(0);
}
    
    
    

#ifdef MODULE
int init_module (void)
{
  return(register_chrdev(DXR2_MAJOR, DXR2_LOGNAME, &dxr2_fops));
}

void cleanup_module (void)
{
  unregister_chrdev(DXR2_MAJOR, DXR2_LOGNAME);
}
#endif

