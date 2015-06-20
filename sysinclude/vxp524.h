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
 * Include file
 *
 */


#ifndef __VXP524_H__
#define __VXP524_H__

#include <asm/atomic.h>

// *******************************************************************
// useful defines

// full name of the supported thing
#define VXP524_FULLNAME                 "Auravision VxP524"

// log name of the driver
#define VXP524_LOGNAME			"VXP524"

// memory range used by the chip
#define VXP524_MEMRANGE 1024ul * 1024ul

// overlay status
#define VXP524_OVERLAY_DISABLED 0
#define VXP524_OVERLAY_ENABLED  1

// max width of video data
#define VXP524_MAX_WIDTH 500

// ratio value limits
#define VXP524_MIN_RATIO 1
#define VXP524_MAX_RATIO 2500

// enable/disable different bits of the chip
#define VXP524_DISABLE_INPUT_PIPELINE(INSTANCE) if (INSTANCE->inputDisabledCount++ == 0) vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0x10)
#define VXP524_ENABLE_INPUT_PIPELINE(INSTANCE) if (INSTANCE->inputDisabledCount-- == 1) vxp524_set_bits(instance, VXP524_INPUT_CFG_A, 0x10, 0)

#define VXP524_DISABLE_OUTPUT_PIPELINE(INSTANCE) if (INSTANCE->outputDisabledCount++ == 0) vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x40, 0x40)
#define VXP524_ENABLE_OUTPUT_PIPELINE(INSTANCE) if (INSTANCE->outputDisabledCount-- == 1) vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x40, 0)

#define VXP524_DISABLE_VPREGS_LOAD(INSTANCE) if (INSTANCE->vploadDisabledCount++ == 0) vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x80, 0)
#define VXP524_ENABLE_VPREGS_LOAD(INSTANCE) if (INSTANCE->vploadDisabledCount-- == 1) vxp524_set_bits(instance, VXP524_OUTPUT_PROC_CTRL_B, 0x80, 0x80)

// ratio adjustments
#define VXP524_RATIO_ADJUST(INSTANCE,VALUE) (((VALUE) * instance->xratio) / 1000)

// *******************************************************************
// register names

#define VXP524_PCI_DMA_ADDR0		0x00
#define VXP524_PCI_DMA_ADDR1		0x01
#define VXP524_PCI_DMA_ADDR2		0x02
#define VXP524_PCI_DMA_ADDR3		0x03
#define VXP524_PCI_DMA_CNT0		0x04
#define VXP524_PCI_DMA_CNT1		0x05
#define VXP524_PCI_DMA_CNT2		0x06
#define VXP524_PCI_BURST_SIZE		0x07
#define VXP524_PCI_FIFO_THRESH		0x08
#define VXP524_PCI_GLOBAL_REGS		0x09
#define VXP524_REFRESH_CNT		0x12
#define VXP524_REFRESH_PENDING		0x13
#define VXP524_DRAM_CTRL		0x17
#define VXP524_CLK_MODE			0x1f
#define VXP524_CLK_FREQ_LOW		0x27
#define VXP524_CLK_FREQ_HIGH		0x28
#define VXP524_VIO_HS_WIDTH		0x29
#define VXP524_VIO_VS_HEIGHT		0x2a
#define VXP524_VIO_WIDTH_A		0x2b
#define VXP524_VIO_HEIGHT_A		0x2c
#define VXP524_VIO_SYNC_B		0x2d
#define VXP524_CODEC_CFG		0x2f
#define VXP524_I2C			0x34
#define VXP524_INPUT_CFG		0x35
#define VXP524_PLL_CTRL_A		0x36
#define VXP524_PLL_CTRL_B		0x37
#define VXP524_INPUT_CFG_A		0x38
#define VXP524_INPUT_CFG_B		0x39
#define VXP524_PCI_SRC_WIDTH_A		0x3a
#define VXP524_PCI_SRC_WIDTH_B		0x3b
#define VXP524_PCI_SRC_HEIGHT_A		0x3c
#define VXP524_PCI_SRC_HEIGHT_B		0x3d
#define VXP524_HORZ_CROP_LEFT_A		0x40
#define VXP524_HORZ_CROP_LEFT_B		0x41
#define VXP524_HORZ_CROP_RIGHT_A	0x44
#define VXP524_HORZ_CROP_RIGHT_B	0x45
#define VXP524_VERT_CROP_TOP_A		0x48
#define VXP524_VERT_CROP_TOP_B		0x49
#define VXP524_VERT_CROP_BOTTOM_A	0x4c
#define VXP524_VERT_CROP_BOTTOM_B	0x4d
#define VXP524_FILTER_CTRL		0x50
#define VXP524_HORZ_SCALE_CTRL_A	0x54
#define VXP524_HORZ_SCALE_CTRL_B	0x55
#define VXP524_VERT_SCALE_CTRL_A	0x5b
#define VXP524_VERT_SCALE_CTRL_B	0x5c
#define VXP524_PICTURE_CTRL_ADDR	0x5d
#define VXP524_PICTURE_CTRL_LUMA	0x5e
#define VXP524_PICTURE_CTRL_CHROMA	0x5f
#define VXP524_INTR_CTRL		0x60
#define VXP524_INTR_STATUS		0x61
#define VXP524_IN_REQ_LEVEL		0x67
#define VXP524_VIN_MULTIBUF_DEPTH_A	0x68
#define VXP524_TIME_SCALE_CTRL		0x6d
#define VXP524_VIDEO_LAYOUT_CTRL	0x73
#define VXP524_INPUT_FIFO		0x78
#define VXP524_CAPTURE_CTRL		0x80
#define VXP524_CAPTURE_VP_WIDTH_A	0x84
#define VXP524_CAPTURE_VP_WIDTH_B	0x85
#define VXP524_CAPTURE_VP_HEIGHT_A	0x86
#define VXP524_CAPTURE_VP_HEIGHT_B	0x87
#define VXP524_CAPTURE_MULTIBUF_DEPTH	0x8a
#define VXP524_OFIFO_LTHRESH_CTRL	0x8f
#define VXP524_OFIFO_HTHRESH_CTRL	0x90
#define VXP524_OREQ_ACTIVE_LEVEL	0x91
#define VXP524_DISP_CTRL		0x92
#define VXP524_MULTIBUF_DEPTH_CTRL	0x93
#define VXP524_VGA_CTRL_A		0x94
#define VXP524_VGA_CTRL_B		0x95
#define VXP524_OUTPUT_PROC_CTRL_A	0x96
#define VXP524_OUTPUT_PROC_CTRL_B	0x97
#define VXP524_OUTPUT_PROC_CTRL_C	0x98
#define VXP524_CHROMA_KEY_CTRL		0x9b
#define VXP524_KEY_COLOR		0x9c
#define VXP524_KEY_COLOR_HI		0x9d
#define VXP524_KEY_COLOR_HI2		0x9e
#define VXP524_VIDEON_CTRL		0x9f
#define VXP524_VP_START_ADDR_A		0xa0
#define VXP524_VP_WIDTH_A		0xa4
#define VXP524_VP_HEIGHT_A		0xa6
#define VXP524_VP_ADDR_WH_B		0xa7
#define VXP524_VP_DORG_TOP_A		0xa8
#define VXP524_VP_DORG_LEFT_A		0xaa
#define VXP524_VP_DORG_TOP_LEFT_B	0xab
#define VXP524_DWIN_LEFT_A		0xb0
#define VXP524_DWIN_RIGHT_A		0xb4
#define VXP524_DWIN_LEFT_RIGHT_B	0xb5
#define VXP524_DWIN_TOP_A		0xb8
#define VXP524_DWIN_BOTTOM_A		0xbc
#define VXP524_DWIN_TOP_BOTTOM_B	0xbd
#define VXP524_OVERT_ZOOM_CTRL_A	0xc0
#define VXP524_OVERT_ZOOM_CTRL_B	0xc1
#define VXP524_OHORZ_ZOOM_CTRL_A	0xc4
#define VXP524_OHORZ_ZOOM_CTRL_B	0xc5
#define VXP524_HORZ_COORD_OFFSET	0xc6
#define VXP524_VERT_COORD_OFFSET	0xc7
#define VXP524_GPO_DATA1		0xcc
#define VXP524_VGA_CTRL_C		0xcd
#define VXP524_VGA_CTRL_D		0xce
#define VXP524_MAX_HCOUNT1		0xd0
#define VXP524_MAX_HCOUNT2		0xd1
#define VXP524_INTR_CTRL2		0xd2
#define VXP524_INTR_STATUS2		0xd3
#define VXP524_VGA_DB0			0xd5
#define VXP524_VGA_DB1			0xd6
#define VXP524_VGA_DB2			0xd7
#define VXP524_CHROMA_KEY_RED_LOW	0xd8
#define VXP524_CHROMA_KEY_RED_HIGH	0xd9
#define VXP524_CHROMA_KEY_GREEN_LOW	0xda
#define VXP524_CHROMA_KEY_GREEN_HIGH	0xdb
#define VXP524_CHROMA_KEY_BLUE_LOW	0xdc
#define VXP524_CHROMA_KEY_BLUE_HIGH	0xdd


// *******************************************************************
// Structures

typedef struct _vxp524_ops_t vxp524_ops_t;
typedef struct _vxp524_t vxp524_t;

// generic driver structure
struct _vxp524_t {

  struct pci_dev* pci_dev;
  vxp524_ops_t* ops;
  void* data;
  unsigned long base;
  unsigned long mem;
  unsigned long memlen;

  // RAW input width/height (before cropping)
  int raw_width; // without ratioing
  int raw_height;

  // input width/height after cropping
  int cropped_width; // without ratioing
  int cropped_height;

  // x/y pos of the display window
  int xpos; // without rationing
  int ypos;

  // display window width/height
  int dw_width; // without ratioing
  int dw_height;

  // screen offsets 
  int xoffset;
  int yoffset;

  // horizontal ratio adjust par#ameter;
  int xratio;

  // feature disabled counter
  int inputDisabledCount;
  int outputDisabledCount;
  int vploadDisabledCount;
  
  // is BM in use?
  int bmInUse;
};


// lowlevel access operations
struct _vxp524_ops_t {

  char name[32];
};



// *******************************************************************
// function declarations


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions



/**
 *
 * Create new vxp524 driver instance
 *
 * @param pci_dev PCI device structure
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern vxp524_t* vxp524_new(vxp524_ops_t *ops, void *data);


/**
 *
 * Destroy a vxp524 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void vxp524_free(vxp524_t* instance);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions



/**
 *
 * Initlialises the VXP524
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on error
 *
 */

extern int vxp524_init(vxp524_t* instance);


/**
 *
 * Initialise the VxP524 video functions
 * Should be called AFTER initialising the AnP82
 *
 * @param instance VxP524 instance to use
 *
 */

extern int vxp524_init_video(vxp524_t* instance);


/**
 *
 * Enable memory mapped access & IRQ for the VxP524
 *
 * @param instance instance to do this for
 * 
 */

extern void vxp524_enable_mem(vxp524_t* instance);


/**
 *
 * Disable memory mapped access & IRQ for the VxP524
 *
 * @param instance instance to do this for
 * 
 */

extern void vxp524_disable_mem(vxp524_t* instance);




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
				       int left, int right, int top, int bottom);


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
					int display_window_height);

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
				       int xpos, int ypos);


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

extern int vxp524_set_overlay_ratio(vxp524_t* instance, int ratio);



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
extern int vxp524_set_overlay_mode(vxp524_t* instance, int status);


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
extern int vxp524_set_overlay_offsets(vxp524_t* instance, int xoffset, int yoffset);



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
					       int brightness, int saturation);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Low level i2s functions



/**
 *
 * Initialise the i2s bus
 *
 * @param instance VxP524 instance to use
 */

extern void vxp524_i2s_init(vxp524_t* instance);



/**
 * 
 * Close the i2s bus after use
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_close(vxp524_t* instance);



/**
 *
 * Set the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_set_sda(vxp524_t* instance);



/**
 *
 * Clear the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_clear_sda(vxp524_t* instance);



/**
 *
 * Tristate the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_tri_sda(vxp524_t* instance);



/**
 *
 * Un-Tristate the DATA line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_untri_sda(vxp524_t* instance);



/**
 *
 * Get the current value of the DATA line on the i2s bus
 *
 * @param instance the VxP524 instance to use
 *
 */

extern int vxp524_i2s_get_sda(vxp524_t* instance);



/**
 *
 * Set the CLOCK line on the i2s bus
 *
 * @param instance VxP524 instance to use
 *
 */

extern void vxp524_i2s_set_scl(vxp524_t* instance);



/**
 *
 * Clear the clock line on the i2s bus
 *
 * @param instance the VxP524 instance to use
 *
 */

extern void vxp524_i2s_clear_scl(vxp524_t* instance);



/**
 *
 * Reads the DATA line until it becomes set. Max. count reads
 *
 * @param instance the VxP524 instance to use
 * @param count Maximum number of reads
 *
 * @return 0 on success, -ETIMEDOUT if the DATA line never became set
 *
 */

extern int vxp524_i2s_wait_till_sda_set(vxp524_t* instance, int count);



/**
 *
 * Reads the CLOCK line until it becomes set. Max. count reads
 *
 * @param instance the VxP524 instance to use
 * @param count Maximum number of reads
 *
 * @return 0 on success, -ETIMEDOUT if the CLOCK line never became set
 *
 */

extern int vxp524_i2s_wait_till_scl_set(vxp524_t* instance, int count);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// i2c functions


/**
 *
 * Read a value from a register on a device connected to the i2c bus
 *
 * @param instance VXP524 instance to use
 * @param devId Device id to read from
 * @param reg Register on the device to read from
 *
 * @return Value on success (>=0), or <0 on error
 *
 */

extern int vxp524_i2c_read_reg(vxp524_t* instance, int devId, int reg);


/**
 *
 * Write a value to a register on a device connected to the i2c bus
 *
 * @param instance VXP524 instance to use
 * @param devId Device id to write to
 * @param reg Register on the device to write to
 * @param val Value to write
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int vxp524_i2c_write_reg(vxp524_t* instance, int devId, int reg, int val);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// vxp524 Bus mastering routines



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
			      int irqEndFlag);


/**
 *
 * Flush the vxp BM system
 *
 * @param instance Vxp524 instance to use
 *
 */

extern void vxp524_bm_flush(vxp524_t* instance);


/**
 *
 * Reset the Vxp524 BM system
 *
 * @param instance Vxp524 instance to use
 *
 */

extern void vxp524_bm_reset(vxp524_t* instance);


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

extern int vxp524_bm_completed(vxp524_t* instance, int checkType);



/**
 *
 * Checks if BM in use, and sets BM in use flag if it wasn't
 *
 * @param instance vxp524 instance to use
 *
 * @return 0 on success, -EBUSY if BM is already in use
 *
 */

extern int vxp524_bm_check_status(vxp524_t* instance);



/**
 *
 * Mark BM as no longer in use
 *
 */

extern int vxp524_bm_not_in_use(vxp524_t* instance);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Register get/set functions


/**
 *
 * Get (8bit) register from the Vxp524
 * This retrives the value: [reg] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg(vxp524_t* instance, int reg);


/**
 *
 * Set (8bit) register on the Vxp524
 * This sets the value: [reg] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg(vxp524_t* instance, int reg, int val);



/**
 *
 * Get (16bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg16(vxp524_t* instance, int reg);


/**
 *
 * Set (16bit) register on the Vxp524
 * This sets the value: [reg], [reg+1] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg16(vxp524_t* instance, int reg, int val);



/**
 *
 * Get (24bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) | ([reg+2]<<16) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg24(vxp524_t* instance, int reg);



/**
 *
 * Set (24bit) register on the Vxp524
 * This sets the value: [reg], [reg+1], [reg+2] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg24(vxp524_t* instance, int reg, int val);


/**
 *
 * Get (32bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) | ([reg+2]<<16) | ([reg+3]<<24) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg32(vxp524_t* instance, int reg);



/**
 *
 * Set (32bit) register on the Vxp524
 * This sets the value: [reg], [reg+1], [reg+2], [reg+3] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg32(vxp524_t* instance, int reg, int val);



/**
 *
 * Get specified bitmask of an (8bit) register from vxp524
 *
 * @param instance Instance of the vxp524 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int vxp524_get_bits(vxp524_t* instance, int reg, int bitmask);



/**
 *
 * Set specified bits of an (8bit) register on vxp524
 *
 * @param instance Instance of the vxp524 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void vxp524_set_bits(vxp524_t* instance, int reg, int bitmask, int valuemask);



#endif
