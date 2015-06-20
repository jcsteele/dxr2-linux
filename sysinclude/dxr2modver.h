/*
  **********************************************************************
  *
  *     
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     March 6, 2002        Gerald Henriksen     File added
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

/*
 ***********************************************************************
 *
 * Added ability for compiler to determine whether module version
 * support is required
 *
 * This must be the first file included in any source files that require
 * it.
 ***********************************************************************
 */


#ifndef __DXR2MODVER_H__
#define __DXR2MODVER_H__


#include <linux/config.h> 
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#  define MODVERSIONS
#endif

#ifdef MODVERSIONS
#  include <linux/modversions.h>
#endif

#endif
