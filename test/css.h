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



/*
 **************************************************************************
 **************************************************************************
 * And now, a quick note about CSS and this software:                     *
 * 									  *
 **************************************************************************
 * Although this program permits playing of encrypted DVDs, 		  *
 * there are no "CSS secrets" contained in it, and none were used in 	  *
 * it's construction. No decrypted keys/data are handled by this program  *
 * AT ANY TIME. The DXR2 card does all the CSS decryption on-board, 	  *
 * in hardware. All this program does is permit the exchange of 	  *
 * *encrypted* keys and *encrypted* data between the drive and 		  *
 * DXR2 card's hardware.						  *
 **************************************************************************
 *									  *
 * The authors are NOT connected, in any way, with any of the 		  *
 * "software CSS decryption" programs out there, and *DO NOT* endorse 	  *
 * their use in any way. Piracy is a crime!				  *
 *									  *
 * Just wanted to make that ABSOLUTELY CLEAR. 				  *
 * 									  *
 **************************************************************************
 **************************************************************************
 */

#ifndef CSS_H
#define CSS_H

#include <linux/cdrom.h>



/**
 *
 * Recieves and transmits the encrypted disk key to the card
 *
 * @param driveFD File descriptor of DVD device (e.g. /dev/hdc)
 * @param dxr2FD File descriptor of DXR2 device
 * @param authBuf dvd_authinfo buffer to use 
 *
 * @return 0 on success, nonzero on failure
 *
 */

int css_do_disc_key(int driveFD, int dxr2FD, dvd_authinfo* authBuf);



/**
 *
 * Recieves and transmits the encrypted title key to the card
 *
 * @param driveFD File descriptor of DVD device (e.g. /dev/hdc)
 * @param dxr2FD File descriptor of DXR2 device
 * @param authBuf dvd_authinfo buffer to use 
 * @param lba LBA of the title to retrive the title key for
 *
 * @return 0 on success, nonzero on failure
 *
 */

int css_do_title_key(int driveFD, int dxr2FD, dvd_authinfo* authBuf, int lba);




/**
 *
 * Invalidate the AGID supplied in auth
 *
 * @param driveFD File descriptor of DVD device (e.g. /dev/hdc)
 * @param dxr2FD File descriptor of DXR2 device
 * @param authBuf dvd_authinfo buffer to use 
 *
 * @return 0 on success, nonzero on failure
 *
 */

int css_invalidate_AGID(int driveFD, dvd_authinfo* auth);



/**
 *
 * Gets the LBA of the supplied MPEG file descriptor
 *
 */

int css_get_lba(char* fname, int* LBA);


/**
 *
 * Authenticate the DXR2 card and the DVD drive
 *
 * @param driveFD Drive's device file descriptor
 * @param dxr2FD  DXR2 device's file descriptor
 * @param auth    dvd_authinfo structure to use
 *
 * @return 0 on success, nonzero on failure
 *
 */

int css_authenticate(int driveFD, int dxr2FD, dvd_authinfo* auth);


#endif


