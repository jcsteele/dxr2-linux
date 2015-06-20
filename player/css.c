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

/*
  This is a bit crap however I am not sure if you can open the dxr2 device
  twice, so allow it to be passed.
*/
static int dxr2fd=-1;

void dxr2_css_set_dxr2_fd(int fd)
{
  dxr2fd=fd;
  setenv("LIBDVDREADCSSLIB","libdxr2css.so.0",1);
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

static int css_invalidate_AGID(dxr2_css_info* info) 
{
  info->auth.type = DVD_INVALIDATE_AGID;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    return(1);
  }  
}

/**
 *
 * Authenticate the DXR2 card and the DVD drive
 *
 * @return 0 on success, nonzero on failure
 *
 */

static int css_authenticate(dxr2_css_info *info)
{
  int agid;
  int tmp = -1;


  // get the AGID
  memset((void *)&info->auth, 0, sizeof(dvd_authinfo));
  info->auth.type = DVD_LU_SEND_AGID;
  info->auth.lsa.agid = 0;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: cannot get AGID\n");
    return(1);
  }

  // get the challenge from the card & send it to the drive
  if (ioctl(info->dxr2FD, DXR2_IOC_GET_CHALLENGE_KEY, info->auth.hsc.chal)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed get challenge from card\n");
    return(1);
  }
  info->auth.type = DVD_HOST_SEND_CHALLENGE;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed send challenge to drive\n");
    return(1);
  }


  // get the drive's response & send it to the card
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed get response from drive\n");
    return(1);
  }
  if (ioctl(info->dxr2FD, DXR2_IOC_SEND_RESPONSE_KEY, info->auth.lsk.key)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed send response to card\n");
    return(1);
  }


  // get the drive challenge key & send it to the card
  info->auth.type = DVD_LU_SEND_CHALLENGE;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed get challenge from drive\n");
    return(1);
  }
  if (ioctl(info->dxr2FD, DXR2_IOC_SEND_CHALLENGE_KEY, info->auth.lsc.chal)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed send challenge to card\n");
    return(1);
  }

  // get the card's response key & send it to the drive
  if (ioctl(info->dxr2FD, DXR2_IOC_GET_RESPONSE_KEY, info->auth.hsk.key)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed get response from card\n");
    return(1);
  }
  info->auth.type = DVD_HOST_SEND_KEY2;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {
    
    fprintf(stderr, "libdxr2css: DVD_AUTH: failed send response to card\n");
    return(1);
  }

  return(0);
}

/**
 *
 * Recieves and transmits the encrypted disk key to the card
 *
 * @return 0 on success, nonzero on failure
 *
 */

static int css_do_disc_key(dxr2_css_info *info)
{
  dvd_struct dvdStructBuf;  
  int tmp = 0;

  fprintf(stderr,"Doing disc auth\n");
  
  // authenticate with the drive.
  if (css_authenticate(info))
  {
    fprintf(stderr, "cannot authenticate with drive\n");
    css_invalidate_AGID(info);
    return(1);
  }

  // get the disk key from drive
  dvdStructBuf.disckey.agid = info->auth.lsa.agid;
  dvdStructBuf.type = DVD_STRUCT_DISCKEY;
  memset(dvdStructBuf.disckey.value, 0, 2048);

  if (tmp = ioctl(info->driveFD, DVD_READ_STRUCT, &(dvdStructBuf))) {
    if(errno == EIO) {
      print_info("libdxr2css: Disc does not appear to be encrypted\n");
      return -1;
    }

    fprintf(stderr, "libdxr2css: cannot get disc key from drive. error %d\n", tmp);
    css_invalidate_AGID(info);
    return(1);
  }
  
  // send it to the card
  if (ioctl(info->dxr2FD, DXR2_IOC_SEND_DISC_KEY, dvdStructBuf.disckey.value)) {
    
    fprintf(stderr, "libdxr2css: cannot send key to card\n");
    css_invalidate_AGID(info);
    return(1);
  }

  // OK
  return(0);
}

/**
 *
 * Recieves and transmits the encrypted title key to the card
 *
 * @param lba LBA of the title to retrive the title key for
 *
 * @return 0 on success, nonzero on failure
 *
 */

int dxr2_css_title(dxr2_css_info *info, int lba)
{
  dvd_struct dvdStructBuf;  
  dxr2_titleKey_t titleKeyBuf;
  dxr2_oneArg_t argBuf;

  fprintf(stderr,"Doing title auth\n");
  
  // authenticate with the drive.
  if (css_authenticate(info)) {

    fprintf(stderr, "libdxr2css: cannot authenticate with drive\n");      
    css_invalidate_AGID(info);
    return(1);
  }

  // get the title key from drive
  info->auth.lstk.type = DVD_LU_SEND_TITLE_KEY;
  info->auth.lstk.lba = lba;
  if (ioctl(info->driveFD, DVD_AUTH, &info->auth)) {

    fprintf(stderr, "libdxr2css: couldn't get title key from drive\n");
    css_invalidate_AGID(info);
    return(1);
  }

  // copy it into the titleKey structure 
  // (since we've got the mysterious extra byte to cope with)
  memcpy(titleKeyBuf.key, info->auth.lstk.title_key, 5);

  // CGMS values
  titleKeyBuf.cgmsFlags = ((info->auth.lstk.cpm << 7) | 
    (info->auth.lstk.cp_sec << 6) | 
    (info->auth.lstk.cgms << 4));

  // send it to the card
  if (ioctl(info->dxr2FD, DXR2_IOC_SEND_TITLE_KEY, &titleKeyBuf)) {
    
    fprintf(stderr, "libdxr2css: couldn't send title key to card\n");
    css_invalidate_AGID(info);
    return(1);
  }

  // turn CSS mode on
  argBuf.arg = DXR2_CSSDECRMODE_ON;
  if (ioctl(info->dxr2FD, DXR2_IOC_SET_DECRYPTION_MODE, &argBuf)) {
    
    fprintf(stderr, "libdxr2css: couldn't turn decryption on\n");
    css_invalidate_AGID(info);
    return(1);
  }

  // OK!
  return(0);
}

dxr2_css_info *dxr2_css_open(int fd)
{
  dxr2_css_info *info;
  
  if (dxr2fd==-1)
  {
    fprintf(stderr, "libdxr2css: DXR2 fd has not been initialised before library start\n");
    return 0;
  }

  if (!(info = malloc(sizeof(dxr2_css_info))))
  {
    fprintf(stderr, "libdxr2css: Cannot allocate space for authorisation structure\n");
    return 0;
  }
  
  info->driveFD=fd;
  info->encrypted=1;
  info->dxr2FD=dxr2fd;

  /* Now we authorise the disc */
  if (css_do_disc_key(info))
  {
    free(info);
    return 0;
  }

  return info;
}

void dxr2_css_close(dxr2_css_info *info)
{
  if (!info)
    return;
  
  css_invalidate_AGID(info);
  close(info->driveFD);
  free(info);
}


