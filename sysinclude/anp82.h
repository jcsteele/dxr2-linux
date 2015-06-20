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
 * Driver for the SkyTune/Auravision AnP82 VGA overlay chip
 *
 */


#ifndef __ANP82_H__
#define __ANP82_H__



// *******************************************************************
// useful defines

// Full name of the chip
#define ANP82_FULLNAME                  "SkyTune/Auravision AnP82 VGA Overlay"

// Log name of the driver
#define ANP82_LOGNAME			"ANP82"

// Chip ID for serial bus
#define ANP82_CHIPID			0x28

// overlay modes
#define ANP82_OVERLAY_DISABLED           0
#define ANP82_OVERLAY_WINDOW_KEY         1
#define ANP82_OVERLAY_COLOUR_KEY         2
#define ANP82_OVERLAY_WINDOW_COLOUR_KEY  3

// chip types
#define ANP_TYPE_81                      0
#define ANP_TYPE_82                      1
#define ANP_TYPE_82_REVC                 2
#define ANP_TYPE_83                      3

// portions of signals to measure
#define ANP82_MEASURE_LOW                0
#define ANP82_MEASURE_HIGH               1

// sync signals
#define ANP82_SYNC_ACTIVE_LOW            0
#define ANP82_SYNC_ACTIVE_HIGH           1

// to retrieve the timer value
#define ANP82_GET_TIMER(INSTANCE) (anp82_get_reg(INSTANCE, ANP82_TCNTR) | (((anp82_get_reg(INSTANCE, ANP82_CNTCTRL) & 0xf0) << 4)))



// *******************************************************************
// register names

#define ANP82_CKREDH			0x02
#define ANP82_CKREDL			0x03
#define ANP82_CKGREENH			0x04
#define ANP82_CKGREENL			0x05
#define ANP82_CKBLUEH			0x06
#define ANP82_CKBLUEL			0x07
#define ANP82_CKOFFSET			0x08
#define ANP82_DACCTRL			0x0e
#define ANP82_PCLKOUT			0x0f
#define ANP82_ALPHAMIX			0x10
#define ANP82_VBSL			0x11
#define ANP82_VBSH			0x12
#define ANP82_VBSW			0x13
#define ANP82_RDACL			0x14
#define ANP82_GDACL			0x15
#define ANP82_BDACL			0x16
#define ANP82_VDACCG			0x17
#define ANP82_VDACCR			0x18
#define ANP82_FADETIME			0x19
#define ANP82_KEYSTAT			0x1a
#define ANP82_KEYCTRL			0x1b
#define ANP82_PWRSAVE			0x1c
#define ANP82_GPCR			0x1d
#define ANP82_GPDR			0x1e
#define ANP82_CTRL			0x1f
#define ANP82_CNTCTRL			0x21
#define ANP82_TCNTR			0x22
#define ANP82_CNTCTRLB			0x23
#define ANP82_PWRONCFG			0x24
#define ANP82_INTRCFG			0x2a
#define ANP82_INTRSTAT			0x2b
#define ANP82_MISCCTRL			0x2c

// *******************************************************************
// Structures

typedef struct _anp82_t anp82_t;
typedef struct _anp82_ops_t anp82_ops_t;

// generic driver structure
struct _anp82_t{

  anp82_ops_t* ops;
  void* data;
  int chip_id;
};

// lowlevel access operations
struct _anp82_ops_t {

  char name[32];
  int (* get_reg) (anp82_t *anp82, int reg);
  int (* set_reg) (anp82_t *anp82, int reg, int val);
};



// *******************************************************************
// function declarations

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions



/**
 *
 * Create new AnP82 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 */

extern anp82_t* anp82_new(anp82_ops_t* ops, void* data);


/**
 *
 * Destroy an Anp82 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void anp82_free(anp82_t* instance);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions


/**
 *
 * Set the Anp82's overlay colour
 *
 * @param instance AnP82 instance to use
 * @param red_low Lower bound for colour
 * @param red_high Upper bound for colour
 * @param green_low Lower bound for colour
 * @param green_high Upper bound for colour
 * @param blue_low Lower bound for colour
 * @param blue_high Upper bound for colour
 *
 */

extern int anp82_set_overlay_colour(anp82_t* instance, 
				    int red_low, int red_high,
				    int green_low, int green_high,
				    int blue_low, int blue_high);


/**
 *
 * Set the colour gain
 *
 * @param instance AnP82 instance
 * @param common Common gain (0 - 0x3f)
 * @param red red gain (0- 63)
 * @param green green gain (0- 63)
 * @param blue blue gain (0- 63)
 *
 */

extern int anp82_set_gain(anp82_t* instance, 
			  int common,
			  int red,
			  int green,
			  int blue);


/**
 *
 * Sets the "in delay" value for the AnP82
 *
 * @param instance AnP82 instance to use
 * @param value In delay value (0-3)
 *
 */

extern int anp82_set_in_delay(anp82_t* instance, int value);


/**
 *
 * Set overlay mode
 *
 * @param instance AnP82 instance
 * @param value One of ANP82_OVERLAY_*
 *
 */

extern int anp82_set_overlay_mode(anp82_t* instance, int value);


/**
 *
 * Initialises the AnP82
 *
 * @param instance AnP82 instance to use
 *
 */

extern int anp82_init(anp82_t* instance);



/**
 *
 * Measures the video blank start & width
 *
 * @param instance AnP82 instance
 * @param vbStart Where to put the VB start value
 * @param vbWidth Where to put the VB width value
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_video_blank(anp82_t* instance, int* vbStart, int* vbWidth);


/**
 *
 * Set the video blank registers
 *
 * @param instance anp82 instance to use
 * @param vbStart Video blank start
 * @param vbWidth Video blank width
 *
 * @return 0 on success, <0 on failure 
 *
 */

extern int anp82_set_video_blank(anp82_t* instance, int vbStart, int vbWidth);




/**
 *
 * Set sync polarities
 *
 * @param instance AnP82 instance
 * @param hPolarity hsync polarity (one of ANP82_SYNC_ACTIVE_*)
 * @param vPolarity vsync polarity (one of ANP82_SYNC_ACTIVE_*)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_sync_polarities(anp82_t* instance, 
				     int hPolarity, int vPolarity);


/**
 *
 * Measures the vertical picture offset
 *
 * @param instance AnP82 instance
 * @param offset Current V position of overlay window
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_vertical_offset(anp82_t* instance, int offset);


  
/**
 *
 * Measures the horizontal picture offset
 *
 * @param instance AnP82 instance
 * @param offset Current H position of overlay window
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_horizontal_offset(anp82_t* instance, int offset);



/**
 *
 * Work out the vsync polarity
 *
 * @param instance AnP82 instance to use
 *
 * @return One of ANP82_SYNC_ACTIVE_* on success, <0 on error
 *
 */

extern int anp82_measure_vsync_polarity(anp82_t* instance);


/**
 *
 * Work out the hsync polarity
 *
 * @param instance AnP82 instance to use
 *
 * @return One of ANP82_SYNC_ACTIVE_* on success, <0 on error
 *
 */

extern int anp82_measure_hsync_polarity(anp82_t* instance);



/**
 *
 * Measures the horizontal ratio for converting VGA card pixels to VxP pixels
 *
 * @param instance AnP82 instance
 * @param screenWidth Width of VGA card's screen in pixels
 *
 * @return ratio (if >0), or error if <0
 *
 */

extern int anp82_measure_horizontal_ratio(anp82_t* instance, int screenWidth);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Register get/set functions


/**
 *
 * Get register from the AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int anp82_get_reg(anp82_t* instance, int reg);


/**
 *
 * Set register on the AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void anp82_set_reg(anp82_t* instance, int reg, int val);


/**
 *
 * Get specified bitmask of a register from AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int anp82_get_bits(anp82_t* instance, int reg, int bitmask);


/**
 *
 * Set specified bits of a register on AnP82
 *
 * @param instance Instance of the AnP82 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void anp82_set_bits(anp82_t* instance, int reg, int bitmask, int valuemask);

#endif
