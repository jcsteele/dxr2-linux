#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "bookmarks.h"
#include "interface.h"

#define MAGIC_NUM 0x424242

/* Internal types */

struct bookmark_t;

typedef struct {
  char name[60];
  __off_t offset;
  void* next;
} bookmark_t;

typedef struct {
  int magic_num;
  char set_title[60];
  int num_bookmarks;
  bookmark_t* first_bookmark;
  bookmark_t* last_bookmark;
} bookmark_set_t;

/* internal functions */

int add_bookmark(char*, offset_t, int);

/* internal variables */

static bookmark_set_t bookmark_set;
static int bookmarks = 0;

int load_bookmarks(char* file_name)
{
  int bookmarkFD=-1;
  int i;
  bookmark_t new_bookmark;
  
  memset(&bookmark_set, 0, sizeof(bookmark_set));

  bookmarkFD = open(file_name, O_RDONLY);
  if(bookmarkFD != -1) {
    read(bookmarkFD, &bookmark_set, sizeof(bookmark_set));
    bookmark_set.first_bookmark = NULL;
    bookmark_set.last_bookmark = NULL;

    for(i=0; i < bookmark_set.num_bookmarks; ++i) {
      read(bookmarkFD, &new_bookmark, sizeof(new_bookmark));
      add_bookmark(new_bookmark.name, new_bookmark.offset, -1);
    }
    close(bookmarkFD);
    bookmarks = 1;
    
    return 1;
  }
  else
    print_error("Can not open bookmark file: %s", file_name);
  
  return 0;
}

int write_bookmarks(char* file_name)
{
  int bookmarkFD = -1;
  int i;
  bookmark_t* current;

  if(!bookmarks)
    return 1;

  // create it if it doesn't exist.
  bookmarkFD = open(file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  if(bookmarkFD != -1) {
    if(!bookmarks || bookmark_set.first_bookmark == NULL) {
      print_error("No bookmarks to save.\n");
      return 1;
    }
    
    write(bookmarkFD, &bookmark_set, sizeof(bookmark_set));
    current = bookmark_set.first_bookmark;

    while(current != NULL) {
      write(bookmarkFD, current, sizeof(bookmark_t));
      current = current->next;
    }

    close(bookmarkFD);
  }
  else
    print_error("Can not open/create bookmark file: %s", file_name);

  return 0;
}

void print_bookmarks()
{
  bookmark_t* current = bookmark_set.first_bookmark;
  int i = 0;
  
  if(bookmarks) {
    print_info("\n");
    print_info("%s\n", bookmark_set.set_title);
    print_info("---------------------\n");
    while(current != NULL) {
      print_info("%d: %s %d\n", i++, current->name, current->offset);
      current = current->next;
    }
  }
}

int create_bookmark_at(offset_t offset)
{
  char buf[60];
  int idx;
  bookmark_t* current;

  if(!bookmarks) {
    memset(&bookmark_set, 0, sizeof(bookmark_set));
    print_info("New bookmark set!\n");
    print_info("What do you want to call it?\n");
    print_info("\t");
    get_a_str(bookmark_set.set_title, 60);
    bookmarks = 1;
  }
  print_info("What do you want to call this bookmark?\n");
  print_info("\t");
  get_a_str(buf, 60);
  idx = 0;
  current = bookmark_set.first_bookmark;

  while(current != NULL && offset > current->offset) {
    ++idx;
    current = current->next;
  }
  if(current==NULL)
    idx = -1;

  ++(bookmark_set.num_bookmarks);
  return add_bookmark(buf, offset, idx);
}

int choose_bookmark()
{
  char buf[4];
  int  idx = -1;

  if(!bookmarks)
    return -1;

  buf[0] = 'a';
  print_bookmarks();
  while(buf[0] != 'q' && buf[0] != 'Q' && ( idx < 0 || idx > bookmark_set.num_bookmarks-1 ) ) {
    print_info("Please choose a bookmark [0 - %d  q to quit]", bookmark_set.num_bookmarks-1);
    get_a_str(buf, 4);
    idx = atoi(buf);
  }

  return (buf[0]=='q' || buf[0]=='Q') ? -1 : idx;
}

offset_t bookmark_offset(int idx)
{
  bookmark_t* current = bookmark_set.first_bookmark;

  if(idx < 0 || idx > bookmark_set.num_bookmarks)
    return -1;

  for(; idx > 0; --idx)
    current = current->next;

  return current->offset;
}

/* Internal definitions */

int add_bookmark(char* name, offset_t offset, int index)
{
  bookmark_t* bookmark;
  bookmark_t* current, *prev;
  int i;

  print_info("adding bookmark at index %d\n", index);
  current = bookmark_set.first_bookmark;
  prev = current;
  bookmark = (bookmark_t*) malloc( sizeof(bookmark_t));
  
  if(index==-1)
    current = bookmark_set.last_bookmark;
  else {
    for(i=0; i < index && current != NULL; ++i) {
      prev = current;
      current = current->next;
    }

    current = prev;
  }

  strncpy(bookmark->name, name, 60);
  bookmark->offset = offset;

  if(bookmark_set.first_bookmark == NULL) {
    // empty set
    bookmark->next = NULL;
    bookmark_set.first_bookmark = bookmark;
    bookmark_set.last_bookmark = bookmark;
  }
  else if(index==0) {
    bookmark->next = current;
    bookmark_set.first_bookmark = bookmark;
  }
  else {
    if(bookmark_set.last_bookmark = current) 
      bookmark_set.last_bookmark = bookmark;

    bookmark->next = current->next;
    current->next = bookmark;
  }

  return 1;
}
