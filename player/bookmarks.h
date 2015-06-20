#ifndef __DXR2_BOOKMARKS
#define __DXR2_BOOKMARKS
#include <fcntl.h>
#include "multifile.h"

extern int load_bookmarks(char*);
extern int write_bookmarks(char*);
extern void print_bookmarks();
extern int create_bookmark_at(offset_t);
extern int choose_bookmark();
extern offset_t bookmark_offset(int idx);

#endif
