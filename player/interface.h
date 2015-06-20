#ifndef __DXR2_INTERFACE
#define __DXR2_INTERFACE
#include "player.h"
#include "dxr2-api.h"

int init_interface(player_info_t*, dxr2_status_info_t*);
void redraw_win();
void print_error(char* format, ...);
void print_info(char* format, ...);
int get_a_char();
void wait_if_needed();
void get_a_str(char*, int);

#endif
