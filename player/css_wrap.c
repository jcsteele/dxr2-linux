/**********************************************************************
  *
  *     Copyright 2002 James Hawtin oolon@ankh.org
  *
  **********************************************************************
  *
  *     This program is free software; you can redistribute it and/or
  *     modify it under the terms of the GNU General Public License as
  *     published by the Free Software Foundation; either version 2 of
  *     the License, or (at your option) any later version.
  *
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *
  *     You should have received a copy of the GNU General Public
  *     License along with this program; if not, write to the Free
  *     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
  *     USA.
  *
  **********************************************************************
  */

/* This file provides an API for libdvdread similar to that of libcss
 It should however be notited it contains NO CSS secrets
 */ 

#include "css.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define DVD_VIDEO_LB_LEN 2048

dxr2_css_info *dvdcss_open(char *psz_target, int i_flags )
{
  int driveFD;
  dxr2_css_info *info;

  if ((driveFD = open64(psz_target, O_RDONLY))<0)
  {
    return 0;
  }

  if (!(info = dxr2_css_open(driveFD)))
  {
    close(driveFD);
    return 0;
  }
  
  return info;
}

int dvdcss_close(dxr2_css_info *info)
{
  int driveFD = info->driveFD;

  dxr2_css_close(info);
  close(driveFD);
  return 1;
}

int dvdcss_title(dxr2_css_info *info, int i_block )
{
  if (!dxr2_css_title(info, i_block))
    return 1;
  return 0;
}

int dvdcss_seek(dxr2_css_info *info, int lb_number)
{
  off64_t off;
  off64_t calc = lb_number * (int64_t) DVD_VIDEO_LB_LEN;

  if(info->driveFD < 0)
  {
    fprintf( stderr, "libdxr2lib: Fatal error in block read.\n" );
    return -1;
  }

  off = (off64_t) lseek64(info->driveFD, calc, SEEK_SET);

  /*
   * The top part of the long long seems to be sign extended from 32 bits so lets just test the bottom 32 bits and
   * hope for the best
   */
  
  if((off & 0xFFFFFFFF) != (calc & 0xFFFFFFFF))
  {
    fprintf(stderr, "libdxr2lib: Can't seek to block %u\n", lb_number);
    return -1;
  }

  return lb_number;
}

int dvdcss_read(dxr2_css_info *info, void *buffer, int i_blocks, int i_flags)
{
  int ret;

  
  if ((ret=read(info->driveFD, buffer, i_blocks*DVD_VIDEO_LB_LEN))<0)
    return ret;

  return ret/DVD_VIDEO_LB_LEN;
}

char *dvdcss_error(dxr2_css_info *info)
{
  return "";
}

/* An extra function to tell libdvdcss about the abilites of the library */
/* its a list of ORED abilities */
/* 1 Don't use pretitle hack */

int dvdcss_libabilities(void)
{
  return 1;
}
