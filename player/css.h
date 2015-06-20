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

typedef struct
{
  int encrypted;
  int driveFD;
  int dxr2FD;
  dvd_authinfo auth;
  /*
   * This is a complete Hack! reauthing the stream while we are writing to it
   * is bad, so basically we don't do it and hope the key is the same for all
   * vobs in any one title
   */
  int lba;
  int reauth_required;
  int auth_done;
} dxr2_css_info;

void           dxr2_css_set_dxr2_fd(int fd);
int            dxr2_css_title(dxr2_css_info *info, int lba);
/* Setup the control structure and authorize the disc */
dxr2_css_info *dxr2_css_open(int fd);
void           dxr2_css_close(dxr2_css_info *info);

#endif


