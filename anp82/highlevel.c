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
 * Driver for the AnP82 VGA overlay processor
 * High level convenience functions
 *
 */

#include <dxr2modver.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <anp82.h>


static int anp82_wait_vsync(anp82_t* instance);
static int anp82_measure_vsync_value(anp82_t* instance, int whichValue);
static int anp82_measure_hsync_value(anp82_t* instance, int whichValue);



/**
 *
 * Set the Anp82's overlay colour
 *
 * @param instance AnP82 instance to use
 * @param red_low Lower bound for colour
 * @param red_high Upper bound for colour
 * @param green_low Lower bound for colour
 * @param green_high Upper bound for colour
 * @param blue_low Lower bound for colour
 * @param blue_high Upper bound for colour
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_overlay_colour(anp82_t* instance, 
				    int red_low, int red_high,
				    int green_low, int green_high,
				    int blue_low, int blue_high)
{
  int ckOffset;


  // read the colour key offset
  ckOffset = anp82_get_reg(instance, ANP82_CKOFFSET);

  // Red colour key
  anp82_set_reg(instance, ANP82_CKREDH, red_high);
  anp82_set_reg(instance, ANP82_CKREDL, red_low);

  // setup Red colour key offset
  // Oh goody! Auravision have different register bit assignments
  // depending on which AnP type you have! ARRRRGH!
  if (red_low != 0) {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset |= 2;
    }
    else {
      
      ckOffset &= 0xFC;
    }
  }
  else {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset &= 0xFD;
    }
    else {
      
      ckOffset |= 3;
    }
  }
  
  // Green colour key
  anp82_set_reg(instance, ANP82_CKGREENH, green_high);
  anp82_set_reg(instance, ANP82_CKGREENL, green_low);

  // setup Green colour key offset
  // Oh goody! Auravision have different register bit assignments
  // depending on which AnP type you have! ARRRRGH!
  if (green_low != 0) {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset |= 8;
    }
    else {
      
      ckOffset &= 0xF3;
    }
  }
  else {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset &= 0xF7;
    }
    else {
      
      ckOffset |= 0x0C;
    }
  }

  // Blue colour key
  anp82_set_reg(instance, ANP82_CKBLUEH, blue_high);
  anp82_set_reg(instance, ANP82_CKBLUEL, blue_low);

  // setup Blue colour key offset
  // Oh goody! Auravision have different register bit assignments
  // depending on which AnP type you have! ARRRRGH!
  if (blue_low != 0) {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset |= 4;
    }
    else {
      
      ckOffset &= 0xCF;
    }
  }
  else {

    if (instance->chip_id == ANP_TYPE_83) {
      
      ckOffset &= 0xFB;
    }
    else {
      
      ckOffset |= 0x30;
    }
  }

  // OK, set the new colour key offset
  anp82_set_reg(instance, ANP82_CKOFFSET, ckOffset);

  // OK
  return(0);
}



/**
 *
 * Set the colour gain
 *
 * @param instance AnP82 instance
 * @param common Common gain (0 - 0x3f)
 * @param red red gain (0- 63)
 * @param green green gain (0- 63)
 * @param blue blue gain (0- 63)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_gain(anp82_t* instance, 
			  int common,
			  int red,
			  int green,
			  int blue) 
{
  anp82_set_reg(instance, ANP82_RDACL, (red & 0xff) << 4);
  anp82_set_reg(instance, ANP82_GDACL, (green & 0xff) << 4);
  anp82_set_reg(instance, ANP82_GDACL, (blue & 0xff) << 4);
  anp82_set_reg(instance, ANP82_VDACCG, common & 0x3f);

  // OK
  return(0);
}


/**
 *
 * Sets the "in delay" value for the AnP82
 *
 * @param instance AnP82 instance to use
 * @param value In delay value (0-3)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_in_delay(anp82_t* instance, int value) 
{
  anp82_set_bits(instance, ANP82_PCLKOUT, 0x03, value & 3);
  
  // OK
  return(0);
}



/**
 *
 * Set overlay mode
 *
 * @param instance AnP82 instance
 * @param value One of ANP82_OVERLAY_*
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_overlay_mode(anp82_t* instance, int value)
{
  switch(value) {
  case ANP82_OVERLAY_DISABLED:
    
    anp82_set_bits(instance, ANP82_KEYCTRL, 6, 0);
    break;
  
  case ANP82_OVERLAY_WINDOW_KEY:

    anp82_set_bits(instance, ANP82_KEYCTRL, 6, 2);
    break;

  case ANP82_OVERLAY_COLOUR_KEY:

    anp82_set_bits(instance, ANP82_KEYCTRL, 6, 4);
    break;

  case ANP82_OVERLAY_WINDOW_COLOUR_KEY:

    anp82_set_bits(instance, ANP82_KEYCTRL, 6, 6);
    break;
    
  default:
    
    return(-EINVAL);
  }

  // OK
  return(0);
}
    

/**
 *
 * Initialises the AnP82
 *
 * @param instance AnP82 instance to use
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_init(anp82_t* instance)
{
  int val;

  // work out the ID
  val = anp82_get_reg(instance, 1);
  if (val == 0x28) instance->chip_id = ANP_TYPE_81; // AnP81 (I think)
  else if (val == 0x68) instance->chip_id = ANP_TYPE_82; // AnP82 Rev. B
  else if ((val & 0x3f) == 0x28) instance->chip_id = ANP_TYPE_82_REVC; // AnP82 Rev. C, D (don't know if Rev. D existed)
  else if ((val & 0x3f) == 0x29) instance->chip_id = ANP_TYPE_83; // AnP83
  else {
    
    printk(KERN_ERR "Unknown AnP Type (0x%x). Defaulting to AnP82 Rev C.\n", val);
    instance->chip_id = ANP_TYPE_82_REVC;
  }

  // enable extended DAC power saving mode, turn power on to all other components,
  anp82_set_reg(instance, ANP82_PWRSAVE, 0x20);


  // setup inital overlay colour
  anp82_set_overlay_colour(instance, 
			   0xff, 00,
			   0xff, 00,
			   0xff, 00);

  // DAC levels
  anp82_set_reg(instance, ANP82_RDACL, 0xa0);
  anp82_set_reg(instance, ANP82_GDACL, 0xa0);
  anp82_set_reg(instance, ANP82_BDACL, 0xa0);
  anp82_set_reg(instance, ANP82_VDACCG, 0xf0);

  // init DAC control  (changed to 0x0b from 00 for AnP82 Rev C)
  // (in delay = EXTF, DAC clock delay=3, DAC clock inverted)
  //  anp82_set_reg(instance, ANP82_DACCTRL, 0x00);
  anp82_set_reg(instance, ANP82_DACCTRL, 0x0b);
  
  // invert PCLK output, no delay to PCLK
  anp82_set_reg(instance, ANP82_PCLKOUT, 0x08);

  // alpha mixing disabled
  anp82_set_reg(instance, ANP82_ALPHAMIX, 0x00);

  // PCLK/1, external window key active high
  anp82_set_reg(instance, ANP82_VBSH, 0x10);

  // fading off
  anp82_set_reg(instance, ANP82_VDACCR, 0);
  anp82_set_reg(instance, ANP82_FADETIME, 0);
  
  // video blank off, overlay off, key feedback off, VxP available, 
  // PCLK comparison every four PCLK cycles (AnP82)
  anp82_set_reg(instance, ANP82_KEYCTRL, 0x90);

  // gating region disabled, other unknown things
  anp82_set_reg(instance, ANP82_MISCCTRL, 0x36);

  // OK
  return(0);
}


extern int anp82_shutdown(anp82_t* instance)
{
  // turn power off to everything
  anp82_set_reg(instance, ANP82_PWRSAVE, 0x27);
  // changed by zulli
  return(1);
}




/**
 *
 * Measures the video blank start & width
 *
 * @param instance AnP82 instance
 * @param vbStart Where to put the VB start value
 * @param vbWidth Where to put the VB width value
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_video_blank(anp82_t* instance, int* vbStart, int* vbWidth)
{
  int hi=0;
  int lo=0;
  int start;
  int width;
  int i;
  int tmp;
  
  printk(KERN_ERR "anp82_measure_video_blank()\n");


  // measure the widths of the various bits of the sync
  for(i=0; i< 2; i++) {

    tmp = anp82_measure_hsync_value(instance, ANP82_MEASURE_LOW);
    if (lo < tmp) {
      
      lo = tmp;
    }

    tmp = anp82_measure_hsync_value(instance, ANP82_MEASURE_HIGH);
    if (hi < tmp) {
      
      hi = tmp;
    }
  }
  
  // OK, calculate the values
  if (hi <= lo) { // i.e. HSYNC is +ve
    
    start = lo - (hi / 5);
    width = (hi * 5) / 3;
  }
  else {
    
    start = hi - (lo / 5);
    width = (lo * 5) / 3;
  }
  
  // return 'em
  *vbStart = start;
  *vbWidth = width;

  // OK!
  return(0);  
}


/**
 *
 * Set the video blank registers
 *
 * @param instance anp82 instance to use
 * @param vbStart Video blank start
 * @param vbWidth Video blank width
 *
 * @return 0 on success, <0 on failure 
 *
 */

extern int anp82_set_video_blank(anp82_t* instance, int vbStart, int vbWidth)
{

  printk(KERN_ERR "anp82_set_video_blank()\n");

  // setup the VB start regs
  anp82_set_reg(instance, ANP82_VBSL, vbStart & 0xff);
  anp82_set_bits(instance, ANP82_VBSH, 0xf8, (vbStart & 0x700) >> 8);
  
  // setup the VB width register
  anp82_set_reg(instance, ANP82_VBSW, vbWidth);

  // ok
  return(0);
}


/**
 *
 * Set sync polarities
 *
 * @param instance AnP82 instance
 * @param hPolarity hsync polarity (one of ANP82_SYNC_ACTIVE_*)
 * @param vPolarity vsync polarity (one of ANP82_SYNC_ACTIVE_*)
 *
 * @return 0 on success, <0 on failure
 *
 */

extern int anp82_set_sync_polarities(anp82_t* instance, 
				     int hPolarity, int vPolarity)
{
  int value = 0;
  
  printk(KERN_ERR "anp82_set_sync_polarities()\n");

  // work out HSYNC
  // don't set HSYNC to anything other than 0 if we're >= Rev C Anp82
  if ((instance->chip_id == ANP_TYPE_81) ||
      (instance->chip_id == ANP_TYPE_82)) {
    
    if (hPolarity == ANP82_SYNC_ACTIVE_HIGH) {
      
      value |= 0x80;
    }
  }
  
  // work out VSYNC
  if (vPolarity == ANP82_SYNC_ACTIVE_HIGH) {
    
    value |= 0x40;
  }

  // set it
  anp82_set_bits(instance, ANP82_VBSH, 0xC0, value);

  // OK
  return(0);
}



  
/**
 *
 * Measures the vertical picture offset
 *
 * @param instance AnP82 instance
 * @param offset Current V position of overlay window
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_vertical_offset(anp82_t* instance, int offset)
{
  int ckeyLead;
  int wkeyLead;

  printk(KERN_ERR "anp82_measure_vertical_offset()\n");

  // setup for measuring vertical leading edge of colour key,
  // HSYNC clock src, measure VSYNC
  anp82_set_reg(instance, ANP82_CNTCTRLB, 0x2D);

  // start timer measuring low portion of signal
  anp82_set_reg(instance, ANP82_CNTCTRL, 0);
  anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);

  // wait a bit
  anp82_wait_vsync(instance);
  anp82_wait_vsync(instance);

  // get the value
  ckeyLead = ANP82_GET_TIMER(instance);

  // setup for measuring vertical leading edge of window key
  // HSYNC clock src, measure VSYNC
  anp82_set_reg(instance, ANP82_CNTCTRLB, 0xAD);

  // start timer measuring low portion of signal
  anp82_set_reg(instance, ANP82_CNTCTRL, 0);
  anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);

  // wait a bit
  anp82_wait_vsync(instance);
  anp82_wait_vsync(instance);

  // get the value
  wkeyLead = ANP82_GET_TIMER(instance);

  // OK, return the result
  return(offset + ckeyLead - wkeyLead);
}

  
/**
 *
 * Measures the horizontal ratio for converting VGA card pixels to VxP pixels
 *
 * @param instance AnP82 instance
 * @param screenWidth Width of VGA card's screen in pixels
 *
 * @return ratio (if >0), or error if <0
 *
 */

extern int anp82_measure_horizontal_ratio(anp82_t* instance, int screenWidth)
{
  int ckeyLead=0;
  int ckeyTrail=0;
  int ratio;
  int i;
  int tmp;
  int acc;

  printk(KERN_ERR "anp82_measure_horizontal_ratio()\n");

  // I tried this with 20 before, but every so often the AnP82 was out by 1.
  // 40 seems to give it the number of reads it needs to get the correct number
  for(i=0; i< 40; i++) {
    
    // setup for measuring horizontal leading edge of colour key
    // EF1 clock src, measure HSYNC
    anp82_set_reg(instance, ANP82_CNTCTRLB, 0xC);
    
    // start timer measuring low portion of signal
    anp82_set_reg(instance, ANP82_CNTCTRL, 0);
    anp82_wait_vsync(instance);
    anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);
    
    // wait a bit
    anp82_wait_vsync(instance);


    // get the value
    tmp  = ANP82_GET_TIMER(instance);
    
    // Hmmm... the AnP82 regularly reads this completely wrongly!!!
    // it seems to set the top 4 bits to 1001 quite a lot.... so we just
    // also, it always seems to be the smaller numbers that are correct
    // so we just get the minimum number
    if (((tmp < ckeyLead) && (tmp > 0)) ||
	(ckeyLead == 0)) {

      ckeyLead = tmp;
    }
  }
  
  acc = 0;
  for(i=0; i< 20; i++) {
  
    // setup for measuring horizontal trailing edge of colour key
    // EF1 clock src, measure HSYNC
    anp82_set_reg(instance, ANP82_CNTCTRLB, 0x4C);
    
    // start timer measuring low portion of signal
    anp82_set_reg(instance, ANP82_CNTCTRL, 0);
    anp82_wait_vsync(instance);
    anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);
    
    // wait a bit
    anp82_wait_vsync(instance);

    // get the value
    acc += ANP82_GET_TIMER(instance);
  }
  ckeyTrail = acc / i;    

  // calculate the ratio
  ratio = ((ckeyTrail - ckeyLead) * 1000) / screenWidth;
  if (ratio == 0) {
    
    ratio = 1;
  }
  
  // OK, return the ratio
  return(ratio);
}


  
/**
 *
 * Measures the horizontal picture offset
 *
 * @param instance AnP82 instance
 * @param offset Current H position of overlay window
 *
 * @return offset (if >0), or error if <0
 *
 */

extern int anp82_measure_horizontal_offset(anp82_t* instance, int offset)
{
  int ckeyLead = 0;
  int wkeyLead = 0;
  int i;
  int tmp;
  // changed by zulli
//  int endTime;
  int acc;


  printk(KERN_ERR "anp82_measure_horizontal_offset()\n");

  // I tried this with 20 before, but every so often the AnP82 was out by 1.
  // 40 seems to give it the number of reads it needs to get the correct number
  for(i=0; i< 40; i++) {
    
    // setup for measuring horizontal leading edge of colour key
    // EF1 clock src, measure HSYNC
    anp82_set_reg(instance, ANP82_CNTCTRLB, 0xC);
    
    // start timer measuring low portion of signal
    anp82_set_reg(instance, ANP82_CNTCTRL, 0);
    anp82_wait_vsync(instance);
    anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);
    anp82_wait_vsync(instance);

    // get the value
    tmp  = ANP82_GET_TIMER(instance);
    
    // Hmmm... the AnP82 regularly reads this completely wrongly!!!
    // it seems to set the top 4 bits to 1001 quite a lot.... so we just
    // also, it always seems to be the smaller numbers that are correct
    // so we just get the minimum number
    if (((tmp < ckeyLead) && (tmp > 0)) ||
	(ckeyLead == 0)) {

      ckeyLead = tmp;
    }
  }


  acc = 0;
  for(i=0; i< 20; i++) {

    // setup for measuring horizontal leading edge of window key
    // EF1 clock src, measure HSYNC
    anp82_set_reg(instance, ANP82_CNTCTRLB, 0x8C);

    // start timer measuring low portion of signal
    anp82_set_reg(instance, ANP82_CNTCTRL, 0);
    anp82_wait_vsync(instance);
    anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);
    
    // wait a bit
    anp82_wait_vsync(instance);
    
    // get the value
    acc += ANP82_GET_TIMER(instance);
  }
  wkeyLead = acc / i;

  // OK, return the result
  return(offset + ckeyLead - wkeyLead);
}



/**
 *
 * Work out the vsync polarity
 *
 * @param instance AnP82 instance to use
 *
 * @return One of ANP82_SYNC_ACTIVE_* on success, <0 on error
 *
 */

extern int anp82_measure_vsync_polarity(anp82_t* instance)
{
  int vHigh;
  int vLow;
  
  printk(KERN_ERR "anp82_measure_vsync_polarity()\n");

  // measure the values
  vLow = anp82_measure_vsync_value(instance, ANP82_MEASURE_LOW);
  vHigh = anp82_measure_vsync_value(instance, ANP82_MEASURE_HIGH);
  
  // if vlow < vhigh => sync is high for longer => sync is active low
  if (vLow < vHigh) {
    
    return(ANP82_SYNC_ACTIVE_LOW);
  }
  
  // otherwise, sync is active high
  return(ANP82_SYNC_ACTIVE_HIGH);
}



/**
 *
 * Work out the hsync polarity
 *
 * @param instance AnP82 instance to use
 *
 * @return One of ANP82_SYNC_ACTIVE_* on success, <0 on error
 *
 */

extern int anp82_measure_hsync_polarity(anp82_t* instance)
{
  int vHigh;
  int vLow;
  
  printk(KERN_ERR "anp82_measure_hsync_polarity()\n");

  // measure the values
  vLow = anp82_measure_hsync_value(instance, ANP82_MEASURE_LOW);
  vHigh = anp82_measure_hsync_value(instance, ANP82_MEASURE_HIGH);
  
  // if vlow < vhigh => sync is high for longer => sync is active low
  if (vLow < vHigh) {
    
    return(ANP82_SYNC_ACTIVE_LOW);
  }
  
  // otherwise, sync is active high
  return(ANP82_SYNC_ACTIVE_HIGH);
}
  




// ---------------------------------------------------------------------
// private functions


/**
 *
 * Wait until VSYNC occurs
 *
 * @param anp82 instance to use
 *
 * @return 1=> VSYNC detected, 0=> VSYNC not detected
 *
 */

static int anp82_wait_vsync(anp82_t* instance)
{
  int i;
  int loopOk;
  int endTime;

  printk(KERN_ERR "anp82_wait_vsync()\n");

  if (instance->chip_id == ANP_TYPE_83) {
    
    // The docs don't describe what register 0x2a contains
    anp82_set_reg(instance, ANP82_INTRCFG, 0);
    anp82_set_reg(instance, ANP82_INTRCFG, 3);

    // wait until VSYNC turns on
    loopOk = 0;
    endTime = jiffies + ((20*HZ)/100);
    while(jiffies < endTime) {

      // let other things in
      schedule();

      // check if VSYNC is set yet
      if (anp82_get_reg(instance, ANP82_INTRSTAT) & 2) {
	
	loopOk = 1;
	break;
      }
    }
    if (!loopOk) return(0);

    // wait until VSYNC turns off
    loopOk = 0;
    endTime = jiffies + ((20*HZ)/100);
    while(jiffies < endTime) {

      // let other things in
      schedule();

      // The docs don't describe what register 0x2a contains
      anp82_set_reg(instance, ANP82_INTRCFG, 0);
      anp82_set_reg(instance, ANP82_INTRCFG, 3);

      if (!(anp82_get_reg(instance, ANP82_INTRSTAT) & 2)) {
	
	loopOk = 1;
	break;
      }
    }
    if (!loopOk) return(0);
  }
  else {
    
    // The docs don't describe what register 0x2a contains
    anp82_set_reg(instance, ANP82_INTRCFG, 0);
    anp82_set_reg(instance, ANP82_INTRCFG, 3);    

    // wait until VSYNC turns on
    loopOk = 0;
    endTime = jiffies + ((20*HZ)/100);
    while(jiffies < endTime) {

      // let other things in
      schedule();

      // is VSYNC set yet?
      if (anp82_get_reg(instance, ANP82_INTRSTAT) & 2) {
	
	loopOk = 1;
	break;
      }
    }
    if (!loopOk) return(0);

    // wait until VSYNC turns off
    for(i=0; i< 30; i++) {
      
      // The docs don't describe what register 0x2a contains
      anp82_set_reg(instance, ANP82_INTRCFG, 0);
      anp82_set_reg(instance, ANP82_INTRCFG, 5);

      // wait until HSYNC turns on
      loopOk = 0;
      endTime = jiffies + ((20*HZ)/100);
      while(jiffies < endTime) {
	
	// let other things in
	schedule();

	// is HSYNC set yet?
	if (anp82_get_reg(instance, ANP82_INTRSTAT) & 4) {
	  
	  loopOk = 1;
	  break;
	}
      }
      if (!loopOk) return(0);
    }
  }
  
  // OK, detected it
  return(1);
}  


/**
 *
 * Measure one of the HSYNC timings
 *
 * @param instance AnP82 instance to use
 * @param whichValue Which value to measure (one of ANP82_MEASURE_*)
 *
 * @param the measured value if >0, or error if <0
 *
 */

static int anp82_measure_hsync_value(anp82_t* instance, int whichValue) 
{
  int i;
  int endTime;
  int table[20];


  printk(KERN_ERR "anp82_measure_hsync_value()\n");

  // OK, take the timing 20 times
  for(i=0; i< 20; i++) {

    // setup for measuring HSYNC
    anp82_set_reg(instance, ANP82_CNTCTRLB, 0);
    
    // select which thing to read
    switch(whichValue) {
    case ANP82_MEASURE_LOW:

      anp82_set_reg(instance, ANP82_CNTCTRL, 0);
      break;

    case ANP82_MEASURE_HIGH:

      anp82_set_reg(instance, ANP82_CNTCTRL, 2);
      break;
      
    default:
      
      return(-EINVAL);
    }

    // start the counter going
    anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);

    // small delay (hsyncs are pretty short)
    endTime = jiffies + ((1*HZ)/100);
    while(jiffies < endTime) {

      // let other things in
      schedule();
    }

    // get value & store it
    table[i] = ANP82_GET_TIMER(instance);

    // need >= 2 values for the next bit
    if (i < 2) 
      continue;

    // OK, if i >= 2 and the last few values read aren't TOOO different,
    // exit the loop
    if (((table[i] - 1) <= table[i - 1]) && ((table[i] + 1) >= table[i - 1]) &&
	((table[i - 1] - 1) <= table[i - 2]) && ((table[i - 1] + 1) >= table[i - 2])) {
      
      break;
    }
  }
    
  // return the last read value
  return(table[i]);
}



/**
 *
 * Measure one of the VSYNC timings
 *
 * @param instance AnP82 instance to use
 * @param whichValue Which value to measure (one of ANP82_MEASURE_*)
 *
 * @param the measured value if >0, or error if <0
 *
 */

static int anp82_measure_vsync_value(anp82_t* instance, int whichValue)
{

  printk(KERN_ERR "anp82_measure_vsync_value()\n");

  // setup for measuring VSYNC
  anp82_set_reg(instance, ANP82_CNTCTRLB, 5);
    
  // select which thing to read
  switch(whichValue) {
  case ANP82_MEASURE_LOW:
    
    anp82_set_reg(instance, ANP82_CNTCTRL, 0);
    break;
    
  case ANP82_MEASURE_HIGH:
    
    anp82_set_reg(instance, ANP82_CNTCTRL, 2);
    break;
    
  default:
    
    return(-EINVAL);
  }

  // start the counter going
  anp82_set_bits(instance, ANP82_CNTCTRL, 1, 1);

  // wait a bit
  anp82_wait_vsync(instance);
  anp82_wait_vsync(instance);

  // return the value
  return(ANP82_GET_TIMER(instance));
}
