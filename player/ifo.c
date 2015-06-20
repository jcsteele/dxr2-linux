#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "multifile.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "ifo.h"
#include "vobtitle.h"

int ifoReadLB (int fd, __off64_t pos, u_int count, u_char *data)
{
        if ((pos = lseek64 (fd, pos, SEEK_SET)) == -1) {
                fprintf (stderr, "%s/%d: error in lseek\n",
                        __FILE__, __LINE__);
                return -1;
        }
 
        return read (fd, data, count);
}  

int ifoReadTBL (ifolib_t *ifo, u_int offset, u_int tbl_id)
{
        u_char *data;
        u_int len = 0;
 
        if (!offset)
                return -1;
 
        if (!(data =  (u_char *) malloc (DVD_VIDEO_LB_LEN))) {
                perror ("malloc");
                return -1;
        }
 
        if (ifoReadLB (ifo->fd, ifo->pos + offset * DVD_VIDEO_LB_LEN, DVD_VIDEO_LB_LEN, data) <= 0) {
                perror ("ifoReadLB");
                return -1;
        }
 
        switch (tbl_id) {
                case ID_TITLE_VOBU_ADDR_MAP:
                case ID_MENU_VOBU_ADDR_MAP:
                        len = get4bytes (data);
                        break;
 
                default: {
                        ifo_hdr_t *hdr = (ifo_hdr_t *) data;
                        len = ntohl (hdr->len);
                }
        }
 
        if (len > DVD_VIDEO_LB_LEN) {
                //free (data);
 
                if (!(data =  (u_char *) realloc ((void *) data, len))) {
                //if (!(data =  (char *) malloc (len))) {
                        perror ("realloc");
                        return -1;
                }
 
                ifoReadLB (ifo->fd, ifo->pos + offset * DVD_VIDEO_LB_LEN, len, data);
        }
 
        ifo->data [tbl_id] = data;
 
        return 0;
}  

int ifoIsVTS (ifolib_t *ifo)
{
        if (!strncmp (ifo->data[ID_MAT], "DVDVIDEO-VTS", 12))
                return 0;
 
        return -1;
}
 
 
/**
 *
 */
 
int ifoIsVMG (ifolib_t *ifo)
{
        if (!strncmp (ifo->data[ID_MAT], "DVDVIDEO-VMG", 12))
                return 0;
 
        return -1;
}
 

ifolib_t *ifoOpen (int fd, __off64_t pos)
{
        ifolib_t *ifo;
 
        if (!(ifo = (ifolib_t *) calloc (sizeof (ifolib_t), 1))) {
                fprintf (stderr, "%s/%d: memory squeeze (ifolib)\n", __FILE__, __LINE__);
                return NULL;
        }
 
        if (!(ifo->data[ID_MAT] = (char *) calloc (DVD_VIDEO_LB_LEN, 1))) {
                fprintf (stderr, "%s/%d: memory squeeze (data)\n", __FILE__, __LINE__);
                return NULL;
        }
 
        ifo->pos = pos;
        ifo->fd = fd;
 
        if (ifoReadLB (ifo->fd, ifo->pos, DVD_VIDEO_LB_LEN, ifo->data[ID_MAT]) < 0) {
                fprintf (stderr, "%s/%d: something went wrong when reading file.\n", __FILE__, __LINE__);
                return NULL;
        }
 
        ifo->num_menu_vobs      = get4bytes (ifo->data[ID_MAT] + 0xC0);
        ifo->vob_start          = get4bytes (ifo->data[ID_MAT] + 0xC4);
 
        if (!ifoIsVTS (ifo)) {
                ifoReadTBL (ifo, OFF_PTT, ID_PTT);
                ifoReadTBL (ifo, OFF_TITLE_PGCI, ID_TITLE_PGCI);
                ifoReadTBL (ifo, OFF_MENU_PGCI, ID_MENU_PGCI);
                ifoReadTBL (ifo, OFF_TMT, ID_TMT);
                ifoReadTBL (ifo, OFF_MENU_CELL_ADDR, ID_MENU_CELL_ADDR);
                ifoReadTBL (ifo, OFF_MENU_VOBU_ADDR_MAP, ID_MENU_VOBU_ADDR_MAP);
                ifoReadTBL (ifo, OFF_TITLE_CELL_ADDR, ID_TITLE_CELL_ADDR);
                ifoReadTBL (ifo, OFF_TITLE_VOBU_ADDR_MAP, ID_TITLE_VOBU_ADDR_MAP);
        } else if (!ifoIsVMG (ifo)) {
                ifoReadTBL (ifo, OFF_VMG_PTT, ID_PTT);
//              ifoReadTBL (ifo, OFF_TITLE_PGCI, ID_TITLE_PGCI);
                ifoReadTBL (ifo, OFF_VMG_MENU_PGCI, ID_MENU_PGCI);
                ifoReadTBL (ifo, OFF_VMG_TMT, ID_TMT);
//              ifoReadTBL (ifo, OFF_MENU_CELL_ADDR, ID_MENU_CELL_ADDR);
//              ifoReadTBL (ifo, OFF_MENU_VOBU_ADDR_MAP, ID_MENU_VOBU_ADDR_MAP);
                ifoReadTBL (ifo, OFF_TITLE_CELL_ADDR, ID_TITLE_CELL_ADDR);
                ifoReadTBL (ifo, OFF_TITLE_VOBU_ADDR_MAP, ID_TITLE_VOBU_ADDR_MAP);
//              ifo->title_pgci         = get4bytes (ifo->data[ID_MAT] + 0xCC);
//              ifo->menu_cell_addr     = get4bytes (ifo->data[ID_MAT] + 0xD8);
//              ifo->menu_vobu_addr_map = get4bytes (ifo->data[ID_MAT] + 0xDC);
        }
 
        return ifo;
}

ifo_ptt_t *ifo_get_ptt (ifolib_t *ifo)
{
        u_char *ptr;
        ifo_hdr_t *hdr;
        ifo_ptt_t *ifo_ptt;
        ifo_ptt_sub_t *ifo_ptt_sub;
        ptt_t *ptt = (ptt_t *) ptr;
        int i, s;
        u_short prev_start = 0;
        u_int num;
 
        if (!ifo) {
                fprintf (stderr, "%s/%d:no ifo structure present\n", __FILE__, __LINE__);
                return NULL;
        }
 
        ptr = (u_char *) ifo->data[ID_PTT];
        hdr = (ifo_hdr_t *) ifo->data[ID_PTT];
 
        if (!ptr)
                return NULL;
 
        if (!(ifo_ptt = (ifo_ptt_t *) malloc (sizeof (ifo_ptt_t))))
                return NULL;
 
        ifo_ptt->num = ntohs (hdr->num);
 
        if (!(ifo_ptt_sub = (ifo_ptt_sub_t *) calloc (ifo_ptt->num, sizeof (ifo_ptt_sub_t))))
                return NULL;
 
        ifo_ptt->title = ifo_ptt_sub;
 
        ptr += IFO_HDR_LEN;
 
        prev_start = get4bytes (ptr);
 
        for (i=0; i<ntohs(hdr->num); i++) {
                if (i>0) {
                        ifo_ptt_sub = ifo_ptt->title+i-1;
 
                        num = (get4bytes (ptr) - prev_start)/4;
 
                        ptt = (ptt_t *) ((u_char *) ifo->data[ID_PTT] + prev_start);
 
                        if (!(ifo_ptt_sub->data = (ifo_ptt_data_t *) calloc (num, sizeof (ifo_ptt_data_t))))
                                return NULL;
  
                        ifo_ptt_sub->num = num;
 
                        for (s=0; s<num; s++) {
                                ifo_ptt_sub->data[s].pg = ntohs (ptt->pg);
                                ifo_ptt_sub->data[s].pgc = ntohs (ptt->pgc);
 
                                ptt++;
                        }
 
                        prev_start = get4bytes (ptr);
                }
 
                ptr+=4;
        }
 
        ifo_ptt_sub = ifo_ptt->title+i-1;
        num = (ntohl (hdr->len) - prev_start + 1)/4;
 
        ptt = (ptt_t *) ((u_char *) ifo->data[ID_PTT] + prev_start);
 
// TODO: delete allocated elements when failing here
        if (!(ifo_ptt_sub->data = (ifo_ptt_data_t *) calloc (num, sizeof (ifo_ptt_data_t))))
                return NULL;
 
        ifo_ptt_sub->num = num;
 
        for (s=0; s<num; s++) {
                ifo_ptt_sub->data[s].pg = ntohs (ptt->pg);
                ifo_ptt_sub->data[s].pgc = ntohs (ptt->pgc);
 
                ptt++;
        }
 
 
 
        return ifo_ptt;
}
 
int ifo_is_vts (int fd)
{
        char ptr_descr[13];

        lseek (fd, 0, SEEK_SET);

        read (fd, ptr_descr, 12);
        ptr_descr[12] = '\0';

        if (!strcmp (ptr_descr, "DVDVIDEO-VTS")) {
		return 1;
	}

	return 0;
}

static size_t ifo_get_len (int fd, int offset)
{
	u_int ptr_len;
	size_t len;

	lseek (fd, offset, SEEK_SET);
	read (fd, &ptr_len, 4);

	len = ntohl (ptr_len); // * 0x800; 

	return len;
}

static void *_ifo_mmap (int fd, size_t len)
{
	void *ret;

	if ((ret = mmap (0, len, PROT_READ, MAP_SHARED, fd, 0)) < 0) {
		fprintf (stderr, "%s/%d: unable to mmap\n",
			__FILE__, __LINE__);
	}

	return ret;
}

u_int get4bytes (u_char *buf)
{
        u_int ret;

        ret  = buf[0] << 24;
        ret |= buf[1] << 16;
        ret |= buf[2] << 8;
        ret |= buf[3];

	return ret;
}

u_int get2bytes (u_char *buf)
{
        u_int ret;

        ret = buf[0] << 8;
        ret |= buf[1];

        return ret;
}

int ptt_num_chapters(ifo_ptt_t *ptt, int titlenum)
{
  ifo_ptt_sub_t *title = ptt->title+titlenum;
  print_info("\n\tTitle: 0x%x\tnumber of chapters: %d\n", titlenum, title->num);
  return title->num;
}
  

ifolib_t *ifo_pars(vob_info_t* vob_info){
  ifo_t *ifo;
  ifolib_t *ifolib;
  u_char *ptr;
  cell_addr_hdr_t *cell_addr_hdr;
  int fd;
  int fd2;
  offset_t len;
  u_int num_cells;
  u_int start_program_map;
  int i;
  u_char *ptr_pgc;
  u_char *ptr_pgci,*ptr_prog;
  ifo_hdr_t *hdr;
  u_int *start_list;
  call_stack_t *tmp_call_stack;

  if ((fd2 = fd = open (vob_info->ifo_file, O_RDONLY)) < 0) {
    fprintf (stderr, "error opening ifo file %s.\n",vob_info->ifo_file);
    vob_info->use_ifo=0;
    return;
  }
  vob_info->use_ifo=1;

  if (!(ifolib = ifoOpen (fd2, 0))) {
    fprintf (stderr, "error initializing ifo.\n");
    vob_info->use_ifo=0;
  }

  if (!(ifo = (ifo_t *) malloc (sizeof (ifo_t)))) {
    fprintf (stderr, "%s/%d: memory squeeze\n", __FILE__, __LINE__);
    vob_info->use_ifo=0;
    return;
  }
  
  if (!(len = ifo_get_len (fd, OFFSET_VTS_LEN))) {
    fprintf (stderr, "%s/%d: len @ offset 0x%x is ZERO\n",
	     __FILE__, __LINE__, OFFSET_VTS_LEN);
  }
  
  ifo->ifo = _ifo_mmap (fd, len);
  
  // Get address of pgci "Used in metode=1" 
  ifo->start_title_pgci		= get4bytes (ifo->ifo + 0xCC)
    ? ifo->ifo + get4bytes (ifo->ifo + 0xCC) * DVD_BLOCKSIZE : 0;
  // Get address of cell info "Used to get correct playing siquenc in the vob" 
  ifo->start_title_cell_addr	= get4bytes (ifo->ifo + 0xE0)
    ? ifo->ifo + get4bytes (ifo->ifo + 0xE0) * DVD_BLOCKSIZE : 0;
  
  // Is it a file stream IFO file
  if (ifo_is_vts (fd)) {
    // Find start of pgci.
    ptr_pgci = (u_char *) ifo->start_title_pgci;
    hdr = (ifo_hdr_t *) ptr_pgci;
    
    if (!ptr_pgci)
      return;
    start_list = (u_int *) calloc (ntohs (hdr->num), sizeof (u_int));
    
    ptr_pgci += IFO_HDR_LEN;
    
    for (i=0; i<ntohs(hdr->num); i++) {
      pgci_sub_t *pgci_sub = (pgci_sub_t *) ptr_pgci;
      
      start_list[i] = ntohl (pgci_sub->start);
      
      ptr_pgci += PGCI_SUB_LEN;
    }
    
    
    ptr_pgc = (u_char *) hdr + start_list[0];
    if (!ptr_pgc)
      return;
    ptr_pgc+=2; // Her pgc starts.
    num_cells = *ptr_pgc++;
    vob_info->num_cell_addr = *ptr_pgc++;	
    
    ptr_pgc += OFFSET_START_TBL_CMD - 2;
    start_program_map	 = get2bytes (ptr_pgc);
    // Store the program map in vob_info.
    if (start_program_map){
      ptr_prog=start_program_map+(u_char *) hdr + start_list[0];
      vob_info->ptr_pc=(u_char *)malloc(sizeof(u_char)*(vob_info->num_cell_addr-1));
      for (i=1; i<vob_info->num_cell_addr; i++) {
	vob_info->ptr_pc[i-1]=*ptr_prog;
	ptr_prog++;
      }
    }else{
      ptr_prog==NULL;
    }
    ptr=ifo->start_title_cell_addr;
    // Find start of cell map.
    cell_addr_hdr = (cell_addr_hdr_t *) ptr;
    if (!ptr)
      return;
    
    //    len=ntohl (cell_addr_hdr->len)*0x800;
    ptr += CADDR_HDR_LEN; // This is the start of the cell map.
    
  }else{
    vob_info->use_ifo=0;
    return;
  }
  // Copy info about vob navigation in a usable form.
  vob_info->call_stack=(call_stack_t *)malloc(sizeof(call_stack_t)*(vob_info->num_cell_addr-1));
  vob_info->num_cell_addr=0;
  i=0;

  while (ptr<((u_char *) cell_addr_hdr) + ntohl (cell_addr_hdr->len)){
    vob_info->num_cell_addr++;
    vob_info->call_stack[i].vob_id=ntohs(((cell_addr_t *)ptr)->vob_id);
    vob_info->call_stack[i].cell_id=((cell_addr_t *)ptr)->cell_id;
    vob_info->call_stack[i].start=ntohl(((cell_addr_t *)ptr)->start);
    vob_info->call_stack[i].end=ntohl(((cell_addr_t *)ptr)->end);
    vob_info->call_stack[i].next=(call_stack_t *)malloc(sizeof(call_stack_t));
    tmp_call_stack=&vob_info->call_stack[i];
    ptr += CADDR_LEN; 
    while(ptr<((u_char *) cell_addr_hdr) + ntohl (cell_addr_hdr->len) &&
	  ntohs (((cell_addr_t *)ptr)->vob_id)==i ){
      tmp_call_stack=tmp_call_stack->next;
      tmp_call_stack->vob_id=ntohs(((cell_addr_t *)ptr)->vob_id);
      tmp_call_stack->cell_id=((cell_addr_t *)ptr)->cell_id;
      tmp_call_stack->start=ntohl(((cell_addr_t *)ptr)->start);
      tmp_call_stack->end=ntohl(((cell_addr_t *)ptr)->end);
      tmp_call_stack->next=(call_stack_t *)malloc(sizeof(call_stack_t));
      ptr += CADDR_LEN; 
    }
    tmp_call_stack->next=NULL;
    i++;
  }
  free(ifo);
  free(start_list);

  return ifolib;

}
