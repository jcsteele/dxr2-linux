#ifndef __VOB_TITLE
#define __VOB_TITLE
#include "multifile.h"

typedef struct call_stack_struct{
  u_short vob_id	: 16;	// Video Object Identifier
  u_int	cell_id		: 8;	// Cell Identifier
  u_char      		: 8;	// don't know
  u_int	start		: 32;	// Cell start
  u_int	end		: 32;	// Cell end
  struct call_stack_struct *next;
} call_stack_t;

typedef struct {
  // ifo
  int use_ifo;
  char ifo_file[80];
  u_char *ptr_pc;
  u_int num_cell_addr;
  call_stack_t *call_stack;
  
} vob_info_t;

typedef struct {
        u_short pgc;            // Program Chain (PTT)
        u_short pg;             // Program (PTT)
        u_long  start;          // Start of VOBU (VTS? CADDR)
        u_long  end;            // End of VOBU (VTS? CADDR)
} ifo_ptt_data_t;
 
typedef struct {
        u_int num;              // Number of Chapters
        ifo_ptt_data_t *data;   // Data
} ifo_ptt_sub_t;
 
typedef struct {
        u_int num;              // Number of Titles
        ifo_ptt_sub_t *title;   // Titles
} ifo_ptt_t;
 
typedef struct {
        u_short pgc             : 16;   // Program Chain number
        u_short pg              : 16;   // Program number
} ptt_t;

extern int open_vob(char*);
extern offset_t vob_read(char*, int);
extern offset_t vob_lseek(offset_t, int);
extern void close_vob();
extern offset_t vob_chapter_seek(int vob_id);
extern offset_t vob_next_chapter();
extern offset_t vob_prev_chapter();
extern int vob_scan_forwards();
extern int vob_num_chapters();
extern int vob_choose_angle(int);

#endif
