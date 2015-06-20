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
 * Driver for the Toshiba TC6807AF CSS decode chip
 *
 */


#ifndef __TC6807AF_H__
#define __TC6807AF_H__



// *******************************************************************
// useful defines

// Full name of the chip
#define TC6807AF_FULLNAME			"Toshiba TC6807AF CSS decoder"

// Log name of the driver
#define TC6807AF_LOGNAME			"TC6807AF"


// *******************************************************************
// defines

// register definitions
#define TC6807AF_REGADDRESS 0
#define TC6807AF_REGDATA 1

// location definitions
#define TC6807AF_LOC0 0x0
#define TC6807AF_LOC1 0x1
#define TC6807AF_LOC2 0x2
#define TC6807AF_LOC4 0x4
#define TC6807AF_LOC5 0x5
#define TC6807AF_LOCTITLEKEY 0x10
#define TC6807AF_LOCCHALLENGEKEY 0x30
#define TC6807AF_LOCRESPONSEKEY 0x40

// CSS modes
#define TC6807AF_CSSDECRMODE_OFF 0
#define TC6807AF_CSSDECRMODE_ON  1


// *******************************************************************
// Structures


typedef struct _tc6807af_t tc6807af_t;
typedef struct _tc6807af_ops_t tc6807af_ops_t;

// generic driver structure
struct _tc6807af_t{

  tc6807af_ops_t* ops;
  void* data;
};

// lowlevel access operations
struct _tc6807af_ops_t {

	char name[32];
	int (*get_reg) (tc6807af_t* instance, int reg);
	int (*set_reg) (tc6807af_t* instance, int reg, int val);
};





// *******************************************************************
// function declarations

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions

/**
 *
 * Create new tc6807af driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for the chip
 *
 */

extern tc6807af_t* tc6807af_new(tc6807af_ops_t* ops, void *data);


/**
 *
 * Destroy a Tc6807af driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void tc6807af_free(tc6807af_t* instance);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions


/**
 *
 * Initialise the TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_init(tc6807af_t* instance);


/**
 *
 * Set tc6807af decryption mode
 *
 * @param instance The instance of TC6807AF to use
 * @param mode CSS mode (one of TC6807AF_CSSDECRMODE_X)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_set_decryption_mode(tc6807af_t* instance, int mode);



/**
 *
 * Send challenge key to TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array containing the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_challenge_key(tc6807af_t* instance, unsigned char* key);



/**
 *
 * Get challenge key from tc6807af
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array to recieve the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_get_challenge_key(tc6807af_t* instance, unsigned char* key);



/**
 *
 * Send the response key to the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_response_key(tc6807af_t* instance, unsigned char* key);



/**
 *
 * Get the response key from the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array to receive the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_get_response_key(tc6807af_t* instance, unsigned char* key);



/**
 *
 * Part1 of The TC6807AF specific part of the send disc key protocol. 
 * Since the disc key os 0x800 bytes, you usually DMA it to the chip somehow...
 *
 * @param instance Instance of the TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_disc_key_part1(tc6807af_t* instance);


/**
 *
 * Part2 of The TC6807AF specific part of the send disc key protocol. 
 * This bit waits until (I assume) the TC6807AF has finished processing the key
 *
 * @param instance Instance of the TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_disc_key_part2(tc6807af_t* instance);


/**
 *
 * Send the disc title key
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_title_key(tc6807af_t* instance, unsigned char* key);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Register get/set functions


/**
 *
 * Get location from the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param reg Location to retrieve
 * @return The location's value (or negative on error)
 *
 */

extern int tc6807af_get_loc(tc6807af_t* instance, int loc);


/**
 *
 * Set location on the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param reg Location to set
 * @param val Value to set
 *
 */

extern int tc6807af_set_loc(tc6807af_t* instance, int loc, int val);



#endif








