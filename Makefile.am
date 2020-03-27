
ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = . examples


BUILT_SOURCES = $(top_srcdir)/.version fpga_data.c

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

if OS_LINUX
# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3 @SIMD_FLAGS@ -DGIT_REVISION=\"@VERSION@\"
AM_LDFLAGS =
AM_LIBS    = -ldl
endif

if OS_MACOS
AM_CFLAGS  =  -Wall -O3 -mssse3 -DGIT_REVISION=\"@VERSION@\"
AM_LDFLAGS =
AM_LIBS    = -ldl
endif

if OS_WINDOWS
#
# flags you want to pass to the C compiler & linker
# -mno-ms-bitfields needed in order to properly align fields in structures
# http://stackoverflow.com/questions/7789668/why-would-the-size-of-a-packed-structure-be-different-on-linux-and-windows-when
#
AM_CFLAGS  = -Wall -O3 -DGIT_REVISION=\"@VERSION@\" -mno-ms-bitfields -I../libusb-1.0.last/ -DPTW32_STATIC_LIB -D__CLEANUP_C
AM_LDLIBS  = -ldl
AM_LDFLAGS = -ldl
endif


lib_LTLIBRARIES = libperseus-sdr.la

libperseus_sdr_la_CFLAGS = $(AM_CFLAGS)
libperseus_sdr_la_SOURCES =  perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                             perseusfx2.h perseus-in.h perseus-sdr.h fpga_data.h

nodist_libperseus_sdr_la_SOURCES = fpga_data.c

hdrdir = $(includedir)

#hdr_HEADERS = perseus-sdr.h perseusfx2.h perseus-in.h
hdr_HEADERS = perseus-sdr.h

# connvenience library
noinst_LIBRARIES = libperseus-sdr.a
libperseus_sdr_a_SOURCES = $(libperseus_sdr_la_SOURCES)
nodist_libperseus_sdr_a_SOURCES = fpga_data.c
libperseus_sdr_a_CFLAGS = $(AM_CFLAGS)

if OS_LINUX
#
# auxiliary data for udev daemon configuration
# have to be distributed in tarball _and_ installed
#
dist_pkgdata_DATA = $(udev_DATA)
#
# UDEV rule file: to activate it do the following command:
#
# sudo groupadd perseususb
# sudo usermod -a -G perseususb `whoami`
# sudo udevadm control --reload-rules
#
udevrule_DATA = $(top_srcdir)/95-perseus.rules
udevruledir = $(bindir)
EXTRA_DIST = $(top_srcdir)/95-perseus.rules README.Windows $(top_srcdir)/build-aux/git-version-gen fpga_data.c

install-data-hook:
	-cp $(bindir)/95-perseus.rules /etc/udev/rules.d/ ; groupadd perseususb ; echo $(INST_USER) ;usermod -a -G perseususb $(INST_USER) ; udevadm control --reload-rules

uninstall-hook:
	-rm /etc/udev/rules.d/95-perseus.rules ; groupdel perseususb ; udevadm control --reload-rules
endif

#
# Cypress boot loader firmware image
# have to be distributed in tarball but _not_ installed
# .RBS FPGA images are not distributed in tarball, the equivalent C code is (in fpga_data.c)
#
dist_check_DATA =	$(srcdir)/perseus24v41_512.c


#
# Generate source file for FPGA code
#
CLEANFILES = fpga_data.c

if OS_LINUX
fpga_data.c: $(dist_pkgdata_DATA)
	@[ -f fpga_data.c ] || $(srcdir)/generate_fpga_code.sh $(srcdir)/ > $@
else
fpga_data.c:
	@[ -f fpga_data.c ] || $(srcdir)/generate_fpga_code.sh $(srcdir)/ > $@
endif

distdir = $(PACKAGE)-$(VERSION)

## Install the generated pkg-config file (.pc) into the expected location for
## architecture-dependent package configuration information.  Occasionally,
## pkg-config files are also used for architecture-independent data packages,
## in which case the correct install location would be $(datadir)/pkgconfig.
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libperseus-sdr.pc
