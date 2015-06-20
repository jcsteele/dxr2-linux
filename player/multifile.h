/* $Id: multifile.h,v 1.2 2002/02/20 02:33:33 nmtlinuxman Exp $ */
#ifndef MULTIFILE_H
#define MULTIFILE_H

#include <unistd.h>
#include <sys/types.h>

typedef __off_t offset_t;

extern int multi_open(char **filenames, int get_lba);
extern ssize_t multi_read(void *buf, size_t count);
extern int multi_close();
extern offset_t multi_lseek(offset_t offset, int whence);
extern char** generate_filenames(const char*);


#endif
