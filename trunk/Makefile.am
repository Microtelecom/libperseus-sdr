# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3
AM_LDFLAGS =
AM_LIBS    = -lusb1.0 -ldl



bin_PROGRAMS = perseustest perseustest_dyn

perseustest_SOURCES = perseustest.c perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c fpga_data.c \
                      perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h fpga_data.h

perseustest_dyn_SOURCES = perseustest.c perseuserr.h
perseustest_dyn_LDADD   = -lperseus-sdr


lib_LTLIBRARIES = libperseus-sdr.la

libperseus_sdr_la_CFLAGS = $(AM_CFLAGS) -pthread
libperseus_sdr_la_SOURCES =  perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c fpga_data.c \
                             perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h fpga_data.h

hdrdir = $(includedir)

hdr_HEADERS = perseus-sdr.h perseusfx2.h perseus-in.h perseuserr.h


#
# Generate source file for FPGA code
#
fpga_data.c: $(srcdir)/generate_fpga_code.sh $(srcdir)/*.rbs 
	$(srcdir)/generate_fpga_code.sh > $@

CLEANFILES = generated.cpp

