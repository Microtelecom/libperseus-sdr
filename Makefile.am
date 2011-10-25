# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3
AM_LDFLAGS =
AM_LIBS    = -lusb1.0 -ldl



bin_PROGRAMS = perseustest perseustest_dyn

perseustest_SOURCES = perseustest.c perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                      perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h  fpga_data.h

nodist_perseustest_SOURCES = fpga_data.c

perseustest_dyn_SOURCES = perseustest.c perseuserr.h
perseustest_dyn_LDADD   = -lperseus-sdr


lib_LTLIBRARIES = libperseus-sdr.la

libperseus_sdr_la_CFLAGS = $(AM_CFLAGS) -pthread
libperseus_sdr_la_SOURCES =  perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                             perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h fpga_data.h

nodist_libperseus_sdr_la_SOURCES = fpga_data.c

hdrdir = $(includedir)

hdr_HEADERS = perseus-sdr.h perseusfx2.h perseus-in.h perseuserr.h

noinst_DATA = $(srcdir)/perseus125k24v21.rbs  $(srcdir)/perseus192k24v31.rbs  $(srcdir)/perseus1m24v21.rbs  $(srcdir)/perseus250k24v21.rbs  $(srcdir)/perseus2m24v21.rbs  $(srcdir)/perseus48k24v31.rbs  $(srcdir)/perseus500k24v21.rbs  $(srcdir)/perseus95k24v31.rbs  $(srcdir)/perseus96k24v31.rbs 

dist_pkgdata_DATA = $(noinst_DATA) $(srcdir)/generate_fpga_code.sh $(srcdir)/perseus24v11_512.c

#
# Generate source file for FPGA code
#
BUILT_SOURCES = fpga_data.c
CLEANFILES = fpga_data.c

fpga_data.c: $(srcdir)/generate_fpga_code.sh $(dist_pkgdata_DATA)
	$(srcdir)/generate_fpga_code.sh $(srcdir)/ > $@


