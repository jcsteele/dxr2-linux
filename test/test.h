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


#ifndef __PLAYER_H__
#define __PLAYER_H__

typedef char frame[0x20000];
typedef int playStatus;

#define NUM_BUFFERED_FRAMES 10

typedef struct {
  dxr2_oneArg_t tv_format;
  dxr2_threeArg_t video_freq;
  dxr2_oneArg_t output_aspect_ratio;
  dxr2_oneArg_t source_aspect_ratio;
  dxr2_oneArg_t scaling_mode;
  dxr2_oneArg_t bitstream_type;
  dxr2_oneArg_t macro_vision;
  dxr2_oneArg_t volume;
  dxr2_oneArg_t mute;
  dxr2_oneArg_t audio_width;
  dxr2_oneArg_t audio_freq;
  dxr2_oneArg_t iec_output_mode;  // not sure what this is
  dxr2_oneArg_t subpicture;
  dxr2_threeArg_t audio_stream;
  dxr2_threeArg_t video_stream;
} dxr2_status_info_t;


typedef struct {

  dvd_authinfo authBuf;
  frame frames[NUM_BUFFERED_FRAMES];
  int sizes[NUM_BUFFERED_FRAMES];
  int frames_in_buffer;
  int driveFD, mpegFD, dxr2FD;
  int lba;
  int encrypted;
  char file_name[80];
  char dvd_device[20];
  char dxr2_device[20];
  playStatus status;
  playStatus old_status;
  pthread_mutex_t mutex;
  pthread_cond_t playing, slots_free, slots_full;

} player_info_t;

#endif




