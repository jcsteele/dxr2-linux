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
 * Driver for the Auravision VxP524 Video processor chip
 * High level functions
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <vxp524.h>
#include "contrast_table.h"


static void vxp524_init_pci(vxp524_t* instance);
static void vxp524_init_dram(vxp524_t* instance);
static void vxp524_init_codec(vxp524_t* instance);
static void vxp524_init_capture_pipeline(vxp524_t* instance);
static void vxp524_init_input_pipeline(vxp524_t* instance);
static void vxp524_init_output_pipeline(vxp524_t* instance);



/**
 *
 * Initialise the core VxP524 functions
 *
 * @param instance VxP524 instance to use
 *
 */

int vxp524_init(vxp524_t* instance)
{
  // global reset, memory mode=16bits, flush FIFO
  vxp524_set_reg(instance, VXP524_PCI_GLOBAL_REGS, 0xD0);

  // initialise the DRAM config
  vxp524_init_dram(instance);

  // initialise the codec
  vxp524_init_codec(instance);

  // setup PCI
  vxp524_init_pci(instance);

  // deassert global reset, memory mode=16bits
  vxp524_set_reg(instance, VXP524_PCI_GLOBAL_REGS, 0x60);

  // Delay a bit since we are suspicious that the reset takes a bit of time to complete
  udelay( 1000 ) ;

  // OK
  return(0);
}



/**
 *
 * Initialise the VxP524 video functions
 * Should be called AFTER initialising the AnP82
 *
 * @param instance VxP524 instance to use
 *
 */

extern int vxp524_init_video(vxp524_t* instance)
{
  // global reset, memory mode=16bits, flush FIFO
  vxp524_set_reg(instance, VXP524_PCI_GLOBAL_REGS, 0xD0);

  // initialise the input pipeline
  vxp524_init_input_pipeline(instance);

  // initialise the capture pipeline
  vxp524_init_capture_pipeline(instance);

  // initialise the output pipeline
  vxp524_init_output_pipeline(instance);

  // deassert global reset, memory mode=16bits
  vxp524_set_reg(instance, VXP524_PCI_GLOBAL_REGS, 0x60);

  // Delay a bit since we are suspicious that the reset takes a bit of time to complete
  udelay( 1000 ) ;

  // OK
  return(0);
}


/**
 * 
 * Initialises the PCI setup on the VxP524
 *
 */

static void vxp524_init_pci(vxp524_t* instance)
{
  // init pci
  vxp524_set_reg(instance, VXP524_PCI_BURST_SIZE, 0x3f);
  vxp524_set_reg(instance, VXP524_PCI_FIFO_THRESH, 0xf);
  vxp524_set_bits(instance, VXP524_I2C, 0x80, 0x80);

  // disable all IRQs
  vxp524_set_reg(instance, VXP524_INTR_CTRL, 0);

  // all IRQ status = off
  vxp524_set_reg(instance, VXP524_INTR_STATUS, 0);
}



/**
 *
 * Initialises the DRAM setup on the VxP524
 *
 * @param instance VxP524 instance to use
 *
 */

static void vxp524_init_dram(vxp524_t* instance)
{
  // refresh interval
  vxp524_set_reg(instance, VXP524_REFRESH_CNT, 0x4f);  

  // max refresh queue depth
  vxp524_set_reg(instance, VXP524_REFRESH_PENDING, 0x40);  

  // Memory type EDO, RAS precharge = 4 cycles, RAS->CAS delay = 3 cycles,
  // RAS pulse width = 5 memory cycles, slow memory refresh disabled,
  // refresh enabled
  vxp524_set_reg(instance, VXP524_DRAM_CTRL, 0x8f);
}


/**
 *
 * Initialises the CODEC setup on the VxP524
 *
 * @param instance VxP524 instance to use
 *
 */

static void vxp524_init_codec(vxp524_t* instance)
{
  // set clocks to external source
  vxp524_set_reg(instance, VXP524_CLK_MODE, 0);

  // Codec config = SGS3430 mode
  vxp524_set_reg(instance, VXP524_CODEC_CFG, 3);

  // VIO sync register B
  // HSYNC low, VSYNC low, External decoder is SYNC master,
  // free running sync generator
  vxp524_set_reg(instance, VXP524_VIO_SYNC_B, 0x47);
}



/**
 *
 * Initialises the capture pipeline setup on the VxP524
 *
 * @param instance VxP524 instance to use
 *
 */

static void vxp524_init_capture_pipeline(vxp524_t* instance) 
{
  // turn capture pipeline off
  vxp524_set_bits(instance, VXP524_CAPTURE_CTRL, 3, 2);
}



/**
 *
 * Initialises the input pipeline setup on the VxP524
 *
 * @param instance VxP524 instance to use
 *
 */

static void vxp524_init_input_pipeline(vxp524_t* instance)
{
  int i;

  // reset input pipeline
  vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0x10);

  // TAGN disabled, Demux enabled, Demux order = Cr0, Y0, Cb1, Y1
  // software playback disabled, PCI master cycle OFF
  vxp524_set_reg(instance, VXP524_INPUT_CFG, 2);

  // setup the syncs for the data coming from the decoder. These have nothing
  // to do with the syncs being output to the VGA monitor.
  // HSYNC active low, VSYNC active high, syncs have same timing as data,
  // Video Decoder/MPEG decoder enabled, no HSYNC delay, test mode off
  vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0xEF, 0xA);

  // input data format YUV422, compressed buffer enabled,
  // progressive video input, memory type = 8 bit 422, disable convert chroma
  // sign enable, select VIN
  vxp524_set_reg(instance, VXP524_INPUT_CFG_B, 0x1C);

  // setup picture control registers
  for(i=0; i<256; i++) {
    
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_ADDR, i);
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_LUMA, i);
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_CHROMA, i);
  }
 
  // video buffer layout = single buffer. need to set this to
  // single buffered before changing depth markers
  vxp524_set_reg(instance, VXP524_VIDEO_LAYOUT_CTRL, 0);

  // VIN multi-buffer depth marker.
  vxp524_set_reg(instance, VXP524_VIN_MULTIBUF_DEPTH_A, 0x2f);

  // video buffer layout = 1024x256 double buffer
  vxp524_set_reg(instance, VXP524_VIDEO_LAYOUT_CTRL, 4);

  // no temporal scaling
  vxp524_set_reg(instance, VXP524_TIME_SCALE_CTRL, 0);

  // disable all interrupts
  vxp524_set_reg(instance, VXP524_INTR_CTRL, 0);
  vxp524_set_reg(instance, VXP524_INTR_STATUS, 0);
  
  // horizontal luma filter control enabled, 
  // horizontal chroma filtering enabled, horizontal
  // interpolation enabled, vertical interpolation enabled,
  // luma adaptive encoding disabled, luma small table selected,
  // choma adaptive encoding enabled, chroma small table selected
  vxp524_set_reg(instance, VXP524_FILTER_CTRL, 0x4f);

  // in request level
  vxp524_set_reg(instance, VXP524_IN_REQ_LEVEL, 4);

  // input FIFO high marker = 9, low marker = 7
  vxp524_set_reg(instance, VXP524_INPUT_FIFO, 0x97);

  // un-reset input pipeline
  vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0);
}





/**
 *
 * Initialises the output pipeline setup on the VxP524
 *
 * @param instance VxP524 instance to use
 *
 */

static void vxp524_init_output_pipeline(vxp524_t* instance)
{
  // disable video output, video output reset, disable 
  // viewport registers load
  vxp524_set_reg(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x40);

  // output FIFO low threshold 
  vxp524_set_reg(instance, VXP524_OFIFO_LTHRESH_CTRL, 0x10);

  // output FIFO high threshold
  vxp524_set_reg(instance, VXP524_OFIFO_HTHRESH_CTRL, 0x17);

  // output FIFO active level
  vxp524_set_reg(instance, VXP524_OREQ_ACTIVE_LEVEL, 0x36);

  // disable colour key, disable VGA1555 key, enable display window,
  // output region = key & display window, video in colour key region,
  // video in chroma key region, video in display window region
  vxp524_set_reg(instance, VXP524_DISP_CTRL, 0xe4);
  
  // multibuffer depth control
  vxp524_set_reg(instance, VXP524_MULTIBUF_DEPTH_CTRL, 2);

  // non-interlaced, VSYNC active low, HSYNC active low, 
  // direct colour 16 disabled, direct colour double clock disabled
  // (HSYNC/VSYNC are always active low, since they're coming from the AnP82)
  vxp524_set_reg(instance, VXP524_VGA_CTRL_A, 0);

  // disable 24 bit direct colour, VGA line counting begins with 1, 
  // disable advanced feature connector, disable high speed data path
  vxp524_set_reg(instance, VXP524_VGA_CTRL_B, 0);

  // undelayed clock
  vxp524_set_reg(instance, VXP524_VGA_CTRL_C, 0);
  
  // non inverted VGA clock, falling edge delay = 0 delays, 
  // VGA blank sample delay = -1 clock
  vxp524_set_reg(instance, VXP524_VGA_CTRL_D, 0x08);

  // chroma sign format=EX128, disable horizontal zoom, enable
  // horizontal interpolation, enable colour space conversion, 
  // enable dither, enable chroma interpolation, disable chorma
  // key, disable YUV422 output
  vxp524_set_reg(instance, VXP524_OUTPUT_PROC_CTRL_A, 0x34);

  // disable vertical zoom, enable vertical zoom interpolate, 
  // odd hsync state at VSYNC deassert
  vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 7, 2);

  // disable 565 output, disable internal muxing, enable 24bit output,
  // disable VGA output mux, disable VGA synchronous output delay, 
  // disable VGA asynchronous output delay, disable horizontal zoom by 2
  vxp524_set_reg(instance, VXP524_OUTPUT_PROC_CTRL_C, 0x04);

  // analogue mux key enabled, VIDEON active high, 
  // VIDEON out delay = 6 clocks, enable DAC interface
  vxp524_set_reg(instance, VXP524_VIDEON_CTRL, 0x65);

  // view port starting address = 0
  vxp524_set_reg(instance, VXP524_VP_START_ADDR_A, 0);
  vxp524_set_bits(instance, VXP524_VP_ADDR_WH_B, 0xC0, 0);

  // enable video output, video output UNreset, enable 
  // viewport registers load
  vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0xe0, 0xa0);
}



/**
 *
 * Sets the cropping values for the VxP524
 *
 * @param instance VxP524 instance to use
 * @param left Left cropping in pixels
 * @param right Right in pixels
 * @param top Top cropping in pixels
 * @param bottom Bottom cropping in pixels
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int vxp524_set_overlay_cropping(vxp524_t* instance, 
				       int left, int right, int top, int bottom)
{
  int width;
  int height;
  int status = 0;


  // save the input widths & heights
  width = instance->raw_width - (left + right);
  height = instance->raw_height - (top + bottom);
  if ((width <= 0) || (height <= 0)) {
    
    return(-EINVAL);
  }
  instance->cropped_width = width;
  instance->cropped_height = height;

  // fix the croppings
  right = instance->raw_width - right;
  bottom = instance->raw_height - bottom;
  
  // disable input pipeline
  //VXP524_DISABLE_INPUT_PIPELINE(instance);
  //VXP524_DISABLE_OUTPUT_PIPELINE(instance);

  // set cropping
  vxp524_set_reg(instance, VXP524_HORZ_CROP_LEFT_A, left & 0xff);
  vxp524_set_reg(instance, VXP524_HORZ_CROP_LEFT_B, (left & 0x300) >> 8);
  vxp524_set_reg(instance, VXP524_HORZ_CROP_RIGHT_A, right & 0xff);
  vxp524_set_reg(instance, VXP524_HORZ_CROP_RIGHT_B, (right & 0x300) >> 8);
  vxp524_set_reg(instance, VXP524_VERT_CROP_TOP_A, top & 0xff);
  vxp524_set_reg(instance, VXP524_VERT_CROP_TOP_B, (top & 0x300) >> 8);
  vxp524_set_reg(instance, VXP524_VERT_CROP_BOTTOM_A, bottom & 0xff);
  vxp524_set_reg(instance, VXP524_VERT_CROP_BOTTOM_B, (bottom & 0x300) >> 8);

  // recalculate other overlay parameters
  status = vxp524_set_overlay_dimension(instance, 
  					instance->dw_width, instance->dw_height);


  // reenable input pipeline
  //VXP524_ENABLE_OUTPUT_PIPELINE(instance);
  //VXP524_ENABLE_INPUT_PIPELINE(instance);

  // OK
  return(status);
}



/**
 *
 * Updates the output to follow an onscreen window
 *
 * @param instance VxP524 instance to use
 * @param width width of onscreen window
 * @param height height of onscreen window
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int vxp524_set_overlay_dimension(vxp524_t* instance,
					int display_window_width, 
					int display_window_height)
{ 
  int y_scaling;
  int y_zooming;
  int viewport_height;
  int viewport_width;

  // (any of the r_* variables indicate the value has had the horizontal ratio applied to it)
  int r_display_window_width;    
  int r_x_scaling;
  int r_x_zooming;
  int status =0;


  // adjust horizontal values by current ratio
  r_display_window_width = VXP524_RATIO_ADJUST(instance, display_window_width);

  // check
  if ((r_display_window_width <= 0) ||
      (display_window_height <= 0) ||
      (instance->cropped_width <= 0)) {
    
    return(-EINVAL);
  }
  
  // work out X scaling/zooming/etc values
  if ((r_display_window_width == instance->cropped_width) &&
      (instance->cropped_width <= VXP524_MAX_WIDTH)) {
    
    // no scaling
    r_x_scaling = 1 << 7;
    
    // no zooming
    r_x_zooming = 0;
    
    // viewport width = cropped width
    viewport_width = instance->cropped_width;
  }
  else if ((r_display_window_width < instance->cropped_width) &&
	   (r_display_window_width <= VXP524_MAX_WIDTH)) {

    r_display_window_width +=0;

    // should really be display_window_width / cropped_width, but
    // the scaling values are inverted on the VxP524
    r_x_scaling = (instance->cropped_width << 7) / r_display_window_width;
    if (r_x_scaling == 0) {
      
      return(-EINVAL);
    }

    r_display_window_width -=0;

    // no zooming
    r_x_zooming = 0;
   
    // scale the viewport width
    viewport_width = (instance->cropped_width << 7) / r_x_scaling;
  }
  else if ((r_display_window_width > instance->cropped_width) &&
	   (instance->cropped_width <= VXP524_MAX_WIDTH)) {

    // no scaling
    r_x_scaling = 1 << 7;
    
    // calculate zoom factor
    r_x_zooming = (2048 * instance->cropped_width) / r_display_window_width;
    if (r_x_zooming == 0) {
    
      return(-EINVAL);
    }

    // viewport width = cropped width
    // even though the display_window_width > cropped_width, the viewport_width
    // does not get bigger than the cropped_width.
    viewport_width = instance->cropped_width;
  }
  else { 

    // cropped_width > VXP524_MAX_WIDTH && display_window_width > VXP524_MAX_WIDTH
    
    // because the chip cannot handle a width of > VXP524_MAX_WIDTH, we need
    // to scale the input to width VXP524_MAX_WIDTH, and zoom it to get the correct size.
    r_x_scaling = (instance->cropped_width << 7) / VXP524_MAX_WIDTH;
    
    // Now, we can scale this up from 500 to whatever width we actually need
    r_x_zooming = (2048 * VXP524_MAX_WIDTH) / r_display_window_width;
    if (r_x_zooming == 0) {
    
      return(-EINVAL);
    }

    // viewport width = max width
    viewport_width = VXP524_MAX_WIDTH;
  }
  
    
  // we don't scale any y values, so everything is OK as it is.
  // work out Y scaling/zooming/etc values
  if (display_window_height == instance->cropped_height) {
    
    // no scaling
    y_scaling = 1 << 7;
    
    // no zooming
    y_zooming = 0;

    // viewport height = cropped height
    viewport_height = instance->cropped_height;
  }
  else if (display_window_height < instance->cropped_height) {

    // should really be display_window_height / input_height, but
    // the scaling values are inverted on the VxP524
    y_scaling = (instance->cropped_height << 7) / display_window_height;
    if (y_scaling == 0) {
      
      return(-EINVAL);
    }

    // no zooming
    y_zooming = 0;

    // scale the viewport height
    viewport_height = (instance->cropped_height << 7) / y_scaling;
  }
  else {
    
    // no scaling on input
    y_scaling = 1 << 7;

    // OK, work out zoom value
    y_zooming = (2048 * instance->cropped_height) / display_window_height;
    if (y_zooming == 0) {
      
      return(-EINVAL);
    }

    // viewport height = cropped height
    // even though the display_window_height > cropped_height, the 
    // viewport_height does not get bigger than the cropped_height.
    viewport_height = instance->cropped_height;
  }

  // correct viewport dimensions (viewport_width is in 2 byte words,
  // and both have to have -1 subtracted from them)
  viewport_width = (viewport_width/2)-1;
  viewport_height = viewport_height -1;

  // check
  if (viewport_width <1) {
    
    return(-EINVAL);
  }

  // store (unaltered) values for later
  instance->dw_width = display_window_width;
  instance->dw_height = display_window_height;

  // disable input & output pipelines
  VXP524_DISABLE_VPREGS_LOAD(instance);

  // view port width
  vxp524_set_reg(instance, VXP524_VP_WIDTH_A, viewport_width & 0xff);
  vxp524_set_bits(instance, VXP524_VP_ADDR_WH_B, 0x10, (viewport_width & 0x100)>>4 );

  // view port height
  vxp524_set_reg(instance, VXP524_VP_HEIGHT_A, viewport_height & 0xff);
  vxp524_set_bits(instance, VXP524_VP_ADDR_WH_B, 0x03, (viewport_height & 0x300)>>8 );

  // setup horizontal zoom control
  if (r_x_zooming == 0) {

    // turn zooming off if zoom == 0
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_A, 2, 0);
  }
  else {

    // program zoom parameters & turn zooming on
    vxp524_set_reg(instance, VXP524_OHORZ_ZOOM_CTRL_A, r_x_zooming & 0xff);
    vxp524_set_bits(instance, VXP524_OHORZ_ZOOM_CTRL_B, 7, (r_x_zooming & 0x700) >> 8);
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_A, 2, 2);
  }

  // sort out interpolation
  if ((r_x_zooming >= 512) ||
      (r_x_zooming == 0)) {
  
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_A, 4, 4);
  }
  else {

    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_A, 4, 0);
  }

  // setup vertical zoom control
  if (y_zooming == 0) {

    // turn zooming off if zoom == 0
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 1, 0);
  }
  else {

    // program zoom parameters & turn zooming on
    vxp524_set_reg(instance, VXP524_OVERT_ZOOM_CTRL_A, y_zooming & 0xff);
    vxp524_set_bits(instance, VXP524_OVERT_ZOOM_CTRL_B, 7, (y_zooming & 0x700) >> 8);
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 1, 1);
  }

  // sort out interpolation
  if ((y_zooming >= 512) ||
      (y_zooming == 0)) {

    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 2, 2);
  }
  else {
    
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 2, 0);
  }

  // vertical and horizontal scaling
  vxp524_set_reg(instance, VXP524_HORZ_SCALE_CTRL_A, (r_x_scaling & 0x7f) << 1);
  vxp524_set_reg(instance, VXP524_HORZ_SCALE_CTRL_B, (r_x_scaling & 0x7f80) >> 7);
  vxp524_set_reg(instance, VXP524_VERT_SCALE_CTRL_A, (y_scaling & 0x7f) << 1);
  vxp524_set_reg(instance, VXP524_VERT_SCALE_CTRL_B, (y_scaling & 0x7f80) >> 7);

  // recalculate other overlay parameters
  status = vxp524_set_overlay_position(instance, 
				       instance->xpos, instance->ypos);

  // enable input & output pipelines
  VXP524_ENABLE_VPREGS_LOAD(instance);

  // OK
  return(status);
}



/**
 *
 * Set the position of the overlay window (from Vblank/Hblank)
 *
 * @param instance VxP524 instance to use
 * @param xpos X position (> 10)
 * @param ypos Y position (> 10)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int vxp524_set_overlay_position(vxp524_t* instance, 
				       int xpos, int ypos)
{
  int left;
  int right;
  int top;
  int bottom;


  // check
  if ((xpos < 0) || (ypos < 0)) {
    
    return(-EINVAL);
  }

  // fiddle with the register values a bit
  left = instance->xoffset + VXP524_RATIO_ADJUST(instance, xpos);
  right = instance->xoffset + VXP524_RATIO_ADJUST(instance, (xpos+instance->dw_width));
  top = instance->yoffset + ypos;
  bottom = instance->yoffset + ypos + instance->dw_height;

  // check
  if ((left < 5) || (top < 3)) {
    
    return(-EINVAL);
  }

  // remember position
  instance->xpos = xpos;
  instance->ypos = ypos;

  // disable viewport registers load
  VXP524_DISABLE_VPREGS_LOAD(instance);
  
  // set display window left 
  vxp524_set_reg(instance, VXP524_DWIN_LEFT_A, left & 0xff);
  vxp524_set_bits(instance, VXP524_DWIN_LEFT_RIGHT_B, 0x70, (left & 0x700) >> 4);

  // set display window right
  vxp524_set_reg(instance, VXP524_DWIN_RIGHT_A, right & 0xff);
  vxp524_set_bits(instance, VXP524_DWIN_LEFT_RIGHT_B, 7, (right & 0x700) >> 8);

  // set display window top
  vxp524_set_reg(instance, VXP524_DWIN_TOP_A, top & 0xff);
  vxp524_set_bits(instance, VXP524_DWIN_TOP_BOTTOM_B, 0x70, (top & 0x700) >> 4);

  // set display window bottom
  vxp524_set_reg(instance, VXP524_DWIN_BOTTOM_A, bottom & 0xff);
  vxp524_set_bits(instance, VXP524_DWIN_TOP_BOTTOM_B, 7, (bottom & 0x700) >> 8);

  // set viewport display origin left
  vxp524_set_reg(instance, VXP524_VP_DORG_LEFT_A, (left+1) & 0xff);
  vxp524_set_bits(instance, VXP524_VP_DORG_TOP_LEFT_B, 7, ((left+1) & 0x700) >> 8);

  // set viewport display origin top
  vxp524_set_reg(instance, VXP524_VP_DORG_TOP_A, (top+1) & 0xff);
  vxp524_set_bits(instance, VXP524_VP_DORG_TOP_LEFT_B, 0x70, ((top+1) & 0x700) >> 4);

  // enable viewport registers load
  VXP524_ENABLE_VPREGS_LOAD(instance);

  // OK
  return(0);
}



/**
 *
 * Set the ratio, for fine tuning display ratios
 *
 * @param instance VxP524 instance to use
 * @param ratio X-Ratio adjust (1-2500)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int vxp524_set_overlay_ratio(vxp524_t* instance, int ratio)
{
  int status = 0;


  // check
  if ((ratio < VXP524_MIN_RATIO) || (ratio > VXP524_MAX_RATIO)) {
    
    return(-EINVAL);
  }

  // remember it
  instance->xratio = ratio;

  // recalculate other overlay parameters
  status = vxp524_set_overlay_dimension(instance, instance->dw_width, instance->dw_height);

  // return status
  return(status);
}


/**
 *
 * Turn overlay processing on/off
 *
 * @param instance VxP524 instance to use
 * @param status overlay status. One of VXP524_OVERLAY_*
 *
 * @return 0 on success, <0 on failure
 *
 */
extern int vxp524_set_overlay_mode(vxp524_t* instance, int status)
{
  switch(status) {
  case VXP524_OVERLAY_DISABLED:
    
    // disable input & output pipelines
    vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0x10);    
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x20, 0);
    break;
    
  case VXP524_OVERLAY_ENABLED:
    
    // enable input & output pipelines
    vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0);
    vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x20, 0x20);
    break;
    
  default:

    return(-EINVAL);
  }
  
  // OK
  return(0);
}



/**
 *
 * Set x and y offsets
 *
 * @param instance VxP524 instance to use
 * @param xoffset X offset
 * @param yoffset Y offset
 *
 * @return 0 on success, or <0 on error
 *
 */
extern int vxp524_set_overlay_offsets(vxp524_t* instance, int xoffset, int yoffset)
{
  int status = 0;

  // remember the values for later
  instance->xoffset = xoffset;
  instance->yoffset = yoffset;

  // recalculate the other parameters
  status = vxp524_set_overlay_dimension(instance, 
  					instance->dw_width, instance->dw_height);

  // return status
  return(status);
}




/**
 *
 * Sets the overlay picture controls
 *
 * @param instance VxP524 instance to use
 * @param gamma Gamma control value (0 to 96)
 * @param contrast Contrast control value (-128 to 127)
 * @param brightness Brightness control value (-128 to 127)
 * @param saturation Saturation control value (-128 to 127)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int vxp524_set_overlay_picture_controls(vxp524_t* instance,
					       int gamma, int contrast,
					       int brightness, int saturation) 
{
  int contrast_table_offset;
  int luma[256];
  int chroma[256];
  int tmp;
  int i;


  // check args
  if ((gamma < 0) || (gamma > 96)) {
    
    return(-EINVAL);
  }
  if ((contrast < -128) || (contrast > 127)) {
    
    return(-EINVAL);
  }
  if ((brightness < -128) || (brightness > 127)) {
    
    return(-EINVAL);
  }
  if ((saturation < -128) || (saturation > 127)) {
    
    return(-EINVAL);
  }


  // work out contrast table offset from gamma value
  contrast_table_offset = gamma * 256;


  // work out contrast and brightness values
  if (contrast >= 0) {
    
    for(i=-128; i<128; i++) {
      
      // work out table index
      tmp = (brightness + 128) + ((i*128) / (128 - contrast));
      if (tmp > 255) tmp = 255;
      if (tmp < 0) tmp = 0;

      // retrieve value from contrast table if gamma != 0
      if (gamma != 0) {
	
	tmp = contrast_table[ contrast_table_offset + tmp ];
      }

      // store value
      luma[i+128] = tmp;
    }
  }
  else {
    
    for(i=-128; i<128; i++) {
      
      // work out table index
      tmp = (brightness + 128) + (i - contrast);
      if (tmp > 255) tmp = 255;
      if (tmp < 0) tmp = 0;

      // retrieve value from contrast table if gamma != 0
      if (gamma != 0) {
	
	tmp = contrast_table[ contrast_table_offset + tmp ];
      }

      // store value
      luma[i+128] = tmp;
    }
  }


  // OK, work out saturation values
  if (saturation >= 0) {
    
    for(i=-128; i<128; i++) {
      
      // work out table index
      tmp = (128 + ((i*128) / (128 - saturation)));
      if (tmp > 255) tmp = 255;
      if (tmp < 0) tmp = 0;
      
      // store value
      chroma[i+128] = tmp;
    }
  }
  else {
    
    for(i=-128; i<128; i++) {
      
      // work out table index
      tmp = (128 + (i - saturation));
      if (tmp > 255) tmp = 255;
      if (tmp < 0) tmp = 0;
      
      // store value
      chroma[i+128] = tmp;
    }
  }

  // OK, write luma & chroma control data to VxP
  for(i=0; i< 256; i++) {
    
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_ADDR, i);
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_LUMA, luma[i]);
    vxp524_set_reg(instance, VXP524_PICTURE_CTRL_CHROMA, chroma[i]);
  }

  // OK
  return(0);
}
