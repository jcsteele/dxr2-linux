/* $Id: multifile.c,v 1.2 2002/02/20 02:33:22 nmtlinuxman Exp $ */
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "player.h"
#include "interface.h"
#include "multifile.h"

#ifndef FIBMAP
#define FIBMAP 1
#endif

static void lba_to_msf(int lba, char* min, char* sec, char* frame) 
{*frame=lba % 35; lba /= 75; lba +=2; *sec=lba % 60; *min = lba / 60;}

static struct mfile {
  char* name;
  int fd, size, lba;
} files[10];

static int numfiles;
static int curFile;
static int current_lba;

int multi_open(char** filenames, int get_lba) {
  int i;
  struct stat statbuf;

  for(i = 0; i < 10 && filenames[i]; ++i) {
    if ((files[i].fd = open(filenames[i], O_RDONLY)) >= 0 &&
	fstat(files[i].fd, &statbuf) >= 0 ) {
      files[i].name = filenames[i];
      files[i].size = statbuf.st_size/0x800;
      if(get_lba)
	ioctl(files[i].fd, FIBMAP, &files[i].lba);
      else
	files[i].lba = -1;
      ++numfiles;
    } else return -1;
  }

  curFile = 0;
#ifdef __DVD_DEBUG
  print_info("Opening file %s.\n", filenames[curFile]);
#endif

  current_lba = files[curFile].lba;
  return files[curFile].fd;
}

ssize_t multi_read(void *buf, size_t count) {
  ssize_t size;
  int i;
  struct mfile *file;
  struct cdrom_msf* cdbuf = buf;

  file = &files[curFile];

  if (curFile == numfiles) {
#ifdef __DVD_DEBUG
    print_info("last file read.\n");
#endif
    return 0;
  }

  if(file->lba >= 0) {
    int frame = current_lba;
    lba_to_msf(frame, &cdbuf->cdmsf_min0, &cdbuf->cdmsf_sec0, &cdbuf->cdmsf_frame0);
    frame = current_lba + count;
    lba_to_msf(frame, &cdbuf->cdmsf_min0, &cdbuf->cdmsf_sec0, &cdbuf->cdmsf_frame1);
    size = ioctl(file->fd, CDROMREADMODE2, &cdbuf);
    current_lba += size;
  }
  else
    size = read(file->fd, buf, count);

  if (size == -1) {
#ifdef __DVD_DEBUG
    print_info("invalid read.\n");
#endif
    return -1;
  }

  if (size < count) {
#ifdef __DVD_DEBUG
    print_info("finished file %s.\n", files[curFile].name);
#endif
    if( ++curFile >= numfiles)
      return size;

#ifdef __DVD_DEBUG
    print_info("continuing with file %s.\n", files[curFile].name);
#endif
    current_lba = files[curFile].lba;

    if(files[curFile].lba >= 0) {
      int frame = current_lba;
      lba_to_msf(frame, &cdbuf->cdmsf_min0, &cdbuf->cdmsf_sec0, &cdbuf->cdmsf_frame0);
      frame = current_lba + count-size;
      lba_to_msf(frame, &cdbuf->cdmsf_min0, &cdbuf->cdmsf_sec0, &cdbuf->cdmsf_frame1);
      size += ioctl(files[curFile].fd, CDROMREADMODE2, &cdbuf);
      current_lba += size;
    }
    else
      size += read(files[curFile].fd, buf + size, count - size);
  }

  return size;
}

int multi_close() {
	int i;

	for (i = 0; i < numfiles; ++i)
	  if (close(files[i].fd) == -1)
	    return -1;
	return 0;
}

offset_t multi_lseek(offset_t offset, int whence) {
  struct stat statbuf;
  struct mfile *file;
  int i;
  offset_t tmpofs;
  offset_t cursor;

  file = &files[curFile];

  if (whence == SEEK_SET) {
    curFile = 0;
    tmpofs = offset;
    while (curFile < numfiles &&
	   tmpofs > files[curFile].size) {
      tmpofs -= files[curFile++].size;
    }
    if (curFile >= numfiles)
      return -1;
    if (lseek(files[curFile].fd, tmpofs*0x800, SEEK_SET) == -1)
      return -1;
    //curFile = curFile;
    current_lba = files[curFile].lba + offset;
    return offset;
  } else if (whence == SEEK_CUR) {
    offset_t curpos=0;
    if ((curpos = lseek(file->fd, 0, SEEK_CUR)) == -1)
      return -1;
    curpos/=0x800;
    for (i = 0; i < curFile; i++)
      curpos += files[i].size;
    return multi_lseek(curpos+offset, SEEK_SET);
  } else if (whence == SEEK_END) {
    assert(0);
  }
}

char** generate_filenames(const char* orig_file_name)
{
  char** names = (char**) malloc( 10 * sizeof(char*) );
  int i, t;
  char nr='1';
  char* file_name;

  file_name = strdup(orig_file_name);
  
  if(!strcmp(file_name, orig_file_name))
    strcpy(file_name, orig_file_name);

  if (strlen(file_name) < 6) {
    names[0] = strdup(file_name);
    names[1] = 0;
  } else {
    for (i = 0; i < 10; ++i){
      if ((t = open(file_name, O_RDONLY)) != -1) {
	names[i] = strdup(file_name);
	file_name[strlen(file_name)-5]++;
	close(t);
#ifdef __DVD_DEBUG
	print_info("Added file: %s\n", names[i]);
#endif
      } else {
	names[i] = 0;
	break;
      }
    }
  }
  return names;
}

