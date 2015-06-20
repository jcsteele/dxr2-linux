#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

#define AllPointerEventMask \
	(ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | \
	PointerMotionMask | PointerMotionHintMask | \
	Button1MotionMask | Button2MotionMask | \
	Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask | \
	KeymapStateMask)

int getScreenRes(int*xres, int*yres) 
{
  Display* dsp = NULL;
  Screen* screen;
  XF86VidModeModeLine modeLine;
  Window rootWindow;
  int dotClk;


  // open display
  if (!(dsp = XOpenDisplay(NULL))) {
    
    fprintf(stderr, "Cannot open default display\n");
    return(0);
  }

  // get the screen
  screen = DefaultScreenOfDisplay(dsp);

  // get the root window of it
  rootWindow = RootWindowOfScreen(screen);

  // get the screen width & height
  XF86VidModeGetModeLine(dsp, DefaultScreen(dsp), &dotClk, &modeLine);
  *xres = modeLine.hdisplay;
  *yres = modeLine.vdisplay;

  // close display again
  XCloseDisplay(dsp);
}  
  

int whitescreen()
{
  Display* dsp = NULL;
  Window rootWindow;
  Window blackWindow;
  Window whiteWindow;
  Screen* screen;
  int screenNo;
  XSetWindowAttributes xswa;
  XEvent evt;
  int endLoop;
  Cursor myCursor;
  Pixmap lockc;
  Pixmap lockm;
  char no_bits[] = {0};
  XColor nullcolor;
  int dotClk;
  XF86VidModeModeLine modeLine;
  int scrWidth;
  int scrHeight;


  // open display
  if (!(dsp = XOpenDisplay(NULL))) {
    
    fprintf(stderr, "Cannot open default display\n");
    goto err_opendisplay;
  }

  // get the screen
  screen = DefaultScreenOfDisplay(dsp);

  // get the root window of it
  rootWindow = RootWindowOfScreen(screen);

  // setup window attributes
  xswa.background_pixel = WhitePixelOfScreen(screen); // 0x0010#10e0;
  xswa.override_redirect = True;

  // create blank cursor
  lockc = XCreateBitmapFromData(dsp, rootWindow, no_bits, 1, 1);
  lockm = XCreateBitmapFromData(dsp, rootWindow, no_bits, 1, 1);
  myCursor = XCreatePixmapCursor(dsp, lockc, lockm, &nullcolor, &nullcolor, 0, 0);
  XFreePixmap(dsp, lockc);
  XFreePixmap(dsp, lockm);

  // create & open the black window
  if (!(blackWindow = XCreateWindow(dsp, rootWindow, 
				    -2, -2, 
				    WidthOfScreen(screen)+10, // just in case
				    HeightOfScreen(screen)+10, // just in case
				    0,
				    CopyFromParent,
				    InputOutput, 
				    CopyFromParent,
				    CWBackPixel | CWOverrideRedirect,
				    &xswa))) {
    
    fprintf(stderr, "Cannot create window\n");
    exit(1);    
  }
  XMapRaised(dsp, blackWindow);  
  
  // grab the cursor
  if (XGrabPointer(dsp, blackWindow, True, (unsigned int) AllPointerEventMask,
		   GrabModeAsync, GrabModeAsync, None, myCursor, CurrentTime) != GrabSuccess) {
    
    fprintf(stderr, "Couldn't grab pointer\n");
    goto err_grabpointer;
    exit(1);
  }

  // grab the keyboard
  if (XGrabKeyboard(dsp, blackWindow, True, GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess) {
    
    fprintf(stderr, "Couldn't grab keyboard\n");
    goto err_grabkbd;
  } 

  // simple event loop
  endLoop=0;
  while(!endLoop) {
    
    XNextEvent(dsp, &evt);
  }

  // shutdown
  XUngrabKeyboard(dsp, CurrentTime);
 err_grabkbd:
  XUngrabPointer(dsp, CurrentTime);
 err_grabpointer:
  //  XUnmapWindow(dsp, myWindow);
  //  XDestroyWindow(dsp, myWindow);
 err_opendisplay:
  XCloseDisplay(dsp);
}

