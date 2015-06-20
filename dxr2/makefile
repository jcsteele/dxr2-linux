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

OBJS=	_anp82.o _bt865.o _pcm1723.o _tc6807af.o _vxp524.o _zivaDS.o asic.o \
	module.o eeprom.o highlevel.o ioctls.o deferred.o
CFLAGS=-DMODULE -D__KERNEL__ -O2 -I$(KERNINC) -I../sysinclude -D_LOOSE_KERNEL_NAMES -DMAKING_MODULES=1 $(SMP)
CC=gcc

all: dxr2.o

dxr2.o: $(OBJS)
	  ld -i $(OBJS) -o dxr2.o

clean:
	rm -f *.o *~ *#

.PHONY: 	all

