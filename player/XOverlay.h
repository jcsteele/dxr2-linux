#ifndef __X_OVERLAY
#define __X_OVERLAY

#include "types.h"

extern void init_win(int*, char**, geom_t);
extern void destwin();
extern int set_geom_fn( int (*fn)(geom_t));
extern int set_switch_fn( int (*fn)());

#endif
