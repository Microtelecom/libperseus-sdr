
ACLOCAL_AMFLAGS = -I build-aux

if OS_LINUX
# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3 @SIMD_FLAGS@ -DSVN_REVISION=\"@SVNREV@\"
AM_LDFLAGS =
AM_LIBS    = -lusb1.0 -ldl
endif

if OS_WINDOWS
#
# flags you want to pass to the C compiler & linker
# -mno-ms-bitfields needed in order to properly align fields in structures 
# http://stackoverflow.com/questions/7789668/why-would-the-size-of-a-packed-structure-be-different-on-linux-and-windows-when
#
AM_CFLAGS  = -Wall -O3 -DSVN_REVISION=\"@SVNREV@\" -mno-ms-bitfields -I../libusb-1.0.last/ -DPTW32_STATIC_LIB -D__CLEANUP_C   
AM_LDLIBS  = -ldl 
AM_LDFLAGS = -ldl
endif


bin_PROGRAMS = perseustest perseustest_dyn

perseustest_SOURCES = perseustest.c perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                      perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h  fpga_data.h

nodist_perseustest_SOURCES = fpga_data.c

perseustest_dyn_SOURCES = perseustest.c perseuserr.h
perseustest_dyn_LDADD   = -lperseus-sdr

lib_LTLIBRARIES = libperseus-sdr.la

libperseus_sdr_la_CFLAGS = $(AM_CFLAGS)
libperseus_sdr_la_SOURCES =  perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                             perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h fpga_data.h

nodist_libperseus_sdr_la_SOURCES = fpga_data.c

hdrdir = $(includedir)

hdr_HEADERS = perseus-sdr.h perseusfx2.h perseus-in.h perseuserr.h

dist_pkgdata_DATA = $(srcdir)/generate_fpga_code.sh $(srcdir)/perseus24v41_512.c $(udev_DATA)

#
# FPGA images
# have to be distributed in tarball but not installed
#
dist_check_DATA = $(srcdir)/perseus125k24v21.rbs  $(srcdir)/perseus192k24v31.rbs  $(srcdir)/perseus1m24v21.rbs  $(srcdir)/perseus250k24v21.rbs  $(srcdir)/perseus2m24v21.rbs  $(srcdir)/perseus48k24v31.rbs  $(srcdir)/perseus500k24v21.rbs  $(srcdir)/perseus95k24v31.rbs  $(srcdir)/perseus96k24v31.rbs  $(srcdir)/perseus1d6m24v21.rbs

#
# Generate source file for FPGA code
#
BUILT_SOURCES = fpga_data.c
CLEANFILES = fpga_data.c

fpga_data.c: $(srcdir)/generate_fpga_code.sh $(dist_pkgdata_DATA) $(dist_check_DATA)
	$(srcdir)/generate_fpga_code.sh $(srcdir)/ > $@


#
# UDEV rule file: to activate it do the following command:
#
# sudo groupadd perseususb
# sudo usermod -a -G perseususb `whoami`
# sudo udevadm control --reload-rules
#
udevrule_DATA = $(top_srcdir)/95-perseus.rules
udevruledir = $(bindir)
EXTRA_DIST = $(top_srcdir)/95-perseus.rules SVNREV README.Windows

install-data-hook:
	-cp $(bindir)/95-perseus.rules /etc/udev/rules.d/ ; groupadd perseususb ; echo $(INST_USER) ;usermod -a -G perseususb $(INST_USER) ; udevadm control --reload-rules

uninstall-hook:
	-rm /etc/udev/rules.d/95-perseus.rules ; groupdel perseususb ; udevadm control --reload-rules


distdir = $(PACKAGE)-$(VERSION).$(SVNREV)
