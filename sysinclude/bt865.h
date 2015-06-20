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
 * Driver for the Brooktree/Rockwell/Conexant BT865 TV encoder chip
 *
 */

#ifndef __BT865_H__
#define __BT865_H__


// *******************************************************************
// useful defines

// Full name of the chip
#define BT865_FULLNAME	    	        "Brooktree BT865 TV encoder"

// Log name of the driver
#define BT865_LOGNAME			"BT865"

// Chip ID (for i2c interface)
#define BT865_CHIPID			0x8a

// macrovision modes
#define BT865_MACROVISION_OFF                0
#define BT865_MACROVISION_AGC                1
#define BT865_MACROVISION_AGC_2COLOURSTRIPE  2
#define BT865_MACROVISION_AGC_4COLOURSTRIPE  3

// TV output modes
#define BT865_OUTPUT_NTSC      0
#define BT865_OUTPUT_NTSC_60   1
#define BT865_OUTPUT_PAL_M     2
#define BT865_OUTPUT_PAL_M_60  3
#define BT865_OUTPUT_PAL_BDGHI 4
#define BT865_OUTPUT_PAL_N     5
#define BT865_OUTPUT_PAL_Nc    6
#define BT865_OUTPUT_PAL_60    7

// black/white mode
#define BT865_BLACKWHITE_OFF   0
#define BT865_BLACKWHITE_ON    1

// interlacing
#define BT865_INTERLACED_OFF   0
#define BT865_INTERLACED_ON    1

// 7.5 IRE
#define BT865_75IRE_OFF        0
#define BT865_75IRE_ON         1

// pixel modes
#define BT865_PIXEL_CCIR601    0
#define BT865_PIXEL_SQUARE     1


// *******************************************************************
// register definitions

#define BT865_READBACK -1


// *******************************************************************
// Structures

typedef struct _bt865_ops_t bt865_ops_t;
typedef struct _bt865_t bt865_t;


// generic driver structure
struct _bt865_t {

  bt865_ops_t* ops;
  void* data;
  
  char regValues[256];

  int palMode; // flag indicating if we're in PAL mode or not
  
  int chipType; // 4 = bt864, 5 = bt865
};

// lowlevel access operations
struct _bt865_ops_t {

  char name[32];
  int (*get_reg) (bt865_t* instance, int reg);
  int (*set_reg) (bt865_t* instance, int reg, int val);
};



// *******************************************************************
// function declarations

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions


/**
 *
 * Create new BT865 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for the chip
 *
 */

extern bt865_t* bt865_new(bt865_ops_t* ops, void *data);


/**
 *
 * Destroy a BT865 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void bt865_free(bt865_t* instance);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions


/**
 *
 * Initialises the bt865 chip
 *
 * @param instance bt865 instance to use
 *
 */

extern int bt865_init(bt865_t* instance);



/**
 *
 * Sets the bt865 output mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_OUTPUT_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_output_mode(bt865_t* instance, int mode);



/**
 *
 * Set/unset the black/white mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_BLACKWHITE_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_blackwhite_mode(bt865_t* instance, int mode);


/**
 *
 * Set/unset the black/white mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_BLACKWHITE_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_blackwhite_mode(bt865_t* instance, int mode);


/**
 *
 * Set/unset pixel mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_PIXEL_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_pixel_mode(bt865_t* instance, int mode);


/**
 *
 * Set/unset interlaced mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_INTERLACED_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_interlaced_mode(bt865_t* instance, int mode);


/**
 *
 * Set/unset 7.5IRE mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_75IRE_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_75IRE_mode(bt865_t* instance, int mode);



/**
 *
 * Set the macrovision mode.
 *
 * @param instance bt865 instance to use
 * @param macrovisionMode (one of BT865_MACROVISION_*)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_macrovision_mode(bt865_t* instance, int macrovisionMode);

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Register get/set functions


/**
 *
 * Get register from the BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int bt865_get_reg(bt865_t* instance, int reg);


/**
 *
 * Set register on the BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void bt865_set_reg(bt865_t* instance, int reg, int val);


/**
 *
 * Get specified bitmask of a register from BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int bt865_get_bits(bt865_t* instance, int reg, int bitmask);



/**
 *
 * Set specified bits of a register on BT865
 *
 * @param instance Instance of the BT865 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void bt865_set_bits(bt865_t* instance, int reg, int bitmask, int valuemask);


#endif
