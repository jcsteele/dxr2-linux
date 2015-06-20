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
 * Driver for the C-Cube Ziva-DS MPEG decoder chip
 * CSS functions.
 *
 */

#include <dxr2modver.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <zivaDS.h>


static int zivaDS_css_delay(zivaDS_t* instance, int cssStart, int command, int timeout, 
			    int test);

/**
 *
 * Sets the CSS decryption mode
 *
 * @param instance Ziva instance to use
 * @param mode CSS mode one of ZIVADS_CSSDECRMODE_OFF, ZIVADS_CSSDECRMODE_ON
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_decryption_mode(zivaDS_t* instance, int mode)
{
  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);
  int command;
  int tmp;
  int status;

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // ziva command (for some reason...)
  if ((status = zivaDS_command(instance, 0x137,1,0,0,0,0,0,0,3)) < 0) {
    
    return(status);
  }

  // set the mode
  switch(mode) {
  case ZIVADS_CSSDECRMODE_OFF:
    
    command = 8;
    break;

  case ZIVADS_CSSDECRMODE_ON:

    command = 7;
    break;

  default:

    return(-EINVAL);
    break;
  }

  // wait until processing finished
  if ((status = zivaDS_css_delay(instance, cssStart, command, 30, 1)) <0) {
    
    return(status);
  }

  // last bit
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, 0);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);
  tmp = zivaDS_get_mem(instance, ZIVADS_CSS_UNKNOWN0) & 0xfffffffb;
  if (mode == ZIVADS_CSSDECRMODE_OFF) {
    
    tmp |= 0x4;
  }
  zivaDS_set_mem(instance, ZIVADS_CSS_UNKNOWN0, tmp);

  // OK
  return(0);
}



/**
 *
 * Send the challenge key to the Ziva
 *
 * @param instance Ziva instance to use
 * @param key 10 byte char array containing the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_send_challenge_key(zivaDS_t* instance, unsigned char* key)
{
  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);
  
  // check CSS status
  if (zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // send it
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(0*4), key[0] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(1*4), key[1] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(2*4), key[2] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(3*4), key[3] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(4*4), key[4] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(5*4), key[5] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(6*4), key[6] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(7*4), key[7] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(8*4), key[8] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDCHALLENGEKEY-(9*4), key[9] & 0xff);
  
  // wait until processing finished
  return(zivaDS_css_delay(instance, cssStart, 3, 30, 1));
}


/**
 *
 * Get the challenge key from the Ziva
 *
 * @param instance Instance of the Ziva to use
 * @param key 10 byte char array to put the key in
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_get_challenge_key(zivaDS_t* instance, unsigned char* key)
{
  int status;

  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart + ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // do it
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, 1);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);
  if ((status = zivaDS_command(instance, 0x137,1,0,0,0,0,0,0,3)) < 0) {
    
    return(status);
  }

  // delay
  if ((status = zivaDS_css_delay(instance, cssStart, -1, 30, 1)) < 0) {
    
    return(status);
  }

  // get it
  key[0] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY) & 0xff;
  key[1] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(1*4)) & 0xff;
  key[2] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(2*4)) & 0xff;
  key[3] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(3*4)) & 0xff;
  key[4] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(4*4)) & 0xff;
  key[5] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(5*4)) & 0xff;
  key[6] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(6*4)) & 0xff;
  key[7] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(7*4)) & 0xff;
  key[8] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(8*4)) & 0xff;
  key[9] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETCHALLENGEKEY-(9*4)) & 0xff;

  // ok
  return(0);
}



/**
 *
 * Send the response key to the Ziva
 *
 * @param instance Instance of the Ziva to use
 * @param key 5 byte char array containing the key
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_send_response_key(zivaDS_t* instance, unsigned char* key)
{
  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart + ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }
  
  // send it
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDRESPONSEKEY, key[0] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDRESPONSEKEY-(1*4), key[1] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDRESPONSEKEY-(2*4), key[2] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDRESPONSEKEY-(3*4), key[3] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDRESPONSEKEY-(4*4), key[4] & 0xff);

  // wait until processing finished
  return(zivaDS_css_delay(instance, cssStart, 2, 30, 1));
}



/**
 *
 * Get the response key from the Ziva
 *
 * @param instance Instance of the Ziva to use
 * @param key 5 byte char array to put the key in
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_get_response_key(zivaDS_t* instance, unsigned char* key)
{
  int status;

  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // wait for processing
  if ((status = zivaDS_css_delay(instance, cssStart, 4, 30, 1)) <0) {
    
    return(status);
  }
  
  // get it
  key[0] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETRESPONSEKEY) & 0xff;
  key[1] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETRESPONSEKEY-(1*4)) & 0xff;
  key[2] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETRESPONSEKEY-(2*4)) & 0xff;
  key[3] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETRESPONSEKEY-(3*4)) & 0xff;
  key[4] = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_GETRESPONSEKEY-(4*4)) & 0xff;

  // done
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, 0);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);

  // OK
  return(0);
}



/**
 *
 * Part 1 of send disc key (setup)
 *
 * @param instance Instance of the Ziva to use
 *
 * @return 0 on success, or <0 on failure
 *
 */

extern int zivaDS_send_disc_key_part1(zivaDS_t* instance)
{
  int status;

  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // do command
  if ((status = zivaDS_command(instance, 0x137,1,0,0,0,0,0,0,3)) < 0) {
    
    return(status);
  }

  // wait for processing       
  return(zivaDS_css_delay(instance, cssStart, 5, 30, 3));
}


/**
 *
 * Part 2 of send disc key (finalise)
 *
 * @param instance Instance of the Ziva to use
 *
 * @return 0 on success, or <0 on failure
 *
 */

extern int zivaDS_send_disc_key_part2(zivaDS_t* instance)
{
  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // end
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, 0);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);
  
  // OK
  return(0);
}



/**
 *
 * Send the disc title key
 *
 * @param instance Instance of the Ziva to use
 * @param key 5 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_send_title_key(zivaDS_t* instance, unsigned char* key)
{
  int status;

  // get start of CSS data in ziva
  int cssStart = zivaDS_get_mem(instance, ZIVADS_CSS_START);

  // check CSS status
  if (zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS) != 1) {
    
    return(-EBUSY);
  }

  // command
  if ((status = zivaDS_command(instance, 0x137,1,0,0,0,0,0,0,3)) < 0) {
    
    return(status);
  }

  // send it
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDTITLEKEY-(0*4), key[0] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDTITLEKEY-(1*4), key[1] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDTITLEKEY-(2*4), key[2] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDTITLEKEY-(3*4), key[3] & 0xff);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_SENDTITLEKEY-(4*4), key[4] & 0xff);

  // wait for processing	
  if ((status = zivaDS_css_delay(instance, cssStart, 6, 30, 1)) < 0) {
    
    return(status);
  }
  
  // end
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, 0);
  zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);

  // OK
  return(0);
}


/**
 *
 * Sets write command to the ziva, and waits (max timeout usecs)
 * until ziva status becomes 1 (or timeout)
 * This is used by the various CSS key functions to wait until processing
 * has been completed (or has failed.. whichever happens first)
 *
 * @param instance zivaDS instance to use
 * @param cssStart Offset into the Ziva memory of the CSS data
 * @param command Command to write
 * @param timeout Timeout for operation in centiseconds (e.g. 3 = 0.03secs)
 * @param test Value to test status against
 *
 * @returns 0 on success, -ETIMEDOUT on timeout
 *
 */

static int zivaDS_css_delay(zivaDS_t* instance, int cssStart, int command, int timeout, 
			    int test) 
{
  int endTime;
  int tmp;

  // write command
  if (command != -1) {

    zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_COMMAND, command);
    zivaDS_set_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS, 0);
  }

  // loop for timeout centisecs (ish), or until condition met
  endTime = jiffies + ((timeout*HZ)/100);
  while(jiffies < endTime) {
    
    // let other things in
    schedule();

    // get value
    tmp = zivaDS_get_mem(instance, cssStart+ZIVADS_CSSOFFSET_STATUS);

    // if test met, exit
    if (tmp == test) {
      
      return(0);
    }
  }
  
  // loop must have timed out
  return(-ETIMEDOUT);
}
