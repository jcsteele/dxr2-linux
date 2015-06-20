#ifndef __IFO_H__
#define __IFO_H__
#include "vobtitle.h"

#ifndef DVD_VIDEO_LB_LEN
#define DVD_VIDEO_LB_LEN 2048
#endif

#define ID_MAT                  0
#define ID_PTT                  1
#define ID_TITLE_PGCI           2
#define ID_MENU_PGCI            3
#define ID_TMT                  4
#define ID_MENU_CELL_ADDR       5
#define ID_MENU_VOBU_ADDR_MAP   6
#define ID_TITLE_CELL_ADDR      7
#define ID_TITLE_VOBU_ADDR_MAP  8

#define __USE_FILE_OFFSET64
 
#define OFFSET_IFO              0x0000
#define OFFSET_VTS              0x0000
#define OFFSET_LEN              0x00C0
#define IFO_OFFSET_TAT          0x00C0
#define OFFSET_VTSI_MAT         0x0080
#define IFO_OFFSET_VIDEO        0x0100
#define IFO_OFFSET_AUDIO        0x0200
#define IFO_OFFSET_SUBPIC       0x0250

#define OFF_PTT get4bytes (ifo->data[ID_MAT] + 0xC8)
#define OFF_TITLE_PGCI get4bytes (ifo->data[ID_MAT] + 0xCC)
#define OFF_MENU_PGCI get4bytes (ifo->data[ID_MAT] + 0xD0)
#define OFF_TMT get4bytes (ifo->data[ID_MAT] + 0xD4)
#define OFF_MENU_CELL_ADDR get4bytes (ifo->data[ID_MAT] + 0xD8)
#define OFF_MENU_VOBU_ADDR_MAP get4bytes (ifo->data[ID_MAT] + 0xDC)
#define OFF_TITLE_CELL_ADDR get4bytes (ifo->data[ID_MAT] + 0xE0)
#define OFF_TITLE_VOBU_ADDR_MAP get4bytes (ifo->data[ID_MAT] + 0xE4)
 
#define OFF_VMG_PTT get4bytes (ifo->data[ID_MAT] + 0xC4)
#define OFF_VMG_MENU_PGCI get4bytes (ifo->data[ID_MAT] + 0xC8)
#define OFF_VMG_TMT get4bytes (ifo->data[ID_MAT] + 0xD0)

#define DVD_BLOCKSIZE   2048
#define IFO_HDR_LEN 8
#define OFFSET_START_TBL_CMD (12+8*2+32*4+8+16*PGCI_COLOR_LEN)
#define PGCI_COLOR_LEN 4
#define PGCI_SUB_LEN 8
#define CADDR_HDR_LEN 8
#define CADDR_LEN 12
#define OFFSET_VTS_LEN          0x000C

typedef struct {
        u_short vob_id          : 16;   // Video Object Identifier
        u_int   cell_id         : 8;    // Cell Identifier
        u_char                  : 8;    // don't know
        u_int   start           : 32;   // Cell start
        u_int   end             : 32;   // Cell end
} cell_addr_t;
 
typedef struct ifo_struct {
        char *ifo;
        char *start_ptt;
        char *start_title_pgci;
        char *start_menu_pgci;
        char *start_tmt;
        char *start_menu_cell_addr;
        char *start_menu_vobu_addr_map;
        char *start_title_cell_addr;
        char *start_title_vobu_addr_map;
} ifo_t;
 
typedef struct {
        u_short num     : 16;   // number of entries
        u_short         : 16;   // don't known (reserved?)
        u_int   len     : 32;   // length of table
} ifo_hdr_t;
 
typedef struct {
        u_short id              : 16;   // Language
        u_short                 : 16;   // don't know
        u_int   start           : 32;   // Start of unit
} pgci_sub_t;
 
typedef struct {
        u_short num             : 16;   // Number of Video Objects
        u_short unknown         : 16;   // don't know
        u_int   len             : 32;   // length of table
} cell_addr_hdr_t;
 
typedef struct {
        u_int num_menu_vobs;
        u_int vob_start;
 
        u_char *data[10];
 
        int fd;         // file descriptor
        __off64_t pos;  // offset of ifo file on device
} ifolib_t;


u_int get4bytes (u_char *buf);
u_int get2bytes (u_char *buf);

int ptt_num_chapters(ifo_ptt_t *ptt, int titlenum);
ifo_ptt_t *ifo_get_ptt(ifolib_t *ifo);

ifolib_t *ifoOpen(int fd, __off64_t pos);
ifolib_t *ifo_pars(vob_info_t*);
#endif
