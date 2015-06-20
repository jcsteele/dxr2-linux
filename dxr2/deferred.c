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



#include <linux/errno.h>
#include <dxr2.h>
#include <zivaDS.h>
#include <pcm1723.h>

/**
 *
 * Queues a deferred command for executing whilst a data transfer is NOT in progress...
 * interrupts for the handler which executes this queue should be disabled before entering...
 *
 * @param instance DXR2 instance
 * @param code Command code to queue (one of DXR2_QUEUE_*)
 * @param arg0 arg0 for the above
 * @param arg1 arg1 for the above
 * @param arg2 arg2 for the above
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_queue_deferred(dxr2_t* instance, int code, int arg0, int arg1, int arg2)
{

  // check we've not run out of space
  if (instance->deferredCount > DXR2_MAX_DEFERRED_COMMANDS) {
    
    return(-ENOMEM);
  }
  
  // store the data
  instance->deferredQueue[instance->deferredCount][0] = code;
  instance->deferredQueue[instance->deferredCount][1] = arg0;
  instance->deferredQueue[instance->deferredCount][2] = arg1;
  instance->deferredQueue[instance->deferredCount][3] = arg2;
  
  // increment counter
  instance->deferredCount++;

  // changed by zulli
  return(0);
}


/**
 *
 * Executes the deferred command queue
 * Should be called by an interrupt handler.
 *
 * @param instance DXR2 instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int dxr2_process_deferred_queue(dxr2_t* instance)
{
  int i;

  // OK, process each element of the queue
  for(i=0; i< instance->deferredCount; i++) {
    
    switch(instance->deferredQueue[i][0]) {
    case DXR2_QUEUE_PAUSED:
      zivaDS_pause(instance->zivaDSInstance);
      break;
      
    case DXR2_QUEUE_SETVOLUME:
      zivaDS_set_audio_volume(instance->zivaDSInstance, instance->deferredQueue[i][1]);
      break;

    case DXR2_QUEUE_ENABLESUBPICTURE:
      zivaDS_enable_subpicture(instance->zivaDSInstance, instance->deferredQueue[i][1]);
      break;

    case DXR2_QUEUE_FASTFORWARDS:
      zivaDS_fast_forwards(instance->zivaDSInstance, instance->deferredQueue[i][1]);
      break;

    case DXR2_QUEUE_FASTBACKWARDS:
      zivaDS_fast_backwards(instance->zivaDSInstance, instance->deferredQueue[i][1]);
      break;

    case DXR2_QUEUE_SELECTSTREAM:
      zivaDS_select_stream(instance->zivaDSInstance, instance->deferredQueue[i][1], instance->deferredQueue[i][2]);
      break;

    case DXR2_QUEUE_SETMUTESTATUS:
      pcm1723_set_mute_mode(instance->pcm1723Instance, instance->deferredQueue[i][1]);
      break;

    case DXR2_QUEUE_HIGHLIGHT:
      zivaDS_highlight(instance->zivaDSInstance, instance->deferredQueue[i][1], instance->deferredQueue[i][2]);
      break;
    }
  }
   
  // OK, queue is done
  instance->deferredCount = 0;

  // OK!
  return(0);
}
