/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     March 1, 2002        Scott Bucholtz       Fixed #define bug
  *
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


#ifndef __PLAYER_H__
#define __PLAYER_H__

/*  The following should take care of the compilation warnings  */
#ifdef __DVD_DEBUG
  #undef __DVD_DEBUG
#endif


//#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
//#include <linux/types.h>
//#include <linux/limits.h>
#include <pthread.h>
#include <dxr2ioctl.h>
#include "dxr2-api.h"
#include "dvd_controls.h"

#define FRAME_SIZE 0x8000

typedef char frame[FRAME_SIZE];
typedef int playStatus;

#define NUM_BUFFERED_FRAMES 15

typedef struct {
  dxr2_status_info_t* dxr2_info;
  dvd_authinfo authBuf;
  frame frames[NUM_BUFFERED_FRAMES];
  int sizes[NUM_BUFFERED_FRAMES];
  int frames_in_buffer;
  int driveFD, mpegFD, dxr2FD;
  int vcd;
  int lba;
  int encrypted;
  int do_overlay;
  int full_screen;
  int do_css;
  int clear_read_buffer, clear_write_buffer;
  char file_name[80];
  char dvd_device[20];
  char dvd_mountpoint[20];
  char dxr2_device[20];
  char uCode_file[80];
  char bookmark_file[80];
  playStatus status;
  int slow_rate;
  int fast_rate;
  int dvd;

  dvd_controls dvd_con;
  
  pthread_mutex_t mutex;
  pthread_cond_t playing, slots_free, slots_full;
} player_info_t;

void cleanup_and_exit(player_info_t*);

#endif
