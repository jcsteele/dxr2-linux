/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *   
  *     October 20, 1999     Andrew de Quincey    Rewrote and extended
  *                          Lucien Murray-Pitts  original incomplete 
  *                                               driver.
  *
  *     April 18, 1999       Andrew Veliath       Original Driver
  *                                               implementation
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

#ifndef __DVD_CONFIG
#define __DVD_CONFIG
#include "dxr2-api.h"

void initialize(int, char**, player_info_t*, dxr2_status_info_t*);
void init_dxr2_info(dxr2_status_info_t*);
int  init_and_parse_args(int, char**, player_info_t*, dxr2_status_info_t*);
void open_files(player_info_t*);
int  open_new_vob(player_info_t*, char*);
void init_dxr2(player_info_t*, dxr2_status_info_t*);
void install_firmware(player_info_t*);
void authenticate_disc(player_info_t*);

#endif
