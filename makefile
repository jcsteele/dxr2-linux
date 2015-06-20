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

SHELL=sh
MODS=anp82 bt865 pcm1723 tc6807af vxp524 zivaDS dxr2
DIRS=${MODS} test player
# this sets the location where the dxr2-headerfiles will be copied (e.g. they are needed by minidxr2)
DXR2HEADER=/usr/include/dxr2

#SMP=$(shell grep '^\#define CONFIG_SMP' /usr/include/linux/autoconf.h>/dev/null && echo -D__SMP__)
SMP=$(shell if [ -n "`uname -a|grep SMP`" ]; then echo -D__SMP__; else echo; fi  )


all:
	for i in ${DIRS}; do \
		( cd $$i && ${MAKE} SMP=${SMP}) || exit 1; \
	done

insert: install
	/sbin/modprobe dxr2


install: $(MODS)
	-@/sbin/modprobe -r dxr2
	@echo installing the headerfiles into $(DXR2HEADER)
	@echo
	@echo if you try to compile and install minidxr2 type ./configure --with-dxr2-headers=$(DXR2HEADER)
	@echo
	@mkdir -p $(DXR2HEADER)
	@install sysinclude/*.h \
		$(DXR2HEADER)
	@mkdir -p /lib/modules/`uname -r`/kernel/drivers/video/dxr2
	@for module in $^ ;\
	do \
                install -m 0644 $$module/$$module.o /lib/modules/`uname -r`/kernel/drivers/video/dxr2/;\
	done
	@/sbin/depmod -a
	@install player/dvdplay* /usr/local/bin
	@install player/driveauth /usr/local/bin
	@install player/dvd-sum /usr/local/bin
	@install -m 4755 player/*dvd /usr/local/bin
	@echo installing libdxr2css library
	@install player/libdxr2css.so.0.0.1 /usr/local/lib
	@ln -sf /usr/local/lib/libdxr2css.so.0.0.1 /usr/local/lib/libdxr2css.so.0
	@ln -sf /usr/local/lib/libdxr2css.so.0 /usr/local/lib/libdxr2css.so
	@ldconfig
	@echo Installed to running kernel

remove:
	/sbin/modprobe -r dxr2

clean:
	for i in ${DIRS}; do ( cd $$i && ${MAKE} clean ) || exit 1; done
