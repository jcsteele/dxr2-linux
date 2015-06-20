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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <linux/cdrom.h>
#include <dxr2ioctl.h>
#include "css.h"

#ifndef FIBMAP
#define FIBMAP 1
#endif




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

int css_do_disc_key(int driveFD, int dxr2FD, dvd_authinfo* authBuf)
{
  dvd_struct dvdStructBuf;  
  int tmp = 0;

  // authenticate with the drive.
  if (css_authenticate(driveFD, dxr2FD, authBuf)) {
      
    fprintf(stderr, "cannot authenticate with drive\n");
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // get the disk key from drive
  dvdStructBuf.disckey.agid = authBuf->lsa.agid;
  dvdStructBuf.type = DVD_STRUCT_DISCKEY;
  memset(dvdStructBuf.disckey.value, 0, 2048);
  if (tmp = ioctl(driveFD, DVD_READ_STRUCT, &(dvdStructBuf))) {
    
    fprintf(stderr, "cannot get disc key from drive. error %d\n", tmp);
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }
  
  // send it to the card
  if (ioctl(dxr2FD, DXR2_IOC_SEND_DISC_KEY, dvdStructBuf.disckey.value)) {
    
    fprintf(stderr, "cannot send key to card\n");
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // OK
  return(0);
}



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

int css_do_title_key(int driveFD, int dxr2FD, dvd_authinfo* authBuf, int lba)
{
  dvd_struct dvdStructBuf;  
  dxr2_titleKey_t titleKeyBuf;
  dxr2_oneArg_t argBuf;

  // authenticate with the drive.
  if (css_authenticate(driveFD, dxr2FD, authBuf)) {

    fprintf(stderr, "cannot authenticate with drive\n");      
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // get the title key from drive
  authBuf->lstk.type = DVD_LU_SEND_TITLE_KEY;
  authBuf->lstk.lba = lba;
  if (ioctl(driveFD, DVD_AUTH, authBuf)) {

    fprintf(stderr, "couldn't get title key from drive\n");
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // copy it into the titleKey structure 
  // (since we've got the mysterious extra byte to cope with)
  memcpy(titleKeyBuf.key, authBuf->lstk.title_key, 5);

  // CGMS values
  titleKeyBuf.cgmsFlags = ((authBuf->lstk.cpm << 7) | 
    (authBuf->lstk.cp_sec << 6) | 
    (authBuf->lstk.cgms << 4));

  // send it to the card
  if (ioctl(dxr2FD, DXR2_IOC_SEND_TITLE_KEY, &titleKeyBuf)) {
    
    fprintf(stderr, "couldn't send title key to card\n");
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // turn CSS mode on
  argBuf.arg = DXR2_CSSDECRMODE_ON;
  if (ioctl(dxr2FD, DXR2_IOC_SET_DECRYPTION_MODE, &argBuf)) {
    
    fprintf(stderr, "couldn't turn decryption on\n");
    css_invalidate_AGID(driveFD, authBuf);
    return(1);
  }

  // OK!
  return(0);
}



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

int css_invalidate_AGID(int driveFD, dvd_authinfo* auth) 
{
  auth->type = DVD_INVALIDATE_AGID;
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    return(1);
  }  
}



/**
 *
 * Gets the LBA of the supplied MPEG file descriptor
 *
 */

int css_get_lba(char* fname, int* LBA)
{
  int fd;

  if ((fd = open(fname, O_RDONLY)) < 0) {
    
    return(1);
  }

  // attempt to get LBA
  if (ioctl(fd, FIBMAP, LBA)) {

    close(fd);
    return(1);
  }
  close(fd);

  // return it
  return(0);
}


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

int css_authenticate(int driveFD, int dxr2FD, dvd_authinfo* auth)
{

  int agid;
  int tmp = -1;


  // get the AGID
  memset(auth, 0, sizeof(*auth));
  auth->type = DVD_LU_SEND_AGID;
  auth->lsa.agid = 0;
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    fprintf(stderr, "DVD_AUTH: cannot get AGID\n");
    return(1);
  }

  // get the challenge from the card & send it to the drive
  if (ioctl(dxr2FD, DXR2_IOC_GET_CHALLENGE_KEY, auth->hsc.chal)) {
    
    fprintf(stderr, "DVD_AUTH: failed get challenge from card\n");
    return(1);
  }
  auth->type = DVD_HOST_SEND_CHALLENGE;
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    fprintf(stderr, "DVD_AUTH: failed send challenge to drive\n");
    return(1);
  }


  // get the drive's response & send it to the card
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    fprintf(stderr, "DVD_AUTH: failed get response from drive\n");
    return(1);
  }
  if (ioctl(dxr2FD, DXR2_IOC_SEND_RESPONSE_KEY, auth->lsk.key)) {
    
    fprintf(stderr, "DVD_AUTH: failed send response to card\n");
    return(1);
  }


  // get the drive challenge key & send it to the card
  auth->type = DVD_LU_SEND_CHALLENGE;
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    fprintf(stderr, "DVD_AUTH: failed get challenge from drive\n");
    return(1);
  }
  if (ioctl(dxr2FD, DXR2_IOC_SEND_CHALLENGE_KEY, auth->lsc.chal)) {
    
    fprintf(stderr, "DVD_AUTH: failed send challenge to card\n");
    return(1);
  }


  // get the card's response key & send it to the drive
  if (ioctl(dxr2FD, DXR2_IOC_GET_RESPONSE_KEY, auth->hsk.key)) {
    
    fprintf(stderr, "DVD_AUTH: failed get response from card\n");
    return(1);
  }
  auth->type = DVD_HOST_SEND_KEY2;
  if (ioctl(driveFD, DVD_AUTH, auth)) {
    
    fprintf(stderr, "DVD_AUTH: failed send response to card\n");
    return(1);
  }

  return(0);
}




