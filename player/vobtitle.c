#include <stdio.h>
#include "player.h"
#include "multifile.h"
#include "ifo.h"
#include "interface.h"

static call_stack_t* cell_stack;
static int id;
static int current;
static player_info_t* player_info;
static offset_t offset;
static int skip_angles;
static int angle;
static int metode;
static vob_info_t vob_info;
static ifolib_t* ifolib;
static ifo_ptt_t* ptt;
static ifo_ptt_sub_t* title;
static int numchapters;
static unsigned long elapsed_secs;
static int frame_tics;
static scan_forwards;

static void free_mem(player_info_t*);
static offset_t vob_recenter_cell(offset_t);
static offset_t vob_next_cell(offset_t);

int open_vob(char* file_name)
{
  char** names = generate_filenames(file_name);

#ifdef __DVD_DEBUG
  print_info("VOB title\n");
#endif
  strcpy(vob_info.ifo_file,file_name);
  if(file_name[strlen(vob_info.ifo_file) - 1]=='B'){
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 5]='0';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 3]='I';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 2]='F';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 1]='O';
  }else{
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 5]='0';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 3]='I';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 2]='F';
    vob_info.ifo_file[strlen(vob_info.ifo_file) - 1]='O';
  }
  elapsed_secs=0;
  frame_tics=0;

  ifolib = ifo_pars(&vob_info);

/* Debugging
  fprintf(stderr,"Num Menu Vobs: %d\n", ifolib->num_menu_vobs);
  fprintf(stderr,"Vob start: %d\n", ifolib->vob_start);
*/
  scan_forwards=0;

  if(vob_info.use_ifo) {
    current = 0;
    cell_stack = &vob_info.call_stack[0];
    id = cell_stack->cell_id;
    metode = 0;
    angle = 0;
    skip_angles=0;
    offset = cell_stack->start;

    if(cell_stack->next!=NULL && id==cell_stack->next->cell_id){  
      // We don't have cell_id info in call_stac so try using pc in stead.
      metode=1;
      if(vob_info.ptr_pc!=NULL) 
	cell_stack=&vob_info.call_stack[vob_info.ptr_pc[current]];
      else
	cell_stack=&vob_info.call_stack[current]; // No pc use simpel counter.
    }

    return multi_open(names, 0);
  }
  else return -1;
}

offset_t vob_read(char* buffer, int len)
{
  offset_t size = -1;
  int vob_id = cell_stack->vob_id;
  unsigned long secs, mins, hrs;

  if(vob_info.use_ifo){
    size=-1;

      if(offset == -1) {
	offset = vob_next_cell(offset);
	offset = multi_lseek(offset-1, SEEK_SET);
      }

      if(scan_forwards) {
#ifdef __DVD_DEBUG
	print_info("scanning!\n");
#endif
	offset +=  FRAME_SIZE / 0x800;
	vob_lseek(offset, SEEK_SET);
	scan_forwards=0;
      }

      while(offset >= cell_stack->end && offset != -1) {
	offset = vob_next_cell(offset);
	if(offset>=0)
	  offset = multi_lseek(offset-1, SEEK_SET);
      }

      if(offset == -1)
	return -1;
      // Read date 0x800 is to get around addressing problems.
      // 0x10 = framesize / 0x800
      if(offset+0x10 <= cell_stack->end){
	size = multi_read(buffer, FRAME_SIZE);
	offset += FRAME_SIZE / 0x800;
      }
      else { // not a full frame.
	//offset += 10;
	size = multi_read(buffer, (cell_stack->end - offset) * 0x800);
	offset = cell_stack->end;
      }
  }
  else{
    size = multi_read(buffer, FRAME_SIZE);
  }

  if(size > 0) {
    frame_tics = (frame_tics+1) % 30;
    if(frame_tics==0)
      ++elapsed_secs;
  }

  return size;
}

void close_vob(){
  int i;
  call_stack_t *cell_stack,*cell_stack1;
  free(vob_info.ptr_pc);
  vob_info.ptr_pc=NULL;  
  for(i=0;i<vob_info.num_cell_addr-1;i++){
    cell_stack=vob_info.call_stack[i].next;
    while(cell_stack->next!=NULL){
      cell_stack1=cell_stack;
      cell_stack=cell_stack->next;
      free(cell_stack1);
    }
    free(cell_stack);
    vob_info.call_stack[i].next=NULL;
  }  
}

offset_t vob_lseek(offset_t new_offset, int type)
{
  offset = multi_lseek(new_offset, type);

  return vob_recenter_cell(offset);
}

offset_t vob_recenter_cell(offset_t _offset)
{
  offset_t new_offset;

  current = 0;
  cell_stack = vob_info.call_stack;

  while(_offset >= cell_stack->end)
    new_offset = vob_next_cell(_offset);

  offset = multi_lseek(_offset, SEEK_SET); 

  return offset;
}

offset_t vob_next_chapter()
{
  int vob_id = cell_stack->vob_id;
  offset_t _offset = offset;

  if(vob_id+1 >= vob_num_chapters())
    return _offset;

  while(cell_stack != NULL && cell_stack->vob_id <= vob_id)
    vob_next_cell(cell_stack->end);

  offset = multi_lseek(cell_stack->start, SEEK_SET);

  return offset;
}

offset_t vob_prev_chapter()
{
  return vob_chapter_seek(cell_stack->vob_id - 1);
}

int vob_num_chapters()
{
  // u_int foo  = vob_info.num_cell_addr-1;
  //  int vob_id = cell_stack->vob_id;

  //  print_info("Vob ID: %d\n", vob_id);

  ptt = ifo_get_ptt(ifolib);
  title = ptt->title;

  return title->num;
}

offset_t vob_chapter_seek(int vob_id)
{
  offset_t _offset = offset;

  if(vob_id < 0 || vob_id >= vob_num_chapters())
    return _offset;

  if(vob_id >= cell_stack->vob_id)
    while(vob_id > cell_stack->vob_id && vob_id < vob_num_chapters())
      _offset = vob_next_chapter(_offset);
  else {

    current = 0;
    cell_stack = vob_info.call_stack;

    while(cell_stack != NULL && vob_id > cell_stack->vob_id)
      vob_next_cell(cell_stack->end);

    _offset = multi_lseek(cell_stack->start, SEEK_SET); 
  }

  offset = _offset;

  return _offset;
}

offset_t vob_next_cell(offset_t offset)
{
  int cur_id;

#ifdef __DVD_DEBUG
  print_info("New cell needed at offset %x.\n", offset);
  print_info("start: %x  end: %x\n", cell_stack->start, cell_stack->end);
#endif
  if(cell_stack->next!=NULL){ // More movie in current vob_id.

    // fix to allow selectible angles.
    while(cell_stack->next->next!=NULL && cell_stack->start==cell_stack->next->start){
      // skip multi angles
      cell_stack=cell_stack->next;
    }
    cell_stack=cell_stack->next;
  }else{
#ifdef __DVD_DEBUG
    print_info("New stack needed.\n");
    print_info("id: %d  cell_stack->cell_id: %d  current: %d", id, cell_stack->cell_id, current);
#endif
    // We need to find a new vob_id to play from.
    if(id!=-1)  // Reinitialize id. 
      id= current ? cell_stack->cell_id : 0;

    current++; // Next vob_id
#ifdef __DVD_DEBUG
    print_info("Selected stack %d\n", current);
#endif

    if(current>=vob_info.num_cell_addr){ // Are We at the end of the movie.
      return -1;
    }

    if(metode==1){ // Metode 1 Navigation via pc and call_stack.
      if(vob_info.ptr_pc!=NULL)
	cell_stack=&vob_info.call_stack[vob_info.ptr_pc[current]];
      else
	cell_stack=&vob_info.call_stack[current]; // No pc use simple counter.
    }else{ // Navigation via call_stack only
      cell_stack=&vob_info.call_stack[current];
    }

#ifdef __DVD_DEBUG
    print_info("New start: %x\n", cell_stack->start);
#endif

    if(cell_stack->next!=NULL){ // There is more than one cell in this vob_id.

      // If a multiangled disk, we need to skip new cell stacks that are the
      // Same as the one we just played.  DIFFERENT from fixing the flip-flop bug.
      if(skip_angles==0){ 
	// Is this multiangled.
	if(cell_stack->next!=NULL&&cell_stack->cell_id==cell_stack->next->cell_id)
	  skip_angles=1;
      }else{
	// OK we are in a multiangled section.
	// We need a cell_id difrent from the one we have now.
	// EVAN to self:  Why does this work? apparently, it should skip through
	//                multiangled sections.  But it doesn't.
	while(cell_stack->next!=NULL&&cell_stack->cell_id==cell_stack->next->cell_id){
	  current++;
	  if(current>=vob_info.num_cell_addr){
#ifdef __DVD_DEBUG
	    print_info("quitting read proc...\n");
#endif
	    pthread_exit(NULL);
	  }
	  cell_stack=&vob_info.call_stack[current];
	}
	// Now whee are out of the multiangled part.
	skip_angles=0;
      }

      // cell_id has to change when vob_id change.
      while(id==cell_stack->cell_id){
	if(cell_stack->next!=NULL){
	  print_info("Skipping cell: id: %x start: %x end: %x\n", cell_stack->cell_id, cell_stack->start, cell_stack->next);
	  cell_stack=cell_stack->next;
	}else{
	  current++;
	  if(current>=vob_info.num_cell_addr){
	    return -1;
	  }
	  cell_stack=&vob_info.call_stack[current];
	}
      }
    }else{
      id=-1;
    }
  }

  return cell_stack->start;
}      

int vob_choose_angle(int new_angle)
{
  angle = new_angle;
  return angle;
}


int vob_scan_forwards()
{
  scan_forwards=1;
}
