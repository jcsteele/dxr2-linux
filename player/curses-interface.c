#include <stdlib.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <dxr2ioctl.h>
#include "player.h"
#include "interface.h"

/* Private types */
typedef struct {
  WINDOW* win;
  int max_x, max_y;
  int orig_x, orig_y;
  player_info_t* player_info;
  dxr2_status_info_t* dxr2_info;
} curses_info_t;

/* local vars */
static curses_info_t curses_info;

/* local functions */
void draw_hline(int, int, int);
void draw_vline(int, int, int);
void draw_sep(int, int);
void redraw_lines();
void redraw_header();
void redraw_file();
void redraw_play_status();
void redraw_tv_format();
void redraw_video_format();
void redraw_video_freq();
void redraw_viewing_mode();
void redraw_audio_type();
void redraw_audio_freq();
void redraw_audio_width();
void redraw_audio_stream();
void redraw_video_stream();


int init_interface(player_info_t* player_info, dxr2_status_info_t* dxr2_info)
{
  curses_info.player_info = player_info;
  curses_info.dxr2_info = dxr2_info;

  curses_info.win = initscr();
  getmaxyx(curses_info.win, curses_info.orig_y, curses_info.orig_x);
  //  resizeterm(25, 80);
  getmaxyx(curses_info.win, curses_info.max_y, curses_info.max_x);
  cbreak();
  noecho();
  idlok(curses_info.win, TRUE);
  scrollok(curses_info.win, TRUE);
  clear();

  wsetscrreg(curses_info.win, 9, curses_info.max_y-2);
  wmove(curses_info.win, 9, 1);

  return 1;
}

void destroy_interface()
{
  resizeterm(1,1);
  wsetscrreg(curses_info.win, 0, curses_info.orig_y-1);
  scrollok(curses_info.win, FALSE);
  endwin();
}

void redraw_lines()
{
  int y, x;

  getyx(curses_info.win, y, x);

  box(curses_info.win, ACS_VLINE, ACS_HLINE);
  wmove(curses_info.win, 2, 10);
  draw_hline(2, 0, curses_info.max_x-1);
  draw_hline(4, 0, curses_info.max_x-1);
  draw_hline(6, 0, curses_info.max_x-1);
  draw_hline(8, 0, curses_info.max_x-1);
  draw_sep(36, 3);
  draw_sep(56, 3);
  draw_sep(24, 5);
  draw_sep(41, 5);
  draw_sep(66, 5);
  draw_sep(24, 7);
  // need a cross here.
  mvwvline(curses_info.win, 6, 24, ACS_PLUS, 1);
  draw_sep(47,7);
  draw_sep(66, 7);
  // need a cross here.
  mvwvline(curses_info.win, 6, 66, ACS_PLUS, 1);

  wmove(curses_info.win, y, x);
}

void draw_hline(int row, int start, int end)
{
  mvwhline(curses_info.win, row, start, ACS_LTEE, 1);
  mvwhline(curses_info.win, row, start+1, ACS_HLINE, end-start-1);
  mvwhline(curses_info.win, row, end, ACS_RTEE, 1);
}

void draw_vline(int col, int start, int end)
{
  mvwvline(curses_info.win, start, col, ACS_TTEE, 1);
  mvwvline(curses_info.win, start+1, col, ACS_VLINE, end-start-1);
  mvwvline(curses_info.win, end, col, ACS_BTEE, 1);
}

void draw_sep(int col, int row)
{
  draw_vline(col, row-1, row+1);
}

void redraw_window()
{
  int y, x;

  getyx(curses_info.win, y, x);

  redraw_header();
  redraw_file();
  redraw_play_status();
  redraw_tv_format();
  redraw_video_format();
  redraw_video_freq();
  redraw_viewing_mode();
  redraw_audio_type();
  redraw_audio_freq();
  redraw_audio_width();
  redraw_audio_stream();
  redraw_video_stream();
  redraw_lines();

  wmove(curses_info.win, y, x);
  wrefresh(curses_info.win);
}

void redraw_header()
{
  mvwprintw(curses_info.win, 1, 2, "Creative Dxr2   DVD Player     Copyright (1999) Creative Labs  ('h' == help)");
}

void redraw_file()
{
  mvwprintw(curses_info.win, 3, 2, "File: %s", curses_info.player_info->file_name);
}

void redraw_play_status()
{
  char* str;
  
  switch(curses_info.player_info->status) {
    case DXR2_PLAYMODE_PLAY:
      str = "Playing       ";
      break;
    case DXR2_PLAYMODE_PAUSED:
      str = "Paused        ";
      break;
    case DXR2_PLAYMODE_STOPPED:
      str = "Stopped       ";
      break;
    case DXR2_PLAYMODE_SINGLESTEP:
      str = "Single Step   ";
      break;
    case DXR2_PLAYMODE_REVERSEPLAY:
      str = "Reverse Play  ";
      break;
  case DXR2_PLAYMODE_SLOWFORWARDS:
    switch(curses_info.player_info->slow_rate) {
    case DXR2_PLAYRATE_2x:
      str = "2x Slow For";
      break;
    case DXR2_PLAYRATE_3x:
      str = "3x Slow For";
      break;
    case DXR2_PLAYRATE_4x:
      str = "4x Slow For";
      break;
    case DXR2_PLAYRATE_5x:
      str = "5x Slow For";
      break;
    case DXR2_PLAYRATE_6x:
      str = "6x Slow For ";
      break;
    }
    break;
  case DXR2_PLAYMODE_SLOWBACKWARDS:
    str = "Slow Backwards";
    break;
  case DXR2_PLAYMODE_FASTFORWARDS:
    switch(curses_info.player_info->fast_rate) {
    case DXR2_PLAYRATE_2x:
      str = "2x Fast For";
      break;
    case DXR2_PLAYRATE_3x:
      str = "3x Fast For";
      break;
    case DXR2_PLAYRATE_4x:
      str = "4x Fast For";
      break;
    case DXR2_PLAYRATE_5x:
      str = "5x Fast For";
      break;
    case DXR2_PLAYRATE_6x:
      str = "6x Fast For ";
      break;
    }
    break;
  case DXR2_PLAYMODE_FASTBACKWARDS:
    str = "Fast Backwards";
    break;
  }

  mvwprintw(curses_info.win, 3, 38, "Status: %s", str);
}

void redraw_tv_format()
{
  char* str;

  switch(curses_info.dxr2_info->tv_format.arg) {
  case DXR2_OUTPUTFORMAT_NTSC:
    str = "NTSC";
    break;
  case DXR2_OUTPUTFORMAT_NTSC_60:
    str = "NTSC 60";
    break;
  case DXR2_OUTPUTFORMAT_PAL_M:
    str = "PAL M";
    break;
  case DXR2_OUTPUTFORMAT_PAL_M_60:
    str = "PAL M 60";
    break;
  case DXR2_OUTPUTFORMAT_PAL_BDGHI:
    str = "PAL BDGHI";
    break;
  case DXR2_OUTPUTFORMAT_PAL_N:
    str = "PAL N";
    break;
  case DXR2_OUTPUTFORMAT_PAL_Nc:
    str = "PAL Nc";
    break;
  case DXR2_OUTPUTFORMAT_PAL_60:
    str = "PAL 60";
    break;
  }

  mvwprintw(curses_info.win, 3, 58, "TV Format: %s", str);
}

void redraw_video_format()
{
  char* str;

  switch(curses_info.dxr2_info->bitstream_type.arg) {
  case DXR2_BITSTREAM_TYPE_MPEG_VOB:
    str = "VOB (DVD)";
    break;
  case DXR2_BITSTREAM_TYPE_CDROM_VCD:
    str = "CDROM VCD";
    break;
  case DXR2_BITSTREAM_TYPE_MPEG_VCD:
    str = "MPEG VCD";
    break;
  case DXR2_BITSTREAM_TYPE_CDDA:
    str = "CDDA(?)";
    break;
  case DXR2_BITSTREAM_TYPE_4:
    str = "Unknown";
    break;
  }

  mvwprintw(curses_info.win, 5, 2, "Video Type: %s", str);
}

void redraw_video_freq()
{
  char* str;

  switch(curses_info.dxr2_info->video_freq.arg1) {
  case DXR2_SRC_VIDEO_FREQ_30:
    str = "30";
    break;
  case DXR2_SRC_VIDEO_FREQ_25:
    str = "25";
    break;
  }
  
  mvwprintw(curses_info.win, 5, 26, "Video Freq: %s", str);
}

void redraw_viewing_mode()
{
  char* str;

  switch(curses_info.dxr2_info->scaling_mode.arg) {
  case DXR2_ASPECTRATIOMODE_NORMAL:
    str = "Normal";
    break;
  case DXR2_ASPECTRATIOMODE_PAN_SCAN:
    str = "Pan & Scan";
    break;
  case DXR2_ASPECTRATIOMODE_LETTERBOX:
    str = "Letterbox";
    break;
  }

  mvwprintw(curses_info.win, 5, 43, "Video Mode: %s", str);
}

void redraw_video_stream()
{
  mvwprintw(curses_info.win, 5, 68, "Stream: %d", curses_info.dxr2_info->video_stream.arg2);
}

void redraw_audio_type()
{
  char* str;

  switch(curses_info.dxr2_info->audio_stream.arg1) {
  case DXR2_STREAM_AUDIO_AC3:
    str = "AC3";
    break;
  case DXR2_STREAM_AUDIO_MPEG:
    str = "MPEG";
    break;
  case DXR2_STREAM_AUDIO_LPCM:
    str = "LPCM";
    break;
  case DXR2_STREAM_AUDIO_5:
    str = "Unknown";
    break;
  }

  mvwprintw(curses_info.win, 7, 2, "Audio Stream: %s", str);
}

void redraw_audio_freq()
{
  char* str;

  switch(curses_info.dxr2_info->audio_freq.arg) {
  case DXR2_AUDIO_FREQ_441:
    str = "44.1 KHz";
    break;
  case DXR2_AUDIO_FREQ_48:
    str = "48 KHz";
    break;
  case DXR2_AUDIO_FREQ_96:
    str = "96 KHz";
    break;
  case DXR2_AUDIO_FREQ_2205:
    str = "2205 Hz";
    break;
  case DXR2_AUDIO_FREQ_32:
    str = "32 KHz";
    break;
  }

  mvwprintw(curses_info.win, 7, 26, "Audio Freq: %s", str);
}

void redraw_audio_width()
{
  char* str;

  switch(curses_info.dxr2_info->audio_width.arg) {
  case DXR2_AUDIO_WIDTH_16:
    str = "16";
    break;
  case DXR2_AUDIO_WIDTH_20:
    str = "20";
    break;
  case DXR2_AUDIO_WIDTH_24:
    str = "40";
    break;
  }

  mvwprintw(curses_info.win, 7, 49, "Audio Width: %s", str);
}

void redraw_audio_stream()
{
  mvwprintw(curses_info.win, 7, 68, "Stream: %d", curses_info.dxr2_info->audio_stream.arg2);
}

void print_error(char* format, ...)
{
  va_list args;
  int y, x;

  getyx(curses_info.win, y, x);

  if(x < 2)
    wmove(curses_info.win, y, 2);

  if(y == curses_info.max_y-1)
    wmove(curses_info.win, y-1, x);

  va_start(args, format);
  vwprintw(curses_info.win, format, args);
  va_end(args);
  
  redraw_lines();

  getyx(curses_info.win, y, x);
  if(x < 2)
    wmove(curses_info.win, y, 2);
  wrefresh(curses_info.win);
}

void print_info(char* format, ...)
{
  va_list args;
  int y, x;

  getyx(curses_info.win, y, x);

  if(x < 2)
    wmove(curses_info.win, y, 2);

  if(y == curses_info.max_y-1)
    wmove(curses_info.win, y-1, x);

  va_start(args, format);
  vwprintw(curses_info.win, format, args);
  va_end(args);
  redraw_lines();

  getyx(curses_info.win, y, x);
  if(x < 2)
    wmove(curses_info.win, y, 2);

  wrefresh(curses_info.win);
}

int get_a_char()
{
  return getch();
}

void wait_if_needed()
{
  print_info("Press a key...\n");
  getch();
}

void get_a_str(char* buf, int len)
{
  int sLen;
  int x, y;

  echo();
  wgetnstr(curses_info.win, buf, len);
  sLen = strlen(buf);
  if(buf[sLen-1]=='\n')
    buf[sLen-1] = '\0';

  getyx(curses_info.win, y, x);

  if(y == curses_info.max_y-1)
    wmove(curses_info.win, y-1, 2);  


  noecho();
}
