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

OBJS=lowlevel.o module.o highlevel.o
CFLAGS=-DMODULE -D__KERNEL__ -O2 -I$(KERNINC) -I../sysinclude -D_LOOSE_KERNEL_NAMES -DMAKING_MODULES=1 $(SMP)
CC=gcc

all: anp82.o

anp82.o: $(OBJS)
	ld -i $(OBJS) -o anp82.o

clean:
	rm -f *.o *~ *#

.PHONY: all clean

