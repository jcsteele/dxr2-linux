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
 * Driver for the Auravision VxP524 Video processor chip
 * Low level functions
 *
 */


#include <dxr2modver.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <linux/types.h>
#include <vxp524.h>



/**
 *
 * Enable memory mapped access & IRQ for the VxP524
 *
 * @param instance instance to do this for
 * 
 */

extern void vxp524_enable_mem(vxp524_t* instance)
{
  u16 word;

  pci_read_config_word (instance->pci_dev, PCI_COMMAND, &word);
  word |= PCI_COMMAND_MEMORY;
  pci_write_config_word (instance->pci_dev, PCI_COMMAND, word);
  pci_set_master (instance->pci_dev);
}


/**
 *
 * Disable memory mapped access & IRQ for the VxP524
 *
 * @param instance instance to do this for
 * 
 */

extern void vxp524_disable_mem(vxp524_t* instance)
{
  u16 word;
  
  pci_read_config_word (instance->pci_dev, PCI_COMMAND, &word);
  word &= ~(PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
  pci_write_config_word(instance->pci_dev, PCI_COMMAND, word);
}




/**
 *
 * Get (8bit) register from the Vxp524
 * This retrives the value: [reg] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg(vxp524_t* instance, int reg)
{
  return(readb(instance->base + (reg << 2)));
}


/**
 *
 * Set (8bit) register on the Vxp524
 * This sets the value: [reg] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg(vxp524_t* instance, int reg, int val)
{
  writeb(val, instance->base + (reg << 2));
}


/**
 *
 * Get (16bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg16(vxp524_t* instance, int reg)
{
  register u32 value;

  value  =  readb(instance->base + (reg << 2)); reg++;
  value |= (readb(instance->base + (reg << 2)) << 8);

  return(value);
}


/**
 *
 * Set (16bit) register on the Vxp524
 * This sets the value: [reg], [reg+1] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg16(vxp524_t* instance, int reg, int val)
{
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2));
}



/**
 *
 * Get (24bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) | ([reg+2]<<16) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg24(vxp524_t* instance, int reg)
{
  register u32 value;

  value  =  readb((u32) instance->base + (reg << 2));       reg++;
  value |= (readb((u32) instance->base + (reg << 2)) << 8); reg++;
  value |= (readb((u32) instance->base + (reg << 2)) << 16);
  
  return(value);
}


/**
 *
 * Set (24bit) register on the Vxp524
 * This sets the value: [reg], [reg+1], [reg+2] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg24(vxp524_t* instance, int reg, int val)
{
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2));
}



/**
 *
 * Get (32bit) register from the Vxp524
 * This retrives the value: [reg] | ([reg+1] <<8) | ([reg+2]<<16) | ([reg+3]<<24) (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @return The register's value (or negative on error)
 *
 */

extern int vxp524_get_reg32(vxp524_t* instance, int reg)
{
  register u32 value;

  value  =  readb((u32) instance->base + (reg << 2));        reg++;
  value |= (readb((u32) instance->base + (reg << 2)) << 8);  reg++;
  value |= (readb((u32) instance->base + (reg << 2)) << 16); reg++;
  value |= (readb((u32) instance->base + (reg << 2)) << 24);
  
  return(value);
}


/**
 *
 * Set (32bit) register on the Vxp524
 * This sets the value: [reg], [reg+1], [reg+2], [reg+3] (LSB first)
 *
 * @param instance Instance of the VxP524 to use
 * @param reg Register to retrieve
 * @param val 8 bit value to set
 *
 */

extern void vxp524_set_reg32(vxp524_t* instance, int reg, int val)
{
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2)); val >>=8; reg++;
  writeb(val, (u32) instance->base + (reg << 2));
}



/**
 *
 * Get specified bitmask of an (8bit) register from vxp524
 *
 * @param instance Instance of the vxp524 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues
 *
 */

extern int vxp524_get_bits(vxp524_t* instance, int reg, int bitmask)
{
  return (vxp524_get_reg (instance, reg) & bitmask);
}



/**
 *
 * Set specified bits of an (8bit) register on vxp524
 *
 * @param instance Instance of the vxp524 to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void vxp524_set_bits(vxp524_t* instance, int reg, int bitmask, int valuemask)
{

  // get the current register value
  int value = vxp524_get_reg(instance, reg);
  
  // set it on the hardware
  vxp524_set_reg(instance, reg, (value & (~bitmask)) | valuemask);
}






