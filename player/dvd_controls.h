#ifndef __DVD_CONTROLS_H__
#define __DVD_CONTROLS_H__

/**********************************************************************
  *
  *     Copyright 2002 James Hawtin oolon@ankh.org
  *
  *     Alot of this is based on play_title.c in libdvdread.
  *     Copyright (C) 2001 Billy Biggs <vektor@dumbterm.net>.
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
 * dvd_controls.c and dvd_controls.h, are additions for controling dvd playback
 * using the dvd_play library
 */

#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>
#include "multifile.h"
#include <dvdread/nav_types.h>

typedef struct
{
  int             angle;
  int             title;
  dvd_reader_t   *dvd;
  dvd_file_t     *title_file;
  tt_srpt_t      *tt_srpt;
  ifo_handle_t   *vmg_file;
  ifo_handle_t   *vts_file;

  int             ttn;
  int             pgc_id;
  int             chapid;
  int             pgn;
  pgc_t          *cur_pgc;
  vts_ptt_srpt_t *vts_ptt_srpt;
  int             start_cell;
  unsigned int    cur_pack;
  
  int             data_pos;
  int             data_max;
  int             cur_cell;
  int             next_cell;
  int             reenter;
  unsigned char  *data;
}  dvd_controls;

/* Initialise any dvd resources to zero */
void dvd_initialise(dvd_controls *con);

/* Setup for dvd play */
int dvd_setup(dvd_controls *con, char *file_name);

/* Free any outstanding resourse */ 
void dvd_cleanup(dvd_controls *con);

offset_t dvd_read(dvd_controls *con, char *buffer, offset_t len);
void dvd_set_chapter(dvd_controls *con, int chapter);
int dvd_num_chapters(dvd_controls *con);
int dvd_next_chapter(dvd_controls *con);
void dvd_prev_chapter(dvd_controls *con);

#endif /* __DVD_CONTROLS_H__ */
