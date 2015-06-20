/*
  **********************************************************************
  *
  *     Date                 Author               Summary of changes
  *     ----                 ------               ------------------
  *     March 4, 2002        Scott Bucholtz       Applied Leonid Froenchenko's
  *                                               <lfroen@galileo.co.il> race
  *                                               condition patch
  **********************************************************************
  */

#include "XOverlay.h"
#include <X11/Xmu/Xmu.h>
#include <X11/Xlib.h>
#include "pthread.h"

static XtAppContext application;
static Widget toplevel;
static pthread_t x_thread;

static void resize_handle(Widget, XtPointer, XEvent*, Boolean*);
static void* x_loop(void*);

static int (*resize_fn)(geom_t);
static int (*switch_fn)();

void init_win(int* argc, char** argv, geom_t geom)
{
  Dimension x, y;
  int mask;

#ifdef __X_DVD_DEBUG
  print_info("doing init.\n"); fflush(stdout);
#endif
  toplevel = XtAppInitialize(&application, "Dxr2 Player",
			     NULL, 0,
			     argc, argv,
			     NULL,
			     NULL, 0);

#ifdef __X_DVD_DEBUG
  print_info("doing init.\n"); fflush(stdout);
#endif

  
  mask = ResizeRedirectMask;

  XtMakeResizeRequest(toplevel, geom.width, geom.height, &x, &y);
  XtRealizeWidget(toplevel);
  XMoveWindow( XtDisplay(toplevel), XtWindow(toplevel), geom.x, geom.y);
  XSetWindowBackground( XtDisplay(toplevel), XtWindow(toplevel), 0x0000ff );

  resize_fn=NULL;

  pthread_create(&x_thread, NULL, x_loop, NULL);
}

void* x_loop(void* arg)
{
  XEvent event;

  while (toplevel != NULL) /* forever */
    { 
      XtAppNextEvent(application, &event);

#ifdef __X_DVD_DEBUG
      print_info("Event type: %d\n", event.type);
#endif
      /* Xevent read off queue */
      XtDispatchEvent(&event);

      if(event.type == 22) { // Resize event
	resize_handle(toplevel, NULL, NULL, NULL);
      }
      else if(event.type == 18 || event.type == 19) { // minimize/restore
	if(switch_fn) (*switch_fn)();
      }
    }
}

void destwin()
{
  pthread_kill( x_thread, 9 );
  XtDestroyWidget(toplevel);
  toplevel = NULL;
  resize_fn = NULL;
}

void resize_handle(Widget w, XtPointer passed_data, XEvent* event, Boolean* bool)
{
  XWindowAttributes attrib;
  Position x, y, x1, y1;
  geom_t geom;

  XGetWindowAttributes( XtDisplay(w), XtWindow(w) , &attrib );
  attrib.y = 0; // y is reported incorrectly
  attrib.x = 0; // reset x just incase
  XtTranslateCoords(w, attrib.x, attrib.y, &x, &y);

  geom.x=x;
  geom.y=y;
  geom.width=attrib.width;
  geom.height=attrib.height;

  if(resize_fn) (*resize_fn)(geom);

  attrib.x = 0;
  attrib.y = 0;
  XtTranslateCoords(w, attrib.x, attrib.y, &x1, &y1);
}

int set_geom_fn( int (*fn)(geom_t))
{
  resize_fn = fn;
  return 1;
}

int set_switch_fn( int (*fn)() )
{
  switch_fn = fn;
  return 1;
}
