# taken from NVIDIA driver. thanks to Gerald Henriksen!
# -----------------------------------------------------

KERNDIR=/lib/modules/$(shell uname -r)

# this is slightly more brain-dead, but works..
ifeq ($(shell if test -d $(KERNDIR)/build; then echo yes; fi),yes)
KERNINC=$(KERNDIR)/build/include
else
KERNINC=/usr/src/linux/include
endif

# -----------------------------------------------------
# END taken from NVIDIA

CSSLIB=libdxr2css.so.0.0.1

CFLAGS=-D__DVD_DEBUG -D_LARGEFILE64_SOURCE -D_REENTRANT -I/usr/local/include -I/usr/X11R6/include -g -O2 -I../sysinclude -I$(KERNINC) -DNEED_SYS_TYPES_H ${SMP}
CC=gcc
LIBS=-lpthread -lncurses -ldvdread -ldl
X_LIBS=-L/usr/X11R6/lib -lXmu -lX11 -lXext -lXxf86vm

OBJS=player.o config.o config-files.o dxr2-api.o whitescreen.o overlay.o multifile.o dvd_controls.o

X_OBJS= X11-overlay.o XOverlay.o

TESTS=xtest

__PLAYER=1

all: players driveauth wrappers

CSSLIBOBJ=css.o css_wrap.o

$(CSSLIB): $(CSSLIBOBJ)
	gcc -fPIC -Wl,-soname -Wl,$(CSSLIB) -o $(CSSLIB) $(CSSLIBOBJ) -shared
	rm -f libdxr2css.so.0
	rm -f libdxr2css.so
	ln -s $(CSSLIB) libdxr2css.so.0
	ln -s libdxr2css.so.0 libdxr2css.so

css.o: css.c css.h
	$(CC) $(CFLAGS) -fPIC -c -o css.o css.c

css_wrap.o: css_wrap.c css.h
	$(CC) $(CFLAGS) -fPIC -c -o css_wrap.o css_wrap.c

players: dvdplay dvdplay-curses

dvdplay: $(OBJS) text-interface.o $(X_OBJS) $(CSSLIB)
	$(CC) -o dvdplay $(OBJS) text-interface.o $(X_OBJS) $(LIBS) $(X_LIBS) -L. -ldxr2css

dvdplay-curses: $(OBJS) curses-interface.o $(X_OBJS) $(CSSLIB)
	$(CC) -o dvdplay-curses $(OBJS) curses-interface.o $(X_OBJS) $(LIBS) $(X_LIBS) -L. -ldxr2css

driveauth: driveauth.o config-files.o css.o text-interface.o dxr2-api.o
	$(CC) -o driveauth driveauth.o css.o config-files.o text-interface.o dxr2-api.o

driveauth.o: driveauth.c css.h player.h dvd_controls.h dxr2-api.h

config-files.o: config-files.c config-files.h interface.h

dxr2-api.o: dxr2-api.c dxr2-api.h

player.o: player.c player.h dvd_controls.h css.h config.h interface.h dxr2-api.h

overlay.o: overlay.h overlay.c

dvd_controls.o: dvd_controls.h dvd_controls.c

config.o: config.c config.h player.h dvd_controls.h interface.h dxr2-api.h

multifile.o: multifile.c multifile.h player.h dvd_controls.h interface.h dxr2-api.h

text-interface.o: text-interface.c interface.h player.h dvd_controls.h dxr2-api.h

curses-interface.o: curses-interface.c interface.h player.h dvd_controls.h dxr2-api.h

wrappers: dvd undvd dvd-sum

dvd: dvd.c
	$(CC) $(CFLAGS) -Wall -o dvd dvd.c

undvd: undvd.c
	$(CC) -Wall -o undvd undvd.c

dvd-sum: dvd-sum.c
	$(CC) -Wall -o dvd-sum dvd-sum.c

X11-overlay.o: overlay.h X11-overlay.c player.h dvd_controls.h

XOverlay.o: XOverlay.c XOverlay.h

xtest.o: xtest.c XOverlay.h

xtest: xtest.o XOverlay.o
	-g++ -o xtest xtest.o XOverlay.o text-interface.o -L/usr/X11R6/lib -lXmu -lX11 -lpthread

clean:
	rm -f dvd undvd dvd-sum ifo2 driveauth dvdplay-curses dvdplay inttest libdxr2css.so* *.o *~ *#

.PHONY: all clean
