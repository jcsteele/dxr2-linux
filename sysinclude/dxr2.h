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
 * Driver for the Creative DXR2 Mpeg decoder card
 *
 */


#ifndef __DXR2_H__
#define __DXR2_H__


#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/tqueue.h>
#include <linux/fs.h>
#include <linux/cdrom.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <anp82.h>
#include <bt865.h>
#include <pcm1723.h>
#include <tc6807af.h>
#include <vxp524.h>
#include <zivaDS.h>
#include <dxr2ioctl.h>



// *******************************************************************
// useful defines

// full name of the supported thing
#define DXR2_FULLNAME			"Creative DXR2 Mpeg decoder"

// log name of the driver
#define DXR2_LOGNAME			"DXR2"

// these control entry to code modifying deferred_queue stuff
#define DXR2_ENTER_CRITICAL(DXR2INST) while(!atomic_dec_and_test(&((DXR2INST)->semaphore))) { atomic_inc(&((DXR2INST)->semaphore)); schedule(); }
#define DXR2_EXIT_CRITICAL(DXR2INST) atomic_inc(&((DXR2INST)->semaphore))

// number of pages in each BM buffer (2^DXR2_PAGE_ORDER)
#define DXR2_PAGE_ORDER 3

#define DXR2_MAJOR 120

#define DXR2_MAX_DEFERRED_COMMANDS 5


// *******************************************************************
// Structures

typedef struct {

  // driver instances
  anp82_t* anp82Instance;
  bt865_t* bt865Instance;
  pcm1723_t* pcm1723Instance;
  tc6807af_t* tc6807afInstance;
  zivaDS_t* zivaDSInstance;
  vxp524_t* vxp524Instance;

  // base address values used for talking to some of the hardware
  unsigned long tc6807afBase;
  unsigned long zivaDSBase;
  unsigned long vxp524Base;
  unsigned long asicBase;

  // current value of the ASIC, since it's read only
  int asicValue;

  // BM buffer stuff
  int bmBuffer;
  int writeBuffer;
  unsigned long buffer[2];
  int bufferSize[2];
  int bufferCount[2];

  // is the HLI int enabled
  int hliFlag;

  // has the ziva been initialised
  int zivaDSInitialised;

  // semaphore so we don't start two BMs/process the deferred queue at the same time
  atomic_t semaphore;

  // wait queue
  #if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,10)
    wait_queue_head_t waitQueue;
  #else
    struct wait_queue* waitQueue;
  #endif

  
  // deferred commands
  int deferredCount;
  int deferredQueue[DXR2_MAX_DEFERRED_COMMANDS][4];

  // to hold data to transfer
  char* userBuffer;            // user supplied buffer
  int userBytesTransferred;      // number of bytes already transferred
  int userBufferSize;            // total amount of data in buffer

  // sizes of audio and video buffers
  int audioBufferSize;
  int videoBufferSize;

  // misc stuff we need to remember
  int currentZivaAudioDACMode;
  int currentSourceVideoFrequency;
  int currentSourceVideoXRes;
  int currentSourceVideoYRes;
  int currentPlayMode;
  int currentAudioVolume;
  int currentOutputAspectRatio;
  int currentSlowRate;
  int currentAspectRatioMode;
  int currentBitstreamType;
  int currentVideoStream;
  int currentSubPictureStream;
  int currentAudioStream;
  int currentAudioStreamType;
  int currentAudioMuteStatus;
  int currentAudioSampleFrequency;
  int currentAudioInputWidth;

} dxr2_t;


// *******************************************************************
// function declarations


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions


/**
 *
 * Create new DXR2 device
 *
 */

extern dxr2_t* dxr2_new();
		     


/**
 *
 * Destroy a dxr2 device (BURNY BURNY!!!!)
 *
 * @param instance DXR2 instance to use
 *
 */

extern void dxr2_free(dxr2_t* instance);


#endif

