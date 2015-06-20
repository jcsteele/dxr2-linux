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
 * Driver for the Toshiba TC6807AF CSS decode chip
 * CSS functions
 *
 */


#include <dxr2modver.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <tc6807af.h>


static int tc6807af_css_delay(tc6807af_t* instance, int value, int timeout);




/**
 *
 * Set tc6807af decryption mode
 *
 * @param instance The instance of TC6807AF to use
 * @param mode CSS mode one of TC6807AF_CSSDECRMODE_OFF,TC6807AF_CSSDECRMODE_ON
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_set_decryption_mode(tc6807af_t* instance, int mode)
{
  // different things depending on mode
  switch(mode) {
  case TC6807AF_CSSDECRMODE_OFF:
    
    tc6807af_set_loc(instance, TC6807AF_LOC2, 0xE7);
    break;

  case TC6807AF_CSSDECRMODE_ON:
    
    tc6807af_set_loc(instance, TC6807AF_LOC1, 3);
    tc6807af_set_loc(instance, TC6807AF_LOC2, 0xE6);
    tc6807af_set_loc(instance, TC6807AF_LOC0, 0);
    tc6807af_set_loc(instance, TC6807AF_LOC0, 0x23);
    break;

  default:
    return(-EINVAL);
  }
  
  // OK
  return(0);
}



/**
 *
 * Send challenge key to TC6807AF
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array containing the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_challenge_key(tc6807af_t* instance, unsigned char* key)
{
  // send it
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY, key[0] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+1, key[1] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+2, key[2] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+3, key[3] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+4, key[4] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+5, key[5] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+6, key[6] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+7, key[7] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+8, key[8] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCCHALLENGEKEY+9, key[9] & 0xff);

  // delay
  return(tc6807af_css_delay(instance, 0x18, 3));
}


/**
 *
 * Get challenge key from tc6807af
 *
 * @param instance The instance of TC6807AF to use
 * @param key 10 byte char array to recieve the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_get_challenge_key(tc6807af_t* instance, unsigned char* key)
{
  int status;
  
  // delay
  if ((status = tc6807af_css_delay(instance, 0x12, 3)) < 0) {
    
    return(status);
  }

  // send it
  key[0] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY) & 0xff;
  key[1] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+1) & 0xff;
  key[2] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+2) & 0xff;
  key[3] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+3) & 0xff;
  key[4] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+4) & 0xff;
  key[5] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+5) & 0xff;
  key[6] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+6) & 0xff;
  key[7] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+7) & 0xff;
  key[8] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+8) & 0xff;
  key[9] = tc6807af_get_loc(instance, TC6807AF_LOCCHALLENGEKEY+9) & 0xff;

  // ok
  return(0);
}


/**
 *
 * Send the response key to the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_response_key(tc6807af_t* instance, unsigned char* key)
{
  // send it
  tc6807af_set_loc(instance, TC6807AF_LOCRESPONSEKEY, key[0] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCRESPONSEKEY+1, key[1] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCRESPONSEKEY+2, key[2] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCRESPONSEKEY+3, key[3] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCRESPONSEKEY+4, key[4] & 0xff);

  // delay
  return(tc6807af_css_delay(instance, 0x17, 3));
}



/**
 *
 * Get the response key from the TC6807AF
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 5 byte char array to receive the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_get_response_key(tc6807af_t* instance, unsigned char* key)
{
  // get it
  key[0] = tc6807af_get_loc(instance, TC6807AF_LOCRESPONSEKEY) & 0xff;
  key[1] = tc6807af_get_loc(instance, TC6807AF_LOCRESPONSEKEY+1) & 0xff;
  key[2] = tc6807af_get_loc(instance, TC6807AF_LOCRESPONSEKEY+2) & 0xff;
  key[3] = tc6807af_get_loc(instance, TC6807AF_LOCRESPONSEKEY+3) & 0xff;
  key[4] = tc6807af_get_loc(instance, TC6807AF_LOCRESPONSEKEY+4) & 0xff;

  return(0);
}



/**
 *
 * Part1 of The TC6807AF specific part of the send disc key protocol. 
 * Since the disc key os 0x800 bytes, you usually DMA it to the chip somehow...
 *
 * @param instance Instance of the TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_disc_key_part1(tc6807af_t* instance)
{
  // send it
  tc6807af_set_loc(instance, TC6807AF_LOC1, 1);
  tc6807af_set_loc(instance, TC6807AF_LOC2, 0x24);
  tc6807af_set_loc(instance, TC6807AF_LOC0, 0x15);
  tc6807af_set_loc(instance, TC6807AF_LOC1, 3);
  tc6807af_set_loc(instance, TC6807AF_LOC2, 0x26);
}


/**
 *
 * Part2 of The TC6807AF specific part of the send disc key protocol. 
 * This bit waits until (I assume) the TC6807AF has finished processing the key
 *
 * @param instance Instance of the TC6807AF to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_disc_key_part2(tc6807af_t* instance)
{
  // wait for processing
  return(tc6807af_css_delay(instance, -1, 100));
}


/**
 *
 * Send the disc title key
 *
 * @param instance Instance of the TC6807AF to use
 * @param key 6 byte char array containing the key to send (incl CGMS flags in byte 0)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int tc6807af_send_title_key(tc6807af_t* instance, unsigned char* key)
{
  // send it
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY, key[0] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY+1, key[1] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY+2, key[2] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY+3, key[3] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY+4, key[4] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOCTITLEKEY+5, key[5] & 0xff);
  tc6807af_set_loc(instance, TC6807AF_LOC0, 0);

  // wait for processing
  return(tc6807af_css_delay(instance, 0x25, 20));
}


/**
 *
 * Sets register 0 to supplied value, and waits (max 30,000 usecs)
 * until bit 7 and bit 6 of register 0 become unset. 
 * This is used by the various CSS key functions to wait until processing
 * has been completed (or has failed.. whichever happens first)
 *
 * @param instance tc6807af instance to use
 * @param value Value to write to register 0 (-1 => don't write anything)
 * @param timeout Timeout for operation in centiseconds (e.g. 3 = 0.03secs)
 *
 * @returns 0 on success, -ETIMEDOUT on timeout, or error
 *
 */

static int tc6807af_css_delay(tc6807af_t* instance, int value, int timeout) 
{
  int endTime;
  int tmp;


  // write value
  if (value != -1) {
    
    tc6807af_set_loc(instance, 0, value);
  }

  // extra tests
  if ((value == 0) || (value == 0x12)) {
    
    return(0);
  }

  // loop for timeout centisecs (ish), or until condition met
  endTime = jiffies + ((timeout*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // get register 0
    tmp =  tc6807af_get_loc(instance, 0);

    // if bit 7 is set
    if (tmp & 0x80) {
      
      // if bit 6 is not set => OK!
      if (!(tmp & 0x40)) {
	
	return(0);
      }
      
      // Erk! bit6 wasn't set! => error
      return(-ETIMEDOUT);
    }
  }
  
  // loop must have timed out
  return(-ETIMEDOUT);
}
