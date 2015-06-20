/*
  **********************************************************************
  *
  *     Copyright 1999, 2000 Creative Labs, Inc.
  *
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
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

/**
 *
 * Driver for the Brooktree/Rockwell/Conexant BT865 TV encoder chip
 * High level convenience functions
 *
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <bt865.h>


/**
 *
 * Initialises the bt865 chip
 *
 * @param instance bt865 instance to use
 *
 */

extern int bt865_init(bt865_t* instance)
{
  // changed by zulli
//  int tmp;

  // reset the chip
  bt865_set_reg(instance, 0xA6, 0x80);
  
  // set TXHS[7:0] to 0
  bt865_set_reg(instance, 0xAC, 0);

  // set TXHE[7:0] to 0
  bt865_set_reg(instance, 0xAE, 0);

  // set TXHS[10:8], TXHE[10:8], LUMADLY[1:0] to 0
  bt865_set_reg(instance, 0xB0, 0);

  // basically, disable teletext
  bt865_set_reg(instance, 0xB2, 0);

  // I bet this turns macrovision support on
  bt865_set_reg(instance, 0xBC, 0x10);

  // noninterlaced mode, setup off, NTSC among other things
  bt865_set_reg(instance, 0xCC, 0x42);
  instance->palMode = 0;

  // normal video mode, ESTATUS = 0
  bt865_set_reg(instance, 0xCE, 2);
  
  // OK
  return(0);
}
  
  

/**
 *
 * Sets the bt865 output mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_OUTPUT_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_output_mode(bt865_t* instance, int mode) 
{
  switch(mode) {
  case BT865_OUTPUT_NTSC:

    instance->palMode = 0;
    bt865_set_bits(instance, 0xCC, 0xBC, 0x80);

    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;
    
  case BT865_OUTPUT_NTSC_60:

    instance->palMode = 0;
    bt865_set_bits(instance, 0xCC, 0xBC, 0x88);
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 8); // SCRESET ON
    break;

  case BT865_OUTPUT_PAL_M:

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xF0); // PAL M mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;

  case BT865_OUTPUT_PAL_M_60:

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xF8); // PAL M-60 mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;

  case BT865_OUTPUT_PAL_BDGHI:

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xE4); // PAL BDGHI mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;

  case BT865_OUTPUT_PAL_N:

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xE4); // PAL N mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 1); // PAL-N mode on
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;

  case BT865_OUTPUT_PAL_Nc:

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xF4); // PAL Nc mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off???
    bt865_set_bits(instance, 0xCE, 8, 0); // SCRESET OFF
    break;

  case BT865_OUTPUT_PAL_60: // Unsure what this is... but some people seem to need it

    instance->palMode = 1;
    bt865_set_bits(instance, 0xCC, 0xFC, 0xE0); // PAL 60 mode ON, 7.5 IRE OFF
    bt865_set_bits(instance, 0xD0, 1, 0); // PAL-N mode off
    bt865_set_bits(instance, 0xCE, 8, 8); // SCRESET ON
    break;
    
  default:
    
    return(-EINVAL);
  }

  return(0);
}



/**
 *
 * Set/unset the black/white mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_BLACKWHITE_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_blackwhite_mode(bt865_t* instance, int mode) 
{
  switch(mode) {
  case BT865_BLACKWHITE_OFF:
    
    bt865_set_bits(instance, 0xCE, 0x20, 0);
    break;

  case BT865_BLACKWHITE_ON:
    
    bt865_set_bits(instance, 0xCE, 0x20, 0x20);
    break;
    
  default:
    
    return(-EINVAL);
  }
  
  return(0);
}



/**
 *
 * Set/unset the pixel mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_PIXEL_* defines
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_pixel_mode(bt865_t* instance, int mode) 
{
  switch(mode) {
  case BT865_PIXEL_CCIR601:
    
    bt865_set_bits(instance, 0xCC, 1, 0);
    break;

  case BT865_PIXEL_SQUARE:
    
    bt865_set_bits(instance, 0xCC, 1, 1);
    break;
    
  default:
    
    return(-EINVAL);
  }
  
  return(0);
}



/**
 *
 * Set/unset interlaced mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_INTERLACED_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_interlaced_mode(bt865_t* instance, int mode) 
{
  switch(mode) {
  case BT865_INTERLACED_OFF:
    
    bt865_set_bits(instance, 0xCC, 2, 2);
    break;

  case BT865_INTERLACED_ON:
    
    bt865_set_bits(instance, 0xCC, 2, 0);
    break;
    
  default:
    
    return(-EINVAL);
  }
  
  return(0);
}



/**
 *
 * Set/unset 7.5IRE mode
 *
 * @param instance bt865 instance to use
 * @param mode One of the BT865_75IRE_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_75IRE_mode(bt865_t* instance, int mode) 
{

  switch(mode) {
  case BT865_75IRE_OFF:
    
    bt865_set_bits(instance, 0xCC, 0x40, 0x40);
    break;

  case BT865_75IRE_ON:
    
    bt865_set_bits(instance, 0xCC, 0x40, 0);
    break;
    
  default:
    
    return(-EINVAL);
  }
  
  return(0);
}


/**
 *
 * Set the macrovision mode.
 *
 * @param instance bt865 instance to use
 * @param macrovisionMode (one of BT865_MACROVISION_*)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int bt865_set_macrovision_mode(bt865_t* instance, int macrovisionMode)
{
  int tmp;

  // get BT865 mode. if it's a bt864, return now, since it doesn't support macrovision
  tmp = bt865_get_reg(instance, BT865_READBACK);
  if (tmp != 0xa0) {
    
    return(0);
  }

  // if we're in PAL mode, only AGC is defined.... and it's a special one for PAL
  if ((instance->palMode) && (macrovisionMode != BT865_MACROVISION_OFF)) {
    
    macrovisionMode = -1; // special mode for PAL AGC
  }

  // OK, do it
  switch(macrovisionMode) {
  case BT865_MACROVISION_OFF:
    
    bt865_set_reg(instance, 0xd8, 0);
    break;
    
  case BT865_MACROVISION_AGC:
    
    bt865_set_reg(instance, 0xd8, 0);
    bt865_set_reg(instance, 0xdA, 0x60);
    bt865_set_reg(instance, 0xdC, 0xF);
    bt865_set_reg(instance, 0xdE, 0xF);
    bt865_set_reg(instance, 0xE0, 0x0);
    bt865_set_reg(instance, 0xE2, 0);
    bt865_set_reg(instance, 0xE4, 0xFC);
    bt865_set_reg(instance, 0xE6, 3);
    bt865_set_reg(instance, 0xE8, 0xB9);
    bt865_set_reg(instance, 0xEA, 0x6D);
    bt865_set_reg(instance, 0xEC, 0xb6);
    bt865_set_reg(instance, 0xEE, 0xd5);
    bt865_set_reg(instance, 0xF0, 0xb0);
    bt865_set_reg(instance, 0xF2, 0x72);
    bt865_set_reg(instance, 0xF4, 0x0D);
    bt865_set_reg(instance, 0xF6, 0xff);
    bt865_set_reg(instance, 0xF8, 0x2c);
    bt865_set_reg(instance, 0xFA, 0xd0);
    bt865_set_reg(instance, 0xD8, 0x36);
    break;
    
  case BT865_MACROVISION_AGC_2COLOURSTRIPE:
    
    bt865_set_reg(instance, 0xd8, 0);
    bt865_set_reg(instance, 0xdA, 0x60);
    bt865_set_reg(instance, 0xdC, 0xF);
    bt865_set_reg(instance, 0xdE, 0xF);
    bt865_set_reg(instance, 0xE0, 0);
    bt865_set_reg(instance, 0xE2, 0);
    bt865_set_reg(instance, 0xE4, 0xFC);
    bt865_set_reg(instance, 0xE6, 3);
    bt865_set_reg(instance, 0xE8, 0xB9);
    bt865_set_reg(instance, 0xEA, 0x6D);
    bt865_set_reg(instance, 0xEC, 0x3c);
    bt865_set_reg(instance, 0xEE, 0xd1);
    bt865_set_reg(instance, 0xF0, 0x32);
    bt865_set_reg(instance, 0xF2, 0xd2);
    bt865_set_reg(instance, 0xF4, 0x0D);
    bt865_set_reg(instance, 0xF6, 0xff);
    bt865_set_reg(instance, 0xF8, 0x2c);
    bt865_set_reg(instance, 0xFA, 0xd0);
    bt865_set_reg(instance, 0xD8, 0x3e);
    break;

  case BT865_MACROVISION_AGC_4COLOURSTRIPE:
    
    bt865_set_reg(instance, 0xd8, 0);
    bt865_set_reg(instance, 0xdA, 0x60);
    bt865_set_reg(instance, 0xdC, 0xF);
    bt865_set_reg(instance, 0xdE, 0xF);
    bt865_set_reg(instance, 0xE0, 0);
    bt865_set_reg(instance, 0xE2, 0);
    bt865_set_reg(instance, 0xE4, 0xFC);
    bt865_set_reg(instance, 0xE6, 3);
    bt865_set_reg(instance, 0xE8, 0xB9);
    bt865_set_reg(instance, 0xEA, 0x6D);
    bt865_set_reg(instance, 0xEC, 0xB6);
    bt865_set_reg(instance, 0xEE, 0xd5);
    bt865_set_reg(instance, 0xF0, 0xb0);
    bt865_set_reg(instance, 0xF2, 0x72);
    bt865_set_reg(instance, 0xF4, 0x0D);
    bt865_set_reg(instance, 0xF6, 0xff);
    bt865_set_reg(instance, 0xF8, 0x2c);
    bt865_set_reg(instance, 0xFA, 0xd0);
    bt865_set_reg(instance, 0xD8, 0x3e);
    break;

  case -1: // PAL AGC
    
    bt865_set_reg(instance, 0xd8, 0);
    bt865_set_reg(instance, 0xdA, 0x60);
    bt865_set_reg(instance, 0xdC, 0x7E);
    bt865_set_reg(instance, 0xdE, 0xFE);
    bt865_set_reg(instance, 0xE0, 0x54);
    bt865_set_reg(instance, 0xE2, 1);
    bt865_set_reg(instance, 0xE4, 0xFF);
    bt865_set_reg(instance, 0xE6, 1);
    bt865_set_reg(instance, 0xE8, 0xD5);
    bt865_set_reg(instance, 0xEA, 0x73);
    bt865_set_reg(instance, 0xEC, 0xa8);
    bt865_set_reg(instance, 0xEE, 0x62);
    bt865_set_reg(instance, 0xF0, 0x55);
    bt865_set_reg(instance, 0xF2, 0xa4);
    bt865_set_reg(instance, 0xF4, 0x5);
    bt865_set_reg(instance, 0xF6, 0x55);
    bt865_set_reg(instance, 0xF8, 0x27);
    bt865_set_reg(instance, 0xFA, 0x40);
    bt865_set_reg(instance, 0xD8, 0x36);
    break;
    
  default:
    
    return(-EINVAL);
  }
  
  // OK
  return(0);
}
