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

#include <linux/ioctl.h>
#ifdef NEED_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <linux/cdrom.h>


#ifndef __DXR2IOCTL_H__
#define __DXR2IOCTL_H__


// *******************************************************************
// IOCTL codes (this is temporary)

#define DXR2_IOC_MAGIC 'X'

#define DXR2_IOC_GET_REGION_CODE		_IO(DXR2_IOC_MAGIC, 0)
#define DXR2_IOC_SET_TV_OUTPUT_FORMAT		_IO(DXR2_IOC_MAGIC, 1)
#define DXR2_IOC_SET_SOURCE_VIDEO_FORMAT 	_IO(DXR2_IOC_MAGIC, 2)
#define DXR2_IOC_GET_CAPABILITIES		_IO(DXR2_IOC_MAGIC, 3)
#define DXR2_IOC_CLEAR_VIDEO			_IO(DXR2_IOC_MAGIC, 4)
#define DXR2_IOC_PAUSE				_IO(DXR2_IOC_MAGIC, 5)
#define DXR2_IOC_SET_AUDIO_VOLUME		_IO(DXR2_IOC_MAGIC, 6)
#define DXR2_IOC_SET_OUTPUT_ASPECT_RATIO	_IO(DXR2_IOC_MAGIC, 7)
#define DXR2_IOC_ABORT				_IO(DXR2_IOC_MAGIC, 8)
#define DXR2_IOC_STOP				_IO(DXR2_IOC_MAGIC, 9)
#define DXR2_IOC_ENABLE_SUBPICTURE		_IO(DXR2_IOC_MAGIC, 10)
#define DXR2_IOC_SLOW_FORWARDS			_IO(DXR2_IOC_MAGIC, 11)
#define DXR2_IOC_SLOW_BACKWARDS			_IO(DXR2_IOC_MAGIC, 12)
#define DXR2_IOC_SET_SOURCE_ASPECT_RATIO	_IO(DXR2_IOC_MAGIC, 13)
#define DXR2_IOC_SET_ASPECT_RATIO_MODE		_IO(DXR2_IOC_MAGIC, 14)
#define DXR2_IOC_SINGLE_STEP			_IO(DXR2_IOC_MAGIC, 15)
#define DXR2_IOC_REVERSE_PLAY			_IO(DXR2_IOC_MAGIC, 16)
#define DXR2_IOC_SET_SUBPICTURE_PALETTE		_IO(DXR2_IOC_MAGIC, 17)
#define DXR2_IOC_GET_CHALLENGE_KEY		_IO(DXR2_IOC_MAGIC, 18)
#define DXR2_IOC_SEND_CHALLENGE_KEY		_IO(DXR2_IOC_MAGIC, 19)
#define DXR2_IOC_GET_RESPONSE_KEY		_IO(DXR2_IOC_MAGIC, 20)
#define DXR2_IOC_SEND_RESPONSE_KEY		_IO(DXR2_IOC_MAGIC, 21)
#define DXR2_IOC_SEND_DISC_KEY			_IO(DXR2_IOC_MAGIC, 22)
#define DXR2_IOC_SEND_TITLE_KEY			_IO(DXR2_IOC_MAGIC, 23)
#define DXR2_IOC_SET_DECRYPTION_MODE		_IO(DXR2_IOC_MAGIC, 24)
#define DXR2_IOC_INIT_ZIVADS			_IO(DXR2_IOC_MAGIC, 25)
#define DXR2_IOC_SET_TV_MACROVISION_MODE	_IO(DXR2_IOC_MAGIC, 27)
#define DXR2_IOC_RESET 				_IO(DXR2_IOC_MAGIC, 28)
#define DXR2_IOC_SET_BITSTREAM_TYPE 		_IO(DXR2_IOC_MAGIC, 29)
#define DXR2_IOC_PLAY 				_IO(DXR2_IOC_MAGIC, 30)
#define DXR2_IOC_GET_STC      			_IO(DXR2_IOC_MAGIC, 31)
#define DXR2_IOC_SET_AUDIO_SAMPLE_FREQUENCY 	_IO(DXR2_IOC_MAGIC, 32)
#define DXR2_IOC_SET_AUDIO_DATA_WIDTH		_IO(DXR2_IOC_MAGIC, 33)
#define DXR2_IOC_IEC958_OUTPUT_MODE		_IO(DXR2_IOC_MAGIC, 34)
#define DXR2_IOC_SET_AC3_MODE			_IO(DXR2_IOC_MAGIC, 35)
#define DXR2_IOC_SELECT_AC3_VOICE		_IO(DXR2_IOC_MAGIC, 36)
#define DXR2_IOC_AUDIO_MUTE			_IO(DXR2_IOC_MAGIC, 37)
#define DXR2_IOC_SET_STEREO_MODE		_IO(DXR2_IOC_MAGIC, 38)
#define DXR2_IOC_SELECT_STREAM			_IO(DXR2_IOC_MAGIC, 39)
#define DXR2_IOC_HIGHLIGHT			_IO(DXR2_IOC_MAGIC, 40)
#define DXR2_IOC_SET_TV_BLACKWHITE_MODE		_IO(DXR2_IOC_MAGIC, 41)
#define DXR2_IOC_SET_TV_INTERLACED_MODE 	_IO(DXR2_IOC_MAGIC, 42)
#define DXR2_IOC_SET_TV_75IRE_MODE      	_IO(DXR2_IOC_MAGIC, 43)
#define DXR2_IOC_SET_TV_PIXEL_MODE      	_IO(DXR2_IOC_MAGIC, 44)
#define DXR2_IOC_SET_OVERLAY_COLOUR    	        _IO(DXR2_IOC_MAGIC, 45)
#define DXR2_IOC_SET_OVERLAY_GAIN	        _IO(DXR2_IOC_MAGIC, 46)
#define DXR2_IOC_SET_OVERLAY_IN_DELAY           _IO(DXR2_IOC_MAGIC, 47)
#define DXR2_IOC_SET_OVERLAY_MODE    	        _IO(DXR2_IOC_MAGIC, 48)
#define DXR2_IOC_SET_OVERLAY_CROPPING	        _IO(DXR2_IOC_MAGIC, 49)
#define DXR2_IOC_SET_OVERLAY_DIMENSION	        _IO(DXR2_IOC_MAGIC, 50)
#define DXR2_IOC_SET_OVERLAY_POSITION	        _IO(DXR2_IOC_MAGIC, 51)
#define DXR2_IOC_SET_OVERLAY_RATIO              _IO(DXR2_IOC_MAGIC, 52)
#define DXR2_IOC_CALCULATE_VGA_PARAMETERS       _IO(DXR2_IOC_MAGIC, 53)
#define DXR2_IOC_SET_VGA_PARAMETERS             _IO(DXR2_IOC_MAGIC, 54)
#define DXR2_IOC_SET_OVERLAY_PICTURE_CONTROLS   _IO(DXR2_IOC_MAGIC, 55)
#define DXR2_IOC_FAST_FORWARDS		        _IO(DXR2_IOC_MAGIC, 56)
#define DXR2_IOC_FAST_BACKWARDS		        _IO(DXR2_IOC_MAGIC, 57)
#define DXR2_IOC_BUFFERS_EMPTY		        _IO(DXR2_IOC_MAGIC, 58)


// *******************************************************************
// stuff for IOCTLS

// video frequencies
#define DXR2_SRC_VIDEO_FREQ_30 0
#define DXR2_SRC_VIDEO_FREQ_25 1

// aspect ratios
#define DXR2_ASPECTRATIO_4_3 0
#define DXR2_ASPECTRATIO_16_9 1

// subpicture modes
#define DXR2_SUBPICTURE_OFF 0
#define DXR2_SUBPICTURE_ON  1

// rates for slow forwards & backwards
#define DXR2_PLAYRATE_2x 0
#define DXR2_PLAYRATE_3x 1
#define DXR2_PLAYRATE_4x 2
#define DXR2_PLAYRATE_5x 3
#define DXR2_PLAYRATE_6x 4

// CSS decryption modes supported
#define DXR2_CSSDECRMODE_OFF 0
#define DXR2_CSSDECRMODE_ON  1

// play modes
#define DXR2_PLAYMODE_STOPPED         0
#define DXR2_PLAYMODE_PAUSED          1
#define DXR2_PLAYMODE_SLOWFORWARDS    2
#define DXR2_PLAYMODE_SLOWBACKWARDS   3
#define DXR2_PLAYMODE_SINGLESTEP      4
#define DXR2_PLAYMODE_PLAY            5
#define DXR2_PLAYMODE_REVERSEPLAY     6
#define DXR2_PLAYMODE_FASTFORWARDS    7
#define DXR2_PLAYMODE_FASTBACKWARDS   8

// for operation queue
#define DXR2_QUEUE_PAUSED             0
#define DXR2_QUEUE_SETVOLUME          1
#define DXR2_QUEUE_ENABLESUBPICTURE   2
#define DXR2_QUEUE_FASTFORWARDS       3
#define DXR2_QUEUE_FASTBACKWARDS      4
#define DXR2_QUEUE_SELECTSTREAM       5
#define DXR2_QUEUE_SETMUTESTATUS      6
#define DXR2_QUEUE_HIGHLIGHT          7

// aspect ratio modes
#define DXR2_ASPECTRATIOMODE_NORMAL    0
#define DXR2_ASPECTRATIOMODE_PAN_SCAN  1
#define DXR2_ASPECTRATIOMODE_LETTERBOX 2

// macrovision modes
#define DXR2_MACROVISION_OFF                 0
#define DXR2_MACROVISION_AGC                 1
#define DXR2_MACROVISION_AGC_2COLOURSTRIPE   2
#define DXR2_MACROVISION_AGC_4COLOURSTRIPE   3


// TV output modes
#define DXR2_OUTPUTFORMAT_NTSC      0
#define DXR2_OUTPUTFORMAT_NTSC_60   1
#define DXR2_OUTPUTFORMAT_PAL_M     2
#define DXR2_OUTPUTFORMAT_PAL_M_60  3
#define DXR2_OUTPUTFORMAT_PAL_BDGHI 4
#define DXR2_OUTPUTFORMAT_PAL_N     5
#define DXR2_OUTPUTFORMAT_PAL_Nc    6
#define DXR2_OUTPUTFORMAT_PAL_60    7

// black/white modes
#define DXR2_BLACKWHITE_OFF   0
#define DXR2_BLACKWHITE_ON    1

// interlacing
#define DXR2_INTERLACED_OFF   0
#define DXR2_INTERLACED_ON    1

// 7.5 IRE
#define DXR2_75IRE_OFF        0
#define DXR2_75IRE_ON         1

// pixel modes
#define DXR2_PIXEL_CCIR601    0
#define DXR2_PIXEL_SQUARE     1

// bitstreams possibly present in files
#define DXR2_STREAM_VIDEO       0
#define DXR2_STREAM_SUBPICTURE  1
#define DXR2_STREAM_AUDIO_AC3   2
#define DXR2_STREAM_AUDIO_MPEG  3
#define DXR2_STREAM_AUDIO_LPCM  4
#define DXR2_STREAM_AUDIO_5 5

// bitstream types
#define DXR2_BITSTREAM_TYPE_MPEG_VOB       0
#define DXR2_BITSTREAM_TYPE_CDROM_VCD      1
#define DXR2_BITSTREAM_TYPE_MPEG_VCD       2
#define DXR2_BITSTREAM_TYPE_CDDA           3
#define DXR2_BITSTREAM_TYPE_4              4

// frequency of output audio data (to the pcm1723)
#define DXR2_AUDIO_FREQ_441    0
#define DXR2_AUDIO_FREQ_48     1
#define DXR2_AUDIO_FREQ_96     2
#define DXR2_AUDIO_FREQ_2205   3
#define DXR2_AUDIO_FREQ_32     4

// widths of output audio data (to the pcm1723)
#define DXR2_AUDIO_WIDTH_16    0
#define DXR2_AUDIO_WIDTH_20    1
#define DXR2_AUDIO_WIDTH_24    2

// play types
#define DXR2_PLAYTYPE_NORMAL 0
#define DXR2_PLAYTYPE_STILLSTOP 1

// iec-958 output types
#define DXR2_IEC958_DECODED    0
#define DXR2_IEC958_ENCODED    1

// AC3 modes
#define DXR2_AC3MODE_LR_STEREO          0
#define DXR2_AC3MODE_LR_STEREO_PROLOGIC 1
#define DXR2_AC3MODE_LR_MONOR           2

// AC3 voice configuration (for karaoke)
#define DXR2_AC3VOICE_NONE        0
#define DXR2_AC3VOICE_V1V2        1

// highlight actions
#define DXR2_BUTTONACTION_SELECT    0
#define DXR2_BUTTONACTION_UNHIGHLIGHT 1
#define DXR2_BUTTONACTION_ACTIVATE 2
#define DXR2_BUTTONACTION_ACTIVATE_SELECTED 3
#define DXR2_BUTTONACTION_4 4
#define DXR2_BUTTONACTION_5 5
#define DXR2_BUTTONACTION_6 6
#define DXR2_BUTTONACTION_7 7
#define DXR2_BUTTONACTION_8 8

// special buttons
#define DXR2_BUTTON_NONE    0
#define DXR2_BUTTON_UP     64
#define DXR2_BUTTON_DOWN   65
#define DXR2_BUTTON_LEFT   66
#define DXR2_BUTTON_RIGHT  67

// mute modes
#define DXR2_AUDIO_MUTE_ON     0
#define DXR2_AUDIO_MUTE_OFF    1

// stereo mode
#define DXR2_AUDIO_STEREO_NORMAL  0
#define DXR2_AUDIO_STEREO_MONOL   1
#define DXR2_AUDIO_STEREO_MONOR   2
#define DXR2_AUDIO_STEREO_REVERSE 3

// overlay modes
#define DXR2_OVERLAY_DISABLED           0
#define DXR2_OVERLAY_WINDOW_KEY         1
#define DXR2_OVERLAY_COLOUR_KEY         2
#define DXR2_OVERLAY_WINDOW_COLOUR_KEY  3


// portion of the sync signal to measure
#define DXR2_MEASURE_PORTION_LOW       0
#define DXR2_MEASURE_PORTION_HIGH      1
#define DXR2_MEASURE_PORTION_ALL       2

// which signal to measure
#define DXR2_TIME_HSYNC                0
#define DXR2_TIME_VSYNC                1
#define DXR2_TIME_HCOLOURKEY           2
#define DXR2_TIME_VCOLOURKEY           3



// *******************************************************************
// Structures


typedef struct {

  int arg;

} dxr2_oneArg_t;


typedef struct {

  int arg1;
  int arg2;

} dxr2_twoArg_t;

typedef struct {

  int arg1;
  int arg2;
  int arg3;

} dxr2_threeArg_t;

typedef struct {

  int arg1;
  int arg2;
  int arg3;
  int arg4;

} dxr2_fourArg_t;

typedef struct {

  int arg1;
  int arg2;
  int arg3;
  int arg4;
  int arg5;
  int arg6;

} dxr2_sixArg_t;


typedef struct {

  int arg1;
  int arg2;
  int arg3;
  int arg4;
  int arg5;
  int arg6;
  int arg7;
  int arg8;
  int arg9;

} dxr2_nineArg_t;

typedef struct {

  int uCodeLength;
  char uCode[0]; // allocate this structure to whatever length you need...

} dxr2_uCode_t;

typedef struct {

  // offsets of window key being measured
  int hOffWinKey;
  int vOffWinKey;

  // screen dimensions
  int xScreen;
  int yScreen;

  // sync polarites
  int hsyncPol;
  int vsyncPol;

  // video blanking 
  int blankStart;
  int blankWidth;
  
  // screen offsets
  int hOffset;
  int vOffset;
  
  // ratio
  int ratio;
  
} dxr2_vgaParams_t;

typedef struct {

  int entries[16];

} dxr2_palette_t;

typedef struct {
  
  unsigned char key[10];

} dxr2_challengeKey_t;


typedef struct {
  
  unsigned char key[5];

} dxr2_responseKey_t;


typedef struct {
  
  unsigned char key[0x800];

} dxr2_discKey_t;


typedef struct {
  
  unsigned char cgmsFlags;
  unsigned char key[5];

} dxr2_titleKey_t;

#endif

