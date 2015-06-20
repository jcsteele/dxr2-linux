/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, 
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     February 18, 2002    Scott Bucholtz       Added root execution
  *                                               check
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




/**
 *
 * test program for playing VCDs/DVDs etc
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dxr2ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h> 
#include <signal.h>
#include "css.h"
#include "player.h"
#include "config.h"
#include "interface.h"
#include "multifile.h"

void invalidate_win();

static int quit;

void* read_data_proc(void*);
void* write_data_proc(void*);
void* command_proc(void*);
void* interface_proc(void*);

void clear_buffer(player_info_t*);

int bookmark_command(char, player_info_t*);

int main(int argc, char* argv[])
{
  player_info_t player_info;
  pthread_t read_thread, write_thread, command_thread, interface_thread;
  int interactive_mode=1;
  dxr2_status_info_t dxr2_info;
  pthread_attr_t thread_attrib;

  if(geteuid()!=0) fprintf(stderr, "WARNING:  %s must be run setuid root.\n", argv[0]);
  
  init_interface(&player_info, &dxr2_info);
  initialize(argc, argv, &player_info, &dxr2_info);

  // enter play mode!
  dxr2_set_playmode(DXR2_PLAYMODE_PLAY);

  if(player_info.file_name[0] == '-')
    interactive_mode = 0;

  // signal to threads that it's time to die if this is 1;
  quit = 0;

  // Initialize thread stuff.
  pthread_mutex_init(&(player_info.mutex), NULL);
  pthread_cond_init(&(player_info.playing), NULL);
  pthread_cond_init(&(player_info.slots_free), NULL);
  pthread_cond_init(&(player_info.slots_full), NULL);

  // set up some player status stuff.
  player_info.status = DXR2_PLAYMODE_PLAY;
  player_info.frames_in_buffer = 0;
 
  redraw_window();

  pthread_attr_init(&thread_attrib);
  pthread_attr_setschedpolicy(&thread_attrib, SCHED_RR);

  pthread_create(&read_thread, &thread_attrib, read_data_proc, (void*) &player_info);
  pthread_create(&write_thread, &thread_attrib, write_data_proc, (void*) &player_info);
  if(interactive_mode)
    pthread_create( &command_thread, &thread_attrib, command_proc, (void*) &player_info);

  pthread_join(read_thread, NULL);
  pthread_join(write_thread, NULL);

  if(interactive_mode)
    pthread_kill(command_thread, SIGINT);

  cleanup_and_exit(&player_info);
}

void* read_data_proc(void* arg)
{
  player_info_t* player_info = (player_info_t*) arg;
  int size=1;
  int ourLoc = 0;
  frame buffer;
  int scan_counter=0;

  
#ifdef __DVD_DEBUG
      print_info("Staring read data proc\n");
#endif
  while(size != 0 && size != -1 && !quit) {
    pthread_mutex_lock(&(player_info->mutex));

    // If we're not playing, wait until we are.
    // if we're single stepping, just read one frame.
    if(player_info->status == DXR2_PLAYMODE_PAUSED || player_info->status == DXR2_PLAYMODE_SINGLESTEP) {
      pthread_cond_wait(&(player_info->playing), &(player_info->mutex));
    }

    // If the array of buffered frames is full, wait till one is empty.
    if(player_info->frames_in_buffer == NUM_BUFFERED_FRAMES) {
      pthread_cond_wait(&(player_info->slots_free), &(player_info->mutex));
    }
    pthread_mutex_unlock(&(player_info->mutex));
    
    if(player_info->status == DXR2_PLAYMODE_STOPPED) {
#ifdef __DVD_DEBUG
      print_info("quitting read proc...\n");
#endif
      pthread_exit(NULL);
    }

    if(player_info->clear_read_buffer) {
      ourLoc = 0;
      player_info->clear_read_buffer = 0;
    }

    size=-1;

    if(player_info->dvd)
      size = dvd_read(&player_info->dvd_con, buffer, FRAME_SIZE);
    else if(player_info->vcd)
      size = multi_read(buffer, FRAME_SIZE);
    else
      size = multi_read(buffer, FRAME_SIZE);

    if(size != 0 && size != -1) {
      pthread_mutex_lock(&(player_info->mutex));
      memcpy(player_info->frames[ourLoc], buffer, size);
      player_info->sizes[ourLoc] = size;
      //    }

      //    if(size != 0 && size != -1) {
      ++(player_info->frames_in_buffer);
      ourLoc = (ourLoc + 1) % NUM_BUFFERED_FRAMES;
      pthread_cond_signal(&(player_info->slots_full));
      //    }

      pthread_mutex_unlock(&(player_info->mutex));    
    }
  }

  quit = 1;

#ifdef __DVD_DEBUG
  print_info("quitting read proc\n");
#endif
}

void* write_data_proc(void* arg)
{
  player_info_t* player_info = (player_info_t*) arg;
  int size=1;
  int ourLoc = 0;
  int tics,secs,mins,hrs;
  frame buffer;
  
  tics=secs=mins=hrs=0;

#ifdef __DVD_DEBUG
      print_info("Staring write data proc\n");
#endif
  
  while(quit!=1 || (player_info->frames_in_buffer != 0)) {
    pthread_mutex_lock(&(player_info->mutex));

    // If we're not playing, wait until we are.
    if(player_info->status == DXR2_PLAYMODE_PAUSED) 
      pthread_cond_wait(&(player_info->playing), &(player_info->mutex));

    if(player_info->status == DXR2_PLAYMODE_SINGLESTEP) {
      player_info->status = DXR2_PLAYMODE_PAUSED;
      dxr2_set_playmode(player_info->status);
    }

    //    if(player_info->status == DXR2_PLAYMODE_SCAN && skip==0)
    //      skip = 35;
  
    // If there are no frames in the buffer, wait until there are.
    if(player_info->frames_in_buffer == 0)
      pthread_cond_wait(&(player_info->slots_full), &(player_info->mutex));

    // If we've quit while waiting, stop.
    if(player_info->status == DXR2_PLAYMODE_STOPPED) {
      pthread_mutex_unlock(&(player_info->mutex));
      pthread_cond_signal(&(player_info->slots_free));
      pthread_exit(NULL);
    }

    if(player_info->clear_write_buffer) {
      ourLoc = 0;
      player_info->clear_write_buffer = 0;
    }

    size = player_info->sizes[ourLoc];

    // don't write a null frame when switching files.
    if(size != 0 && size != -1) {
      memcpy(buffer, player_info->frames[ourLoc], size);
      ourLoc = (ourLoc+1) % NUM_BUFFERED_FRAMES;
      --(player_info->frames_in_buffer);
      pthread_cond_signal(&(player_info->slots_free));
    }

    pthread_mutex_unlock(&(player_info->mutex));

    if(size != 0 && size != -1) {

      char* bufferPointer = buffer;
      int bufferSize = size;
      int sizeWritten;

      while(1) {
	
	// OK, write the data
	sizeWritten = write(player_info->dxr2FD, bufferPointer, bufferSize);

	// if the write was completely successful, exit loop
	if (sizeWritten == bufferSize) {
	  
	  break;
	}

	// OK, if we're stopped, stop sending data
	if (player_info->status == DXR2_PLAYMODE_STOPPED) {
	  
	  break;
	}
	
	// if we're paused, go to sleep for a bit
	if (player_info->status == DXR2_PLAYMODE_PAUSED) {

	  // adjust buffer appropriately
	  bufferPointer += sizeWritten;
	  bufferSize -= sizeWritten;

	  // go to sleep until unpaused
	  pthread_mutex_lock(&(player_info->mutex));	  
	  pthread_cond_wait(&(player_info->playing), &(player_info->mutex));
	  pthread_mutex_unlock(&(player_info->mutex));
	  continue;
	}

	// if we're in any other mode, just continue sending data
	bufferPointer += sizeWritten;
	bufferSize -= sizeWritten;
      }
    }

    tics = (tics+2)%30;
    if(tics==0) {
      secs = (secs+1) % 60;
      if(secs==0) {
	mins = (mins+1) % 60;
	if(mins==0)
	  ++hrs;
      }
    }
  }
#ifdef __DVD_DEBUG
  print_info("quitting write proc\n");
#endif
}

void* command_proc(void* arg)
{
  player_info_t* player_info = (player_info_t*) arg;
  char c;
  int done=0;
  int newmode;
  int bookmark_commands = 0;
  int chapter;
  char input[80];
  geom_t geom;

  while(!done && !quit) {
    c=get_a_char();

    if(quit)
      c='q';

#ifdef NOTDEF
    if(bookmark_commands)
      bookmark_commands = bookmark_command(c, player_info);
    else
#endif /* NOTDEF */
      switch (c) {
	case 'q':
	case 'Q':
	case 's':
	  //      case 'S':
	  if(player_info->status == DXR2_PLAYMODE_PAUSED)
	    dxr2_set_playmode(DXR2_PLAYMODE_PLAY);

	  player_info->status = DXR2_PLAYMODE_STOPPED;
	  quit = done = 1;
	  // unfreeze processes if we're paused.
	  pthread_cond_broadcast(&(player_info->playing));
	  // free up read if it's waiting on empty slots.
	  pthread_cond_broadcast(&(player_info->slots_free));
	  // free up write if it's waiting on full slots.
	  pthread_cond_broadcast(&(player_info->slots_full));
	  invalidate_win();
	  break;
	case 'p':
	case 'P':
	  // play/pause
	  pthread_mutex_lock(&(player_info->mutex));
	  if(player_info->status == DXR2_PLAYMODE_PLAY) {
	    player_info->status = DXR2_PLAYMODE_PAUSED;
	  } else {
	    player_info->status = DXR2_PLAYMODE_PLAY;
	    pthread_cond_broadcast(&(player_info->playing));
	  }
	  dxr2_set_playmode(player_info->status);
	  pthread_mutex_unlock(&(player_info->mutex));
	  invalidate_win();
	  break;
	case 'a':
	case 'A': // fast forward
	  if(player_info->status==DXR2_PLAYMODE_PLAY) {
	    player_info->status=DXR2_PLAYMODE_FASTFORWARDS;
	    player_info->fast_rate = DXR2_PLAYRATE_2x;
	    pthread_mutex_lock(&(player_info->mutex));
	    if(!dxr2_fast_forwards(player_info->fast_rate))
	      invalidate_win();
	    else
	      player_info->status=DXR2_PLAYMODE_PLAY;
	    pthread_mutex_unlock(&(player_info->mutex));
	  }
	  else
	    print_info("not fastmode\n");
	  break;
	case 'f':
	  if(player_info->status==DXR2_PLAYMODE_PLAY) {
	    player_info->status=DXR2_PLAYMODE_SLOWFORWARDS;
	    player_info->slow_rate = DXR2_PLAYRATE_2x;
	    pthread_mutex_lock(&(player_info->mutex));
	    if(!dxr2_slow_forwards(player_info->slow_rate))
	      invalidate_win();
	    else
	      player_info->status=DXR2_PLAYMODE_PLAY;
	    pthread_mutex_unlock(&(player_info->mutex));
	  }
	  else
	    print_info("not slowmode\n");
	  break;
	case '2':
	  if(player_info->status==DXR2_PLAYMODE_SLOWFORWARDS) {
	    player_info->slow_rate = DXR2_PLAYRATE_2x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_slow_forwards(player_info->slow_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  else if(player_info->status==DXR2_PLAYMODE_FASTFORWARDS) {
	    player_info->fast_rate = DXR2_PLAYRATE_2x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_fast_forwards(player_info->fast_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  break;
	case '3':
	  if(player_info->status==DXR2_PLAYMODE_SLOWFORWARDS) {
	    player_info->slow_rate = DXR2_PLAYRATE_3x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_slow_forwards(player_info->slow_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  else if(player_info->status==DXR2_PLAYMODE_FASTFORWARDS) {
	    player_info->fast_rate = DXR2_PLAYRATE_3x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_fast_forwards(player_info->fast_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  break;
	case '4':
	  if(player_info->status==DXR2_PLAYMODE_SLOWFORWARDS) {
	    player_info->slow_rate = DXR2_PLAYRATE_4x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_slow_forwards(player_info->slow_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  else if(player_info->status==DXR2_PLAYMODE_FASTFORWARDS) {
	    player_info->fast_rate = DXR2_PLAYRATE_4x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_fast_forwards(player_info->fast_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  break;
	case '5':
	  if(player_info->status==DXR2_PLAYMODE_SLOWFORWARDS) {
	    player_info->slow_rate = DXR2_PLAYRATE_5x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_slow_forwards(player_info->slow_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  else if(player_info->status==DXR2_PLAYMODE_FASTFORWARDS) {
	    player_info->fast_rate = DXR2_PLAYRATE_5x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_fast_forwards(player_info->fast_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  break;
	case '6':
	  if(player_info->status==DXR2_PLAYMODE_SLOWFORWARDS) {
	    player_info->slow_rate = DXR2_PLAYRATE_6x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_slow_forwards(player_info->slow_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  else if(player_info->status==DXR2_PLAYMODE_FASTFORWARDS) {
	    player_info->fast_rate = DXR2_PLAYRATE_6x;
	    pthread_mutex_lock(&(player_info->mutex));
	    dxr2_fast_forwards(player_info->fast_rate);
	    pthread_mutex_unlock(&(player_info->mutex));
	    invalidate_win();
	  }
	  break;
	case 'h':
	case 'H':
	  print_info("Commands:\n");
	  print_info("   's' == Stop\n");
	  print_info("   'q' == Quit\n");
	  print_info("   'p' == Pause/Resume\n");
	  print_info("   'a' == fAst forwards\n");
	  print_info("   'f' == slow Forwards\n");
	  print_info("     When in slow Forwards mode:\n");
	  print_info("       '2' == 2x slow \n");
	  print_info("       '3' == 3x slow\n");
	  print_info("       '4' == 4x slow\n");
	  print_info("       '5' == 5x slow\n");
	  print_info("       '6' == 6x slow\n");
	  print_info("   'n' == Next chapter\n");
	  print_info("   'v' == preVious chapter\n");
	  print_info("   'c' == Choose chapter\n");
	  print_info("   'b' == Bookmark mode\n");
	  print_info("   'u' == toggle fullscreen\n");
	  print_info("   'd' == Set in_delay\n");
	  print_info("   'h' == Help ( this listing )\n");
	  break;
	case 'b':
	case 'B':
#ifdef NOTDEF
	  print_info("Bookmark commands.\n");
	  print_info("'h' for help.\n");
	  bookmark_commands = 1;
#endif /* NOTDEF */
	  print_info("Bookmarks have been disabled at present.\n");
	  break;
	case 'n':
	case 'N':
	  if(player_info->dvd) {
	    int ret;
	    pthread_mutex_lock(&(player_info->mutex));
	    ret=dvd_next_chapter(&player_info->dvd_con);
	    pthread_mutex_unlock(&(player_info->mutex));
	    if (ret)
	      clear_buffer(player_info);
	  }
	  else
	    print_info("That feature is unavailable with IFO parsing off.  try -T<title number>\n");
	  break;
	case 'v':
	case 'V':
	  if(player_info->dvd) {
	    pthread_mutex_lock(&(player_info->mutex));
	    dvd_prev_chapter(&player_info->dvd_con);
	    pthread_mutex_unlock(&(player_info->mutex));
	    clear_buffer(player_info);
	  }
	  else
	    print_info("That feature is unavailable with IFO parsing off.  try -T<title number>\n");
	  break;
	case 'c':
	case 'C':
	  if(player_info->dvd)
	  {
	    print_info("Which chapter would you like [1-%d]?  ", dvd_num_chapters(&player_info->dvd_con));
	    get_a_str(input, 80);
	    input[strlen(input)] = '\0';
	    chapter=0;
	    sscanf(input, "%d", &chapter);
	    if (chapter==0)
	      break;
	    if (chapter<1 || chapter>dvd_num_chapters(&player_info->dvd_con))
	    {
	      print_info("Chapter %d not in the range 1-%d\n", chapter, dvd_num_chapters(&player_info->dvd_con));
	      break;
	    }
	    print_info("Seeking chapter: %d\n", chapter);
	    
	    --chapter; // convert from 1-based indexing to 0-based indexing
	    pthread_mutex_lock(&(player_info->mutex));
	    dvd_set_chapter(&player_info->dvd_con, chapter);
	    pthread_mutex_unlock(&(player_info->mutex));
	    clear_buffer(player_info);
	  }
	  else
	    print_info("That feature is unavailable with IFO parsing off.  try -T<title number>\n");
	  break;
	case 'o':
	case 'O':
	  //        toggle_overlay(player_info);
	  break;	
	  /*	case 'f':
		// go forward
		pthread_mutex_lock(&(player_info->mutex));
		if(player_info->status == DXR2_PLAYMODE_PAUSED) {
		player_info->status = DXR2_PLAYMODE_SINGLESTEP;
		dxr2_set_playmode(player_info->status);
		pthread_cond_broadcast(&(player_info->playing));
		}
		pthread_mutex_unlock(&(player_info->mutex));
		invalidate_win();
		break;*/
	case 'u':
	  // fUll Screen
	  if(player_info->full_screen) {
	    geom.x = player_info->dxr2_info->overlay_pos.arg1;
	    geom.y = player_info->dxr2_info->overlay_pos.arg2;
	    geom.width = player_info->dxr2_info->overlay_dim.arg1;
	    geom.height = player_info->dxr2_info->overlay_dim.arg2;
	    dxr2_set_overlay_mode(player_info->dxr2_info->overlay_mode.arg);
	  }
	  else {
	    geom.x = 0;
	    geom.y = 0;
	    geom.width=player_info->dxr2_info->vgaBuf.xScreen+2;
	    geom.height=player_info->dxr2_info->vgaBuf.yScreen;
	    dxr2_set_overlay_mode(DXR2_OVERLAY_WINDOW_KEY);
	  }

	  player_info->full_screen = !player_info->full_screen;
	  dxr2_set_overlay_geom(geom);
	  break;
	case 'd':
	  print_info("What value do you want for in-delay[0-3]: ");
	  do {
	    c = get_a_char();
	  } while (c=='\n');
	  if(c>='0' && c <= '3')
	    dxr2_set_in_delay(c-'0');
	  print_info("\n");
	  break;
      }
  }
#ifdef __DVD_DEBUG
  print_info("quitting command proc\n");
#endif
  pthread_exit(NULL);
}

#ifdef NOTDEF
int bookmark_command(char c, player_info_t* player_info)
{
  int idx;
  offset_t offset;

  switch(c) {
  case 'q':
  case 'Q':
    print_info("Leaving Bookmark Mode.\n");
    return 0;
    break;
  case 'p':
  case 'P':
    print_bookmarks();
    break;
  case 'c':
  case 'C':
    offset = 0;
    print_info("getting offset...");
    pthread_mutex_lock(&(player_info->mutex));
    if(player_info->title)
      offset = vob_lseek(0, SEEK_CUR);
    else
      offset = multi_lseek(0, SEEK_CUR);
    pthread_mutex_unlock(&(player_info->mutex));
    print_info("done.\n");
    create_bookmark_at(offset);
    break;
  case 'g':
  case 'G':
    idx = choose_bookmark();
    if(idx >= 0) {
      print_info("Bookmark %d chosen.\n", idx);
      offset = bookmark_offset(idx);
      pthread_mutex_lock(&(player_info->mutex));
      print_info("Seeking to ofset %d\n", offset);
      if(player_info->title)
	vob_lseek(offset, SEEK_SET);
      else
	multi_lseek(offset, SEEK_SET);
      pthread_mutex_unlock(&(player_info->mutex));
    }
    clear_buffer(player_info);
    break;
  case 'l':
  case 'L':
    print_info("What file do you want to load?\n\t");
    get_a_str(player_info->bookmark_file, 80);
    load_bookmarks(player_info->bookmark_file);
    break;
  case 's':
  case 'S':
    if(player_info->bookmark_file[0] == '\0') {
      print_info("What should I save the file as?\n\t");
      get_a_str(player_info->bookmark_file, 80);
    }
    write_bookmarks(player_info->bookmark_file);
    break;
  case 'h':
  case 'H':
    print_info("Bookmark Help:\n");
    print_info("\n");
    print_info("  'p' == Print Bookmarks\n");
    print_info("  'c' == Create a new bookmark at the present location\n");
    print_info("  'g' == Goto a bookmark\n");
    print_info("  'l' == Load Bookmarks\n");
    print_info("  's' == Save Bookmarks\n");
    break;
  }
  return 1;
}
#endif /* NOTDEF */
  
void cleanup_and_exit(player_info_t* player_info)
{
  destroy_interface();
  // since we've finished, invalidate the AGID!
#ifdef NOTDEF
  if (player_info->driveFD != -1) {
    // invalidate AGID
    css_invalidate_AGID(player_info->driveFD, &player_info->authBuf);
  }
#endif /* NOTDEF */
 
  // close it again
  if(player_info->dxr2FD != -1) {
    dxr2_set_playmode(DXR2_PLAYMODE_STOPPED);
    close(player_info->dxr2FD);
  }

  if ((player_info->mpegFD != -1) && (player_info->mpegFD != STDIN_FILENO))
    close(player_info->mpegFD);

  if (player_info->driveFD != -1)
    close(player_info->driveFD);

  if (player_info->dvd)
  {
    dvd_cleanup(&player_info->dvd_con);
  }
  
  exit(0);
}

void clear_buffer(player_info_t* player_info)
{
  pthread_mutex_lock(&(player_info->mutex));
  player_info->frames_in_buffer = 0;
  player_info->clear_read_buffer = 1;
  player_info->clear_write_buffer = 1;
  pthread_mutex_unlock(&(player_info->mutex));
  pthread_cond_signal(&(player_info->slots_free));
}

void invalidate_win()
{
  
  redraw_window();
}


int toggle_overlay(player_info_t* player_info)
{
  dxr2_threeArg_t buf3;

  player_info->do_overlay = !player_info->do_overlay;
  if (player_info->do_overlay == 1)
    dxr2_set_overlay_mode(DXR2_OVERLAY_WINDOW_KEY);
  else
    dxr2_set_overlay_mode(DXR2_OVERLAY_DISABLED);

}
