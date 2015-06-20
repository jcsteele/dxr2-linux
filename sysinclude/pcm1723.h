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
 * Driver for the Burr-Brown PCM1723 DAC
 * Include file
 *
 */


#ifndef __PCM1723_H__
#define __PCM1723_H__



// *******************************************************************
// useful defines

// Full name of the chip
#define PCM1723_FULLNAME		"Burr-Brown PCM1723"

// Log name of the driver
#define PCM1723_LOGNAME			"PCM1723"

// maximum register on chip
#define PCM1723_REGISTERCOUNT 4


// *******************************************************************
// defines

// registers on the pcm1723
#define PCM1723_REG0       0
#define PCM1723_REG1       1
#define PCM1723_REG2       2
#define PCM1723_REG3       3

// possible frequency values
#define PCM1723_FREQ441    0
#define PCM1723_FREQ48     1
#define PCM1723_FREQ96     2
#define PCM1723_FREQ2205   3
#define PCM1723_FREQ32     4

// possible input data width values
#define PCM1723_WIDTH16    0
#define PCM1723_WIDTH20    1
#define PCM1723_WIDTH24    2

// muting flags
#define PCM1723_MUTEON     0
#define PCM1723_MUTEOFF    1

// clock frequency
#define PCM1723_CLKFREQ384 0
#define PCM1723_CLKFREQ256 1

// stereo mode
#define PCM1723_STEREONORMAL  0
#define PCM1723_STEREOMONOL   1
#define PCM1723_STEREOMONOR   2
#define PCM1723_STEREOREVERSE 3




// *******************************************************************
// Structures

typedef struct _pcm1723_ops_t pcm1723_ops_t;
typedef struct _pcm1723_t pcm1723_t;


// generic driver structure
struct _pcm1723_t {

  pcm1723_ops_t* ops;
  int registerValues[PCM1723_REGISTERCOUNT];
  void *data;
};

// lowlevel access operations
struct _pcm1723_ops_t {

  char name[32];
  int (*set_reg) (pcm1723_t* instance, int val);
};





// *******************************************************************
// function declarations

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions

/**
 *
 * Create new pcm1723 driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for the chip
 *
 * @return new instance (or NULL on error)
 *
 */

extern pcm1723_t* pcm1723_new(pcm1723_ops_t* ops, void *data);


/**
 *
 * Destroy a Pcm1723 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void pcm1723_free(pcm1723_t* instance);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions


/**
 *
 * Initialises the DAC. 
 *
 * @param instance Instance of the PCM1723 to use
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_init(pcm1723_t* instance);



/**
 *
 * Sets the DAC Sample frequency.
 *
 * @param instance Instance of the PCM1723 to use
 * @param freqMode. One of the PCM1723_FREQXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_sample_frequency(pcm1723_t* instance, int freqMode);


/**
 *
 * Sets the DAC input width
 *
 * @param instance Instance of the PCM1723 to use
 * @param freqMode. One of the PCM1723_WIDTHXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_input_width(pcm1723_t* instance, int width);



/**
 *
 * Sets/unsets the mute on the DAC
 *
 * @param instance Instance of the PCM1723 to use
 * @param muteMode. One of the PCM1723_MUTEON or PCM1723_MUTEOFF defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_mute_mode(pcm1723_t* instance, int muteMode);


/**
 *
 * Sets stereo mode
 *
 * @param instance Instance of the PCM1723 to use
 * @param muteMode. One of the PCM1723_STEREOXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_stereo_mode(pcm1723_t* instance, int stereoMode);



/**
 *
 * Sets clock frequency
 *
 * @param instance Instance of the PCM1723 to use
 * @param clockFrequency. One of the PCM1723_CLKFREQXX defines
 *
 * @return nonzero on failure
 *
 */

extern int pcm1723_set_clock_frequency(pcm1723_t* instance, int clockFrequency);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Register get/set functions


/**
 *
 * Get register from the PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 *
 * @return the value, or <0 on error
 *
 */

extern int pcm1723_get_reg(pcm1723_t* instance, int reg);


/**
 *
 * Set register on the PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int pcm1723_set_reg(pcm1723_t* instance, int reg, int val);


/**
 *
 * Get specified bitmask of a register from PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues, or <0 on error
 *
 */

extern int pcm1723_get_bits(pcm1723_t* instance, int reg, int bitmask);


/**
 *
 * Set specified bits of a register on PCM1723
 *
 * @param instance Instance of the PCM1723 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 *
 * @return 0 on success, or <0 on error
 *
 */

extern int pcm1723_set_bits(pcm1723_t* instance, int reg, int bitmask, int valuemask);


#endif
