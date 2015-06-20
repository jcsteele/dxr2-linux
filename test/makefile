CFLAGS=-I/usr/X11R6/include -O2 -I../sysinclude  ${SMP}
CC=gcc
LIBS=-lX11 -lXext -lXxf86vm -L/usr/X11R6/lib -lm


all: test

test: test.o css.o whitescreen.o
	$(CC) -o test test.o css.o whitescreen.o $(LIBS)

test.o: test.c
css.o: css.c css.h
whitescreen.o: whitescreen.c

clean:
	rm -f *.o *~ *# test core

.PHONY: all clean

