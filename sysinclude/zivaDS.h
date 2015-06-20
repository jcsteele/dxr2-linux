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
 *
 */


#ifndef __ZIVADS_H__
#define __ZIVADS_H__

#include <linux/types.h>

// *******************************************************************
// useful defines

// Full name of the chip
#define ZIVADS_FULLNAME	    	        "C-Cube Ziva-DS Mpeg Decoder"

// Log name of the driver
#define ZIVADS_LOGNAME			"ZIVADS"



// *******************************************************************
// DRAM location names

#define ZIVADS_AC3_ACMOD_CMIXLEV	0x418
#define ZIVADS_AC3_BSI_FRAME		0x40c
#define ZIVADS_AC3_BSI_IS_BEING_READ	0x404
#define ZIVADS_AC3_BSI_VALID		0x408
#define ZIVADS_AC3_BSID_BSMOD		0x414
#define ZIVADS_AC3_C_LEVEL		0x130
#define ZIVADS_AC3_CENTER_DELAY		0x144
#define ZIVADS_AC3_COMPR_LANGCOD	0x424
#define ZIVADS_AC3_DIALNORM_COMPR2	0x42c
#define ZIVADS_AC3_FRAME_NUMBER		0x400
#define ZIVADS_AC3_FSCOD_FRMSIZECOD	0x410
#define ZIVADS_AC3_HIGH_CUT		0x11c
#define ZIVADS_AC3_L_LEVEL		0x12c
#define ZIVADS_AC3_LANGCOD2_MIXLEV2	0x430
#define ZIVADS_AC3_LFE_OUTPUT_ENABLE	0x124
#define ZIVADS_AC3_LFEON_DIALNORM	0x420
#define ZIVADS_AC3_LOW_BOOST		0x118
#define ZIVADS_AC3_MIXLEV_ROOMTYP	0x428
#define ZIVADS_AC3_OPERATIONAL_MODE	0x114
#define ZIVADS_AC3_ORIGBS_TIMECOD1	0x438
#define ZIVADS_AC3_OUTPUT_MODE		0x110
#define ZIVADS_AC3_PCM_SCALE_FACTOR	0x120
#define ZIVADS_AC3_R_LEVEL		0x134
#define ZIVADS_AC3_ROOMTYP2_COPYRIGHTB	0x434
#define ZIVADS_AC3_SL_LEVEL		0x138
#define ZIVADS_AC3_SR_LEVEL		0x13c
#define ZIVADS_AC3_SURMIXLEV_DSURMOD	0x41c
#define ZIVADS_AC3_SURROUND_DELAY	0x148
#define ZIVADS_AC3_TIMECOD2		0x43c
#define ZIVADS_AC3_VOICE_SELECT		0x128
#define ZIVADS_AEE_INT_SRC		0x2c0
#define ZIVADS_AOR_INT_SRC		0x2bc
#define ZIVADS_ASPECT_RATIO		0x3b8
#define ZIVADS_ASPECT_RATIO_MODE	0x84
#define ZIVADS_AUDIO_ATTENUATION	0xf4
#define ZIVADS_AUDIO_CLOCK_SELECTION	0xec
#define ZIVADS_AUDIO_CONFIG		0xe0
#define ZIVADS_AUDIO_DAC_MODE		0xe8
#define ZIVADS_AUDIO_EMPTINESS		0x2cc
#define ZIVADS_AUDIO_TYPE		0x3f0
#define ZIVADS_AUTOPAAUSE_ENABLE	0x1d4
#define ZIVADS_AV_SYNC_MODE		0x1b0
#define ZIVADS_BACKGROUND_COLOR		0x9c
#define ZIVADS_BIT_RATE			0x3c0
#define ZIVADS_BITSTREAM_SOURCE		0x1a4
#define ZIVADS_BITSTREAM_TYPE		0x1a0
#define ZIVADS_BORDER_COLOR		0x98
#define ZIVADS_BUFF_INT_SRC		0x2b4
#define ZIVADS_CD_MODE			0x1ac
#define ZIVADS_COMMAND			0x40
#define ZIVADS_CURR_PIC_DISPLAYED	0x2d0
#define ZIVADS_DATE_TIME		0x324
#define ZIVADS_DISP_SIZE_H_V		0x3cc
#define ZIVADS_DISPLAY_ASPECT_RATIO	0x80
#define ZIVADS_DRAM_INFO		0x68
#define ZIVADS_DSI_BUFFER_END		0x22c
#define ZIVADS_DSI_BUFFER_START		0x228
#define ZIVADS_DUMP_DATA_BUFFER_END	0x284
#define ZIVADS_DUMP_DATA_BUFFER_START	0x280
#define ZIVADS_ERR_ASPECT_RATIO_INFO	0xc0
#define ZIVADS_ERR_CONCEALMENT_LEVEL	0xb4
#define ZIVADS_ERR_FRAME_RATE_CODE	0xc4
#define ZIVADS_ERR_HORIZONTAL_SIZE	0xb8
#define ZIVADS_ERR_INT_SRC		0x2c4
#define ZIVADS_ERR_VERTICAL_SIZE	0xbc
#define ZIVADS_EXTENDED_VERSION		0x334
#define ZIVADS_FRAME_RATE		0x3bc
#define ZIVADS_GOP_FLAGS		0x3d4
#define ZIVADS_H_SIZE			0x3b0
#define ZIVADS_HLI_INT_SRC		0x2b0
#define ZIVADS_IC_TYPE			0xb0
#define ZIVADS_IEC_958_DELAY		0xf0
#define ZIVADS_INT_MASK			0x200
#define ZIVADS_INT_STATUS		0x2ac
#define ZIVADS_LPCM_AUDIO_DYN_RANGE_CTRL 0x418
#define ZIVADS_LPCM_AUDIO_EMPHASIS_FLAG	0x400
#define ZIVADS_LPCM_AUDIO_FRAME_NUMBER	0x408
#define ZIVADS_LPCM_AUDIO_MUTE_FLAG	0x404
#define ZIVADS_LPCM_AUDIO_NUM_AUDIO_CHANS 0x414
#define ZIVADS_LPCM_AUDIO_SAMPLING_FREQ	0x410
#define ZIVADS_LPCM_AUDIO_QUANT_WORD_LEN 0x40c
#define ZIVADS_MEMORY_MAP		0x21c
#define ZIVADS_MPEG_AUDIO_HEADER1	0x400
#define ZIVADS_MPEG_AUDIO_HEADER2	0x404
#define ZIVADS_MRC_ID			0x2a4
#define ZIVADS_MRC_PIC_PTS		0x2f0
#define ZIVADS_MRC_PIC_STC		0x2f4
#define ZIVADS_MRC_STATUS		0x2a8
#define ZIVADS_N_AUD_DECODED		0x2f8
#define ZIVADS_N_AUD_ERRORS		0x320
#define ZIVADS_N_SYS_ERRORS		0x318
#define ZIVADS_N_VID_ERRORS		0x31c
#define ZIVADS_NEW_AUDIO_CONFIG		0x468
#define ZIVADS_NEW_AUDIO_MODE		0x460
#define ZIVADS_NEW_SUBPICTURE_PALETTE	0x464
#define ZIVADS_NEXT_PIC_DISPLAYED	0x2d4
#define ZIVADS_NEXT_SECTOR_ADDR		0x314
#define ZIVADS_NUM_DECODED		0x2e4
#define ZIVADS_NUM_REPEATED		0x2ec
#define ZIVADS_NUM_SKIPPED		0x2e8
#define ZIVADS_OSD_BUFFER_END		0x244
#define ZIVADS_OSD_BUFFER_IDLE_START	0x248
#define ZIVADS_OSD_BUFFER_START		0x240
#define ZIVADS_OSD_EVEN_FIELD		0xa0
#define ZIVADS_OSD_ODD_FIELD		0xa4
#define ZIVADS_OSD_VALID		0x2e0
#define ZIVADS_PACKET_LEN		0x3a4
#define ZIVADS_PAN_SCAN_HORIZONTAL_OFFSET 0x8c
#define ZIVADS_PAN_SCAN_SOURCE		0x88
#define ZIVADS_PARAMETER_1		0x44
#define ZIVADS_PARAMETER_2		0x48
#define ZIVADS_PARAMETER_3		0x4c
#define ZIVADS_PARAMETER_4		0x50
#define ZIVADS_PARAMETER_5		0x54
#define ZIVADS_PARAMETER_6		0x58
#define ZIVADS_PCI_BUFFER_END		0x224
#define ZIVADS_PCI_BUFFER_START		0x220
#define ZIVADS_PES_HEADER		0x3a8
#define ZIVADS_PIC_HEADER		0x3e4
#define ZIVADS_PIC_TYPE			0x3dc
#define ZIVADS_PIC1_PAN_SCAN		0x348
#define ZIVADS_PIC1_PICTURE_START	0x340
#define ZIVADS_PIC1_PTS			0x344
#define ZIVADS_PIC1_TREF_PTYPE_FLGS	0x358
#define ZIVADS_PIC1_USER_DATA		0x34c
#define ZIVADS_PIC2_PAN_SCAN		0x368
#define ZIVADS_PIC2_PICTURE_BUFFER_START 0x360
#define ZIVADS_PIC2_PTS			0x364
#define ZIVADS_PIC2_TREF_PTYP_FLGS	0x378
#define ZIVADS_PIC2_USER_DATA		0x36c
#define ZIVADS_PIC3_BUFFER_START	0x380
#define ZIVADS_PIC3_PAN_SCAN		0x388
#define ZIVADS_PIC3_PTS			0x384
#define ZIVADS_PIC3_TREF_PTYP_FLGS	0x398
#define ZIVADS_PIC3_USER_DATA		0x38c
#define ZIVADS_PROC_STATE		0x2a0
#define ZIVADS_RDY_S_THRESHOLD_LOW	0x208
#define ZIVADS_ROM_INFO			0x60
#define ZIVADS_SD_MODE			0x1a8
#define ZIVADS_SE_STATUS		0x444
#define ZIVADS_SEQ_FLAGS		0x3c8
#define ZIVADS_STATUS_ADDRESS		0x5c
#define ZIVADS_STREAM_ID		0x3a0
#define ZIVADS_SUBPICTURE_PALETTE_END	0x28c
#define ZIVADS_SUBPICTURE_PALETTE_START	0x288
#define ZIVADS_TEMP_REF			0x2d8
#define ZIVADS_TIME_CODE		0x3d0
#define ZIVADS_TOP_BORDER		0x94
#define ZIVADS_UCODE_MEMORY		0x6c
#define ZIVADS_UND_INT_SRC		0x2b8
#define ZIVADS_USER_DATA_BUFFER_END	0x274
#define ZIVADS_USER_DATA_BUFFER_START	0x270
#define ZIVADS_USER_DATA_READ		0x278
#define ZIVADS_USER_DATA_WRITE		0x27c
#define ZIVADS_V_SIZE			  0x3b4
#define ZIVADS_VBV_DELAY		  0x3e0
#define ZIVADS_SIZE			  0x3c4
#define ZIVADS_VERSION		  	  0x330
#define ZIVADS_VIDEO_EMPTINESS		  0x2c8
#define ZIVADS_VIDEO_FIELD		  0x2d8
#define ZIVADS_VIDEO_MODE		  0x7c
#define ZIVADS_VIDEO_PTS_REPEAT_INTERVAL  0x1bc
#define ZIVADS_VIDEO_PTS_SKIP_INTERVAL	  0x1b8
#define ZIVADS_CSS_START                  0x480
#define ZIVADS_CSS_UNKNOWN0               0xAC
#define ZIVADS_ENABLE_SUBPICTURE	  0x474
#define ZIVADS_SOURCE_ASPECT_RATIO        0xC8
#define ZIVADS_SOURCE_TV_FORMAT           0x1E0


// *******************************************************************
// offsets from [ZIVADS_CSS_START]

#define ZIVADS_CSSOFFSET_COMMAND          0x00
#define ZIVADS_CSSOFFSET_STATUS           0x04
#define ZIVADS_CSSOFFSET_SENDCHALLENGEKEY 0x54
#define ZIVADS_CSSOFFSET_GETCHALLENGEKEY  0x2C
#define ZIVADS_CSSOFFSET_SENDRESPONSEKEY  0x68
#define ZIVADS_CSSOFFSET_GETRESPONSEKEY   0x7C
#define ZIVADS_CSSOFFSET_SENDTITLEKEY     0x90


// *******************************************************************
// Memory area names & numbers

#define ZIVADS_MEMAREA_DRAM	        0
#define ZIVADS_MEMAREA_ROM		1
#define ZIVADS_MEMAREA_IBUS		2

// *******************************************************************
// Register names

#define ZIVADS_REGCONTROL 7
#define ZIVADS_REGADDRESS 4
#define ZIVADS_REGDATA 0

// *******************************************************************
// Ziva commands

#define ZIVADS_CMD_ABORT 0x8120
#define ZIVADS_CMD_PAUSE 0x12A
#define ZIVADS_CMD_SETFILL 0x532
#define ZIVADS_CMD_SLOWFORWARDS 0x235
#define ZIVADS_CMD_BACKWARDS 0x33B
#define ZIVADS_CMD_SCAN 0x32F
#define ZIVADS_CMD_SINGLESTEP 0x134
#define ZIVADS_CMD_RESUME 0x12e
#define ZIVADS_CMD_PLAY 0x42b
#define ZIVADS_CMD_SELECTSTREAM 0x231
#define ZIVADS_CMD_RESET 0x802D
#define ZIVADS_CMD_NEWPLAYMODE 0x28
#define ZIVADS_CMD_HIGHLIGHT 0x326


// *******************************************************************
// misc defines 

// CSS hardware supported
#define ZIVADS_TYPE_1 1
#define ZIVADS_TYPE_2 2
#define ZIVADS_TYPE_3 3
#define ZIVADS_TYPE_4 4

// CSS decryption modes supported
#define ZIVADS_CSSDECRMODE_OFF 0
#define ZIVADS_CSSDECRMODE_ON  1

// subpicture modes
#define ZIVADS_SUBPICTURE_OFF 0
#define ZIVADS_SUBPICTURE_ON 1

// aspect ratios
#define ZIVADS_ASPECTRATIO_4_3 0
#define ZIVADS_ASPECTRATIO_16_9 1

// play rates
#define ZIVADS_PLAYRATE_2x 0
#define ZIVADS_PLAYRATE_3x 1
#define ZIVADS_PLAYRATE_4x 2
#define ZIVADS_PLAYRATE_5x 3
#define ZIVADS_PLAYRATE_6x 4

// bitstreams possibly present in files
#define ZIVADS_STREAM_VIDEO       0
#define ZIVADS_STREAM_SUBPICTURE  1
#define ZIVADS_STREAM_AUDIO_AC3   2
#define ZIVADS_STREAM_AUDIO_MPEG  3
#define ZIVADS_STREAM_AUDIO_LPCM  4
#define ZIVADS_STREAM_AUDIO_5     5

// bitstream types
#define ZIVADS_BITSTREAM_TYPE_MPEG_VOB       0
#define ZIVADS_BITSTREAM_TYPE_CDROM_VCD      1
#define ZIVADS_BITSTREAM_TYPE_MPEG_VCD       2
#define ZIVADS_BITSTREAM_TYPE_CDDA           3
#define ZIVADS_BITSTREAM_TYPE_4              4

// TV formats
#define ZIVADS_TVFORMAT_NTSC     0
#define ZIVADS_TVFORMAT_PAL      1

// video frequencies
#define ZIVADS_SRC_VIDEO_FREQ_30 0
#define ZIVADS_SRC_VIDEO_FREQ_25 1

// aspect ratio modes
#define ZIVADS_ASPECTRATIOMODE_NORMAL    0
#define ZIVADS_ASPECTRATIOMODE_PAN_SCAN  1
#define ZIVADS_ASPECTRATIOMODE_LETTERBOX 2

// play types
#define ZIVADS_PLAYTYPE_NORMAL 0
#define ZIVADS_PLAYTYPE_STILLSTOP 1

// iec-958 output types
#define ZIVADS_IEC958_DECODED    0
#define ZIVADS_IEC958_ENCODED    1

// AC3 modes
#define ZIVADS_AC3MODE_LR_STEREO          0
#define ZIVADS_AC3MODE_LR_STEREO_PROLOGIC 1
#define ZIVADS_AC3MODE_LR_MONOR           2

// AC3 voice configuration (for karaoke)
#define ZIVADS_AC3VOICE_NONE        0
#define ZIVADS_AC3VOICE_V1V2        1

// highlight actions
#define ZIVADS_BUTTONACTION_SELECT    0
#define ZIVADS_BUTTONACTION_UNHIGHLIGHT 1
#define ZIVADS_BUTTONACTION_ACTIVATE 2
#define ZIVADS_BUTTONACTION_ACTIVATE_SELECTED 3

// special buttons
#define ZIVADS_BUTTON_NONE    0
#define ZIVADS_BUTTON_UP     64
#define ZIVADS_BUTTON_DOWN   65
#define ZIVADS_BUTTON_LEFT   66
#define ZIVADS_BUTTON_RIGHT  67

// audio clock frequencies
#define ZIVADS_CLKFREQ256 0
#define ZIVADS_CLKFREQ384 1


// *******************************************************************
// Structures

typedef struct _zivaDS_ops_t zivaDS_ops_t;
typedef struct _zivaDS_t zivaDS_t;

// generic driver structure
struct _zivaDS_t {

  zivaDS_ops_t* ops;
  void* data;

  // type of Ziva chip
  int zivaDSType;

  // interrupt status
  int intEnabledFlag;
};

// lowlevel access operations
struct _zivaDS_ops_t {

  char name[32];
  int (*get_reg) (zivaDS_t* instance, int reg);
  int (*set_reg) (zivaDS_t* instance, int reg, int val);
  int (*get_mem) (zivaDS_t* instance, int addr);
  int (*set_mem) (zivaDS_t* instance, int addr, int val);
  int (*enable_int) (zivaDS_t* instance, int flag);
};


// to hold flags during CSS operations
typedef struct {
  
  unsigned int flag1;
  unsigned int flag2;
} zivaDS_cssFlags_t;


// to hold ziva interrupt src
typedef struct {
  
  unsigned int HLI_int_src;
  unsigned int BUF_int_src;
  unsigned int UND_int_src;
  unsigned int AOR_int_src;
  unsigned int AEE_int_src;
  unsigned int ERR_int_src;

} zivaDS_int_src_t;
  

// *******************************************************************
// function declarations

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Driver maintenance functions



/**
 *
 * Create new ZivaDS driver instance
 *
 * @param ops Lowlevel operations to talk to chip
 * @param data Any extra data for said functions
 *
 * @return The new instance (or NULL on error)
 *
 */

extern zivaDS_t* zivaDS_new(zivaDS_ops_t* ops, void* data);


/**
 *
 * Destroy an Anp82 driver instance
 *
 * @param instance The instance to destroy
 *
 */

extern void zivaDS_free(zivaDS_t* instance);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// High level convenience functions



/**
 *
 * This detects the Ziva chip. You can't just reset the thing, because
 * you need the firmware uploaded first (I think)... 
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_detect(zivaDS_t* instance);


/**
 *
 * Get the CSS flags from the ziva and store them in supplied structure
 *
 * @param instance Ziva instance to use
 * @param flags Where to store the flags
 *
 */

extern void zivaDS_get_css_flags(zivaDS_t* instance, zivaDS_cssFlags_t* flags);



/**
 *
 * Restore the CSS flags to the ziva from the supplied structure
 *
 * @param instance Ziva instance to use
 * @param flags Where to store the flags
 *
 */

extern void zivaDS_restore_css_flags(zivaDS_t* instance, zivaDS_cssFlags_t* flags);



/**
 *
 * Set the CSS mode... used for the tc6807af functions
 * You call this with mode=1 before doing anything with the tc6807af, and then with
 * mode=0 after you have finished.
 *
 * @param instance Ziva instance to use
 * @param mode Mode to set (0 or 1).
 *
 */

extern void zivaDS_set_css_mode(zivaDS_t* instance, int mode);



/**
 *
 * Enable/disable subpicture
 *
 * @param instance instance to use
 * @param flag one of ZIVADS_SUBPICTURE_OFF, ZIVADS_SUBPICTURE_ON
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_enable_subpicture(zivaDS_t* instance, int flag);



/**
 *
 * Abort playback
 *
 * @param instance zivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_abort(zivaDS_t* instance);




/**
 *
 * Set the output aspect ratio
 *
 * @param instance zivaDS instance to use
 * @param buffer instance of dxr2_oneArg_t. 
 *               arg is one of ZIVADS_ASPECTRATIO_4_3 or ZIVADS_ASPECTRATIO_16_9
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_output_aspect_ratio(zivaDS_t* instance, int ratio);



/**
 *
 * Set source aspect ratio
 *
 * @param instance ZivaDS instance
 * @param ratio one of ZIVADS_ASPECTRATIO_4_3, ZIVADS_ASPECTRATIO_16_9
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_source_aspect_ratio(zivaDS_t* instance, int ratio);



/**
 *
 * Set audio volume
 *
 * @param instance ZivaDS instance to use
 * @param volume Volume to to (0-19), 0 =min, 19=max
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_audio_volume(zivaDS_t* instance, int volume);



/**
 *
 * Pause playback
 *
 * @param instance ZivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_pause(zivaDS_t* instance);



/**
 *
 * Clear video
 *
 * @param instance ZivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_clear_video(zivaDS_t* instance);




/**
 *
 * Slow forwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_slow_forwards(zivaDS_t* instance, int rate);



/**
 *
 * Slow backwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_slow_backwards(zivaDS_t* instance, int rate);



/**
 *
 * Fast forwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_fast_forwards(zivaDS_t* instance, int rate);



/**
 *
 * Fast backwards
 *
 * @param instance ZivaDS instance
 * @param rate Rate of play (one of ZIVADS_PLAYRATE_2x, ZIVADS_PLAYRATE_3x
 *                           ZIVADS_PLAYRATE_4x, ZIVADS_PLAYRATE_5x, 
 *                           ZIVADS_PLAYRATE_6x)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_fast_backwards(zivaDS_t* instance, int rate);




/**
 * 
 * Set aspect ratio mode
 *
 * @param instance ZivaDS instance to use
 * @param mode Aspect ratio mode (one of ZIVADS_ASPECTRATIOMODE_NORMAL,
 *                                ZIVADS_ASPECTRATIOMODE_PAN_SCAN,
 *                                ZIVADS_ASPECTRATIOMODE_LETTERBOX)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_aspect_ratio_mode(zivaDS_t* instance, int mode);




/**
 * 
 * Single Step
 *
 * @param instance ZivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_single_step(zivaDS_t* instance);



/**
 * 
 * Reverse play
 *
 * @param instance ZivaDS instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_reverse_play(zivaDS_t* instance);



/**
 * 
 * Set subpicture palettes
 *
 * @param instance ZivaDS instance to use	
 * @param palette (16x32bit palette entries)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_subpicture_palettes(zivaDS_t* instance, unsigned int palettes[]);


/**
 *
 * Initialise the ziva
 *
 * @param instance instance to use
 * @param uCode ucode data to upload
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_init(zivaDS_t* instance, char* uCode);



/**
 *
 * Resume play (e.g. if the device is paused)
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_resume(zivaDS_t* instance);



/**
 *
 * Start play (e.g. if the device is stopped)
 *
 * @param instance instance to use
 * @param playType ZIVADS_PLAYTYPE_NORMAL, ZIVADS_PLAYTYPE_STILLSTOP
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_play(zivaDS_t* instance, int arg);




/**
 *
 * Select stream
 *
 * @param streamType Type of stream. One of ZIVADS_STREAM_VIDEO, ZIVADS_STREAM_SUBPICTURE,
 *                   ZIVADS_STREAM_AUDIO_AC3, ZIVADS_STREAM_AUDIO_MPEG1, ZIVADS_STREAM_AUDIO_LPCM,
 *                   ZIVADS_STREAM_AUDIO4
 * @param streamNumber Stream of that type to select
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_select_stream(zivaDS_t* instance, 
				int streamType, int streamNumber);

/**
 *
 * Reset the Ziva
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_reset(zivaDS_t* instance);



/**
 *
 * Performs a new play mode command on the ziva
 *
 * @param instance instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_new_play_mode(zivaDS_t* instance);



/**
 *
 * Sets the bitstream type
 *
 * @param instance instance to use
 * @param type Bitstream type, one of ZIVADS_BITSTREAM_TYPE_MPEG_VOB,
 * ZIVADS_BITSTREAM_TYPE_CDROM_VCD, ZIVADS_BITSTREAM_TYPE_MPEG_VCD, 
 * ZIVADS_BITSTREAM_TYPE_CDDA, ZIVADS_BITSTREAM_TYPE_4
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_bitstream_type(zivaDS_t* instance, int type);



/**
 *
 * Sets the source video frequency
 *
 * @param instance instance to use
 * @param freq ZIVADS_SRC_VIDEO_FREQ_30, ZIVADS_SRC_VIDEO_FREQ_25
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_source_video_frequency(zivaDS_t* instance, int format);



/**
 *
 * Setup audio DAC.
 *
 * @param instance ZivaDS instance to use
 * @param dacMode Current Audio DAC mode
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_setup_audio_dac(zivaDS_t* instance, int dacMode);



/**
 *
 * Get the most significant 32 bits of the STC value
 * this is the System time..
 *
 * @param instance instance to use
 *
 * @return Value on success, <0 on failure
 *
 */

extern int zivaDS_get_mrc_pic_stc(zivaDS_t* instance);



/**
 * 
 * Get the most significant 32 bits of the PTS value
 * this is the presentation time..
 *
 * @param instance instance to use
 *
 * @return Value on success, <0 on failure
 *
 */

extern int zivaDS_get_mrc_pic_pts(zivaDS_t* instance);



/**
 * 
 * Sets the IEC-958 output mode (either decoded AC3, or encoded AC3)
 *
 * @param instance instance to use
 * @param flag (ZIVADS_IEC958_DECODED, ZIVADS_IEC958_ENCODED)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_iec958_output_mode(zivaDS_t* instance, int flag);




/**
 * 
 * Set the AC3 mode... probably for Karaoke... Mmmmm... how useful ;)
 *
 * @param instance instance to use
 * @param dacMode Current Audio DAC mode
 * @param param One of ZIVADS_AC3MODE_LR_STEREO, 
 *                     ZIVADS_AC3MODE_LR_STEREO_PROLOGIC,
 *                     ZIVADS_AC3MODE_LR_MONOR
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_AC3_mode(zivaDS_t* instance, 
			       int currentDacMode,
			       int param);



/**
 * 
 * Selects AC3 voice, either to NONE, or V1V2. This is for karaoke
 *
 * @param instance instance to use
 * @param voice Voice to select (ZIVADS_AC3VOICE_NONE, ZIVADS_AC3VOICE_V1V2)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_select_AC3_voice(zivaDS_t* instance, int voice);


/**
 *
 * Set the audio attenuation
 *
 * @param instance instance to use
 * @param attenuation The attenuation
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_audio_attenuation(zivaDS_t* instance, int attenuation);



/**
 *
 * Highlights a button
 *
 * @param instance zivaDS instance
 * @param button Index of button to highlight (1-36), or ZIVADS_BUTTON_NONE,
 *     ZIVADS_BUTTON_UP, ZIVADS_BUTTON_DOWN, ZIVADS_BUTTON_LEFT, 
 *     ZIVADS_BUTTON_RIGHT
 * @param action to perform on button:
 *     ZIVADS_BUTTONACTION_SELECT, ZIVADS_BUTTONACTION_UNHIGHLIGHT,
 *     ZIVADS_BUTTONACTION_ACTIVATE, ZIVADS_BUTTONACTION_ACTIVATE_SELECTED
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_highlight(zivaDS_t* instance, int buttonIndex, int colour);


/**
 *
 * Waits for the current HLI interrupt to finish??
 *
 * @param instance zivaDS instance
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_wait_for_HLI_int(zivaDS_t* instance);



/**
 *
 * Sets the ziva Audio clock frequency
 *
 * @param instance ZivaDS instance
 * @param freq Frequency to set (ZIVADS_CLKFRE256 or ZIVADS_CLKFREQ384)
 *
 */

extern int zivaDS_set_audio_clock_frequency(zivaDS_t* instance, int freq);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CSS functions


/**
 *
 * Sets the CSS decryption mode
 *
 * @param instance Ziva instance to use
 * @param mode CSS mode one of ZIVADS_CSSDECRMODE_ON,ZIVADS_CSSDECRMODE_OFF
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_set_decryption_mode(zivaDS_t* instance, int mode);


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

extern int zivaDS_send_challenge_key(zivaDS_t* instance, unsigned char* key);


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

extern int zivaDS_get_challenge_key(zivaDS_t* instance, unsigned char* key);


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

extern int zivaDS_send_response_key(zivaDS_t* instance, unsigned char* key);


/**
 *
 * Get the challenge key from the Ziva
 *
 * @param instance Instance of the Ziva to use
 * @param key 5 byte char array to put the key in
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_get_response_key(zivaDS_t* instance, unsigned char* key);


/**
 *
 * Part 1 of send disc key (setup)
 *
 * @param instance Instance of the Ziva to use
 *
 * @return 0 on success, or <0 on failure
 *
 */

extern int zivaDS_send_disc_key_part1(zivaDS_t* instance);


/**
 *
 * Part 2 of send disc key (finalise)
 *
 * @param instance Instance of the Ziva to use
 *
 * @return 0 on success, or <0 on failure
 *
 */

extern int zivaDS_send_disc_key_part2(zivaDS_t* instance);


/**
 *
 * Send the disc title key
 *
 * @param instance Instance of the Ziva to use
 * @param key 6 byte char array containing the key to send
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_send_title_key(zivaDS_t* instance, unsigned char* key);


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Low level functions


/**
 *
 * Get register from the zivaDS
 *
 * @param instance Instance of the zivaDS to use
 * @param reg Register to retrieve
 *
 * @return The register's value, or <0 on failure
 *
 */

extern int zivaDS_get_reg(zivaDS_t* instance, int reg);


/**
 *
 * Set register on the ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param val Value to set
 *
 */

extern void zivaDS_set_reg(zivaDS_t* instance, int reg, int val);


/**
 *
 * Get specified bitmask of a register from ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to retrive from that register
 *
 * @return The register bitvalues, or <0 on failure
 *
 */

extern int zivaDS_get_bits(zivaDS_t* instance, int reg, int bitmask);


/**
 *
 * Set specified bits of a register on ZivaDS
 *
 * @param instance Instance of the ZivaDS to use
 * @param reg Register to retrieve
 * @param bitmask Bitmask of bits to set from that register
 * @param valuemask Values of the bits in the bitmask
 *
 */

extern void zivaDS_set_bits(zivaDS_t* instance, int reg, int bitmask, 
			    int valuemask);


/**
 *
 * Read from a ZivaDS memory location
 *
 * @param instance Instance of Ziva DS to read from
 * @param addr Address to read from in that memory area
 *
 * @return Memory Value
 *
 */

extern int zivaDS_get_mem(zivaDS_t* instance, int addr);


/**
 *
 * Write to a ZivaDS memory location
 *
 * @param instance Instance of Ziva DS to read from
 * @param addr Address to write to in that memory area
 * @param val Value to write
 *
 */

extern void zivaDS_set_mem(zivaDS_t* instance, int addr, int val);



/**
 *
 * Issues a command to the Ziva chip
 *
 * @param instance Instance of the ZivaDS to use
 * @param command Ziva command to send
 * @param arg0 Argument 0 for command
 * @param arg1 Argument 1 for command
 * @param arg2 Argument 2 for command
 * @param arg3 Argument 3 for command
 * @param arg4 Argument 4 for command
 * @param arg5 Argument 5 for command
 * @param intMask Ziva Int Mask to set (0 for don't bother)
 * @param statusToWaitFor Ziva Status to wait for (0 for don't bother)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_command(zivaDS_t* instance,
			  int command,
			  int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
			  int intMask, int statusToWaitFor);




/**
 *
 * Determines whether the ziva Int is currently enabled
 *
 * @param instance zivaDS instance
 *
 * @return 0 => int NOT enabled, 1 => int enabled
 *
 */

extern int zivaDS_is_int_enabled(zivaDS_t* instance);


/**
 *
 * Enable/disable ziva interrupt
 *
 * @param instance zivaDS instance
 * @param flag 0=> disable, 1=> enable
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int zivaDS_enable_int(zivaDS_t* instance, int flag);




/**
 *
 * Get ziva int status
 *
 * @param instance zivaDS instance
 * @param itnSrcBuf buffer to hold intsrcs
 * 
 * @return int status bitmap from ziva
 *
 */

extern int zivaDS_get_int_status(zivaDS_t* instance, zivaDS_int_src_t* intSrcBuf);


/**
 *
 * Checks if the ziva is ZIVADS_TYPE_1 or not (i.e. doesn't have onboard CSS)
 *
 * @param instance zivaDS instance
 *
 * @return 0 => NOT ZIVADS_TYPE_1, 1=> it IS ZIVADS_TYPE_1
 *
 */

extern int zivaDS_check_type_1(zivaDS_t* instance);


#endif
