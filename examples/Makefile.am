
ACLOCAL_AMFLAGS = -I build-aux

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

if OS_LINUX
# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3 @SIMD_FLAGS@ -DGIT_REVISION=\"@VERSION@\" -I$(top_srcdir)/
AM_LDFLAGS =
AM_LIBS    = -ldl -L$(top_srcdir)/
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


bin_PROGRAMS = perseustest_dyn perseustest
check_PROGRAMS = simple

perseustest_SOURCES = perseustest.c fifo.c fifo.h $(top_srcdir)/perseus-sdr.h
perseustest_LDADD   = ../libperseus-sdr.a

perseustest_dyn_SOURCES = perseustest.c fifo.c fifo.h $(top_srcdir)/perseus-sdr.h
perseustest_dyn_LDADD   = ../libperseus-sdr.la

simple_SOURCES = simple.c $(top_srcdir)/perseus-sdr.h
simple_LDADD   = ../libperseus-sdr.la
