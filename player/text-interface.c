#include <stdlib.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <dxr2ioctl.h>
#include "player.h"
#include "interface.h"

int init_interface(player_info_t* p, dxr2_status_info_t* d)
{
}

void destroy_interface()
{
}

void redraw_window()
{
}

void print_error(char* format, ...)
{
  va_list args;

  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void print_info(char* format, ...)
{
  va_list args;
  int y, x;

  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

int get_a_char()
{
  return getchar();
}

void wait_if_needed()
{
}

void get_a_str(char* buf, int len)
{
  int sLen;

  while ((buf[0] = getchar()) == '\n') ;
  fgets(buf + 1, len - 1, stdin);
  sLen = strlen(buf);
  if(buf[sLen-1]=='\n')
    buf[sLen-1] = '\0';
}
