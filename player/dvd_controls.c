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

#include "dvd_controls.h"
#include "multifile.h"
#include <stdio.h>

#define min(x,y) (x < y ? x : y)

int is_nav_pack(unsigned char *buffer)
{
    return (buffer[41] == 0xbf && buffer[ 1027 ] == 0xbf );
}


int write_frame(char *buffer, offset_t len, offset_t *frame_pos,
		dvd_controls *con)
{
  if ((con->data_pos<con->data_max) && (*frame_pos<len))
  {
    int tot = min(len - *frame_pos, con->data_max-con->data_pos);

    memcpy(&buffer[*frame_pos], &con->data[con->data_pos], tot);

    *frame_pos+=tot;
    con->data_pos+=tot;
    
    if (len <= *frame_pos)
      return 1;
  }
  return 0;
}

offset_t dvd_read(dvd_controls *con, char *buffer, offset_t buff_len)
{
  offset_t frame_pos=0;
  int len;

  if (write_frame(buffer, buff_len, &frame_pos, con))
    return frame_pos;

  /**
   * Playback by cell in this pgc, starting at the cell for our chapter.
   */
  while(con->next_cell <= con->cur_pgc->nr_of_cells)
  {
    if (!con->reenter)
    {
      con->cur_cell = con->next_cell;
      /* test for increments in chapter number and pgn */
      {
	int chapid;
	int pgn;
	int cell;
	chapid=con->chapid+1;

	pgn = con->vts_ptt_srpt->title[con->ttn-1].ptt[chapid].pgn;
	cell = con->cur_pgc->program_map[pgn - 1]-1;
      
	if (con->cur_cell == cell)
	{
	  con->pgn=pgn;
	  con->chapid=chapid;
	}
      }
      
      
      /* Check if we're entering an angle block. */

      if( con->cur_pgc->cell_playback[con->cur_cell].block_type
	  == BLOCK_TYPE_ANGLE_BLOCK )
      {
	int i;
	
	con->cur_cell += con->angle;
	for( i = 0;; ++i )
	{
	  if( con->cur_pgc->cell_playback[con->cur_cell + i].block_mode
	      == BLOCK_MODE_LAST_CELL )
	  {
	    con->next_cell = con->cur_cell + i + 1;
	    break;
	  }
	}
      }
      else
      {
	con->next_cell = con->cur_cell + 1;
      }
      
      con->cur_pack = con->cur_pgc->cell_playback[con->cur_cell].first_sector;
    }

    while (con->cur_pack < con->cur_pgc->cell_playback[con->cur_cell].last_sector)
    {
      
      dsi_t dsi_pack;
      unsigned int next_vobu, next_ilvu_start, cur_output_size;

      /**
       * Read NAV packet.
       */
      len = DVDReadBlocks(con->title_file, (int) con->cur_pack, 1, con->data);
      if( len != 1 ) {
	fprintf( stderr, "Read failed for block %d\n", con->cur_pack);
	/* Fuck up must close */
	return 0;
      }
//      assert( is_nav_pack(con->data));
      
      /**
       * Parse the contained dsi packet.
     */
      navRead_DSI( &dsi_pack, &(con->data[ DSI_START_BYTE ]) );
//      assert(con->cur_pack == dsi_pack.dsi_gi.nv_pck_lbn );
      
      
      /**
       * Determine where we go next.  These values are the ones we mostly
       * care about.
       */
      next_ilvu_start = con->cur_pack
			+ dsi_pack.sml_agli.data[con->angle].address;
      cur_output_size = dsi_pack.dsi_gi.vobu_ea;
      

      /**
       * If we're not at the end of this cell, we can determine the next
       * VOBU to display using the VOBU_SRI information section of the
       * DSI.  Using this value correctly follows the current angle,
       * avoiding the doubled scenes in The Matrix, and makes our life
       * really happy.
       *
       * Otherwise, we set our next address past the end of this cell to
       * force the code above to go to the next cell in the program.
       */
      if( dsi_pack.vobu_sri.next_vobu != SRI_END_OF_CELL )
      {
	next_vobu = con->cur_pack
		    + (dsi_pack.vobu_sri.next_vobu & 0x7fffffff );
      } else
      {
	next_vobu = con->cur_pack + cur_output_size + 1;
      }
      
//      assert( cur_output_size < 1024 );
      con->cur_pack++;
      
      /**
       * Read in and output cursize packs.
       */
      len = DVDReadBlocks(con->title_file,
			  (int)con->cur_pack, cur_output_size, con->data);
      if( len != (int) cur_output_size ) {
	fprintf( stderr, "Read failed for %d blocks at %d\n",
		 cur_output_size, con->cur_pack);
	/* Fuck up must close */
	return 0;
      }
      
      con->data_pos=0;
      con->data_max = cur_output_size * DVD_VIDEO_LB_LEN;
      
      con->cur_pack = next_vobu;
      con->reenter=1;
      if (write_frame(buffer, buff_len, &frame_pos, con))
	return frame_pos;
    }
    con->reenter=0;
  }
  return frame_pos;
}

void dvd_set_chapter(dvd_controls *con, int chapter)
{
  con->chapid=chapter;

  con->pgc_id = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgcn;
  con->pgn = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgn;
  con->cur_pgc = con->vts_file->vts_pgcit->pgci_srp[con->pgc_id - 1].pgc;
  con->next_cell = con->cur_pgc->program_map[con->pgn - 1] - 1;
  con->data_pos=0;
  con->data_max=0;
  con->reenter=0;
}

int dvd_num_chapters(dvd_controls *con)
{
  return con->tt_srpt->title[con->title].nr_of_ptts;
}

int dvd_next_chapter(dvd_controls *con)
{
  if (con->chapid==con->tt_srpt->title[con->title].nr_of_ptts-1)
    return 0;
  
  con->chapid++;

  con->pgc_id = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgcn;
  con->pgn = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgn;
  con->cur_pgc = con->vts_file->vts_pgcit->pgci_srp[con->pgc_id - 1].pgc;
  con->next_cell = con->cur_pgc->program_map[con->pgn - 1] - 1;
  con->data_pos=0;
  con->data_max=0;
  con->reenter=0;
  return 1;
}

void dvd_prev_chapter(dvd_controls *con)
{
  if (con->chapid>0)
    con->chapid--;

  con->pgc_id = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgcn;
  con->pgn = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgn;
  con->cur_pgc = con->vts_file->vts_pgcit->pgci_srp[con->pgc_id - 1].pgc;
  con->next_cell = con->cur_pgc->program_map[con->pgn - 1] - 1;
  con->data_pos=0;
  con->data_max=0;
  con->reenter=0;
}

void dvd_initialise(dvd_controls *con)
{
  con->data=(unsigned char *) malloc(1024*DVD_VIDEO_LB_LEN);
  con->vts_file=0;
  con->vmg_file=0;
  con->title_file=0;
  con->dvd=0;
  con->angle=0;
  con->title=0;
  con->reenter=0;
  con->chapid=0;
}

int dvd_setup(dvd_controls *con, char *file_name)
{
  if (!con->dvd && !(con->dvd=DVDOpen(file_name)))
    return 0;
  
  if (!con->vmg_file && !(con->vmg_file = ifoOpen(con->dvd, 0)))
    return 0;
  
  con->tt_srpt = con->vmg_file->tt_srpt;
    
  /*
   * Make sure the chapter number is valid for this title.
   */

  if ((con->title<0) || (con->title > con->tt_srpt->nr_of_srpts))
  {
    fprintf(stderr, "There are only %d titles on this DVD.\n",
	    con->tt_srpt->nr_of_srpts);
    return 0;
  }

  /*
   * Make sure the angle number is valid for this title.
   */
  if((con->angle < 0) ||
     (con->angle >= con->tt_srpt->title[con->title].nr_of_angles))
  {
    fprintf( stderr, "Invalid angle %d\n", con->angle + 1 );
    return 0;
  }

  /*
   * Load the VTS information for the title set our title is in.
   */

  if (!(con->vts_file = ifoOpen(con->dvd, con->tt_srpt->title[con->title].title_set_nr)))
  {
    fprintf( stderr, "Can't open the title %d info file.\n",
	     con->tt_srpt->title[con->title].title_set_nr );
    return 0;
  }


  /*
   * Determine which program chain we want to watch.  This is based on the
   * chapter number.
   */
  
  con->ttn = con->tt_srpt->title[con->title].vts_ttn;
  con->vts_ptt_srpt = con->vts_file->vts_ptt_srpt;
  con->pgc_id = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgcn;
  con->pgn = con->vts_ptt_srpt->title[con->ttn - 1].ptt[con->chapid].pgn;
  con->cur_pgc = con->vts_file->vts_pgcit->pgci_srp[con->pgc_id - 1].pgc;
  con->next_cell = con->cur_pgc->program_map[con->pgn - 1] - 1;

  /*  This next line is for the subtitles  */
  dxr2_set_subpicture_palettes(con->vmg_file->first_play_pgc->palette);
  
  /*
   * We've got enough info, time to open the title set data.
   */
  if (!(con->title_file = DVDOpenFile(con->dvd,
				      con->tt_srpt->title[con->title].title_set_nr,
				      DVD_READ_TITLE_VOBS)))
  {
    fprintf(stderr, "Can't open title VOBS (VTS_%02d_1.VOB).\n",
	     con->tt_srpt->title[con->title].title_set_nr );
    return 0;
  }

  con->data_pos=0;
  con->data_max=0;

  return 1;
}

void dvd_cleanup(dvd_controls *con)
{
  if (con->vts_file)
    ifoClose(con->vts_file);
  if (con->vmg_file)
    ifoClose(con->vmg_file);
  if (con->title_file)
    DVDCloseFile(con->title_file);
  if (con->dvd)
    DVDClose(con->dvd);
  if (con->data)
    free(con->data);
}
