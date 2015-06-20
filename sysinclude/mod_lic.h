/*
  **********************************************************************
  *
  *     Copyright 1999, 2000, 2001 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     December 18, 2001    Scott Bucholtz       File added
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
 * Add support for MODULE_* functions in kernels >= 2.4.10
 * and modutils >= 2.4.9
 * NOTE:  This has no obvious effect if modutils is < 2.4.9 but it
 * shouldn't hurt anything.
 *
 ***********************************************************************
 */


#ifndef __MOD_LIC_H__
#define __MOD_LIC_H__

#include <linux/version.h>

/*
 ********************************************
 * Replace the next line with:              *
 *    #ifdef MODULE                         *
 * once the dirver gets added to the kernel *
 ********************************************
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
  MODULE_DESCRIPTION("Creative Labs Dxr2 MPEG decoder card driver");
  MODULE_SUPPORTED_DEVICE("dxr2");
  MODULE_LICENSE("GPL");
#endif

#endif
