#ifndef __dxr2_overlay
#define __dxr2_overlay
#include "dxr2-api.h"

extern void init_overlay(int, char**, dxr2_status_info_t*);
extern void dest_overlay();
extern int  resize_overlay(geom_t geom);
extern int  overlay_switch();
extern int setup_overlay_params(int, dxr2_status_info_t*, char*);

#endif
