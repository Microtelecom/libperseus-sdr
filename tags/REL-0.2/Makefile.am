# flags you want to pass to the C compiler & linker
AM_CFLAGS  =  -Wall -O3
AM_LDFLAGS =
AM_LIBS    = -lusb1.0 -ldl



bin_PROGRAMS = perseustest perseustest_dyn

perseustest_SOURCES = perseustest.c perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                      perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h

perseustest_dyn_SOURCES = perseustest.c perseuserr.h
perseustest_dyn_LDADD   = -lperseus-sdr


lib_LTLIBRARIES = libperseus-sdr.la

libperseus_sdr_la_CFLAGS = $(AM_CFLAGS) -pthread
libperseus_sdr_la_SOURCES =  perseus-sdr.c perseuserr.c perseusfx2.c perseus-in.c \
                             perseuserr.h perseusfx2.h perseus-in.h perseus-sdr.h

hdrdir = $(includedir)

hdr_HEADERS = perseus-sdr.h perseusfx2.h perseus-in.h perseuserr.h


#
# FPGA microcode files
#

EXTRA_DIST = perseus125k24v21.rbs  perseus250k24v21.rbs  perseus500k24v21.rbs \
             perseus1m24v21.rbs    perseus2m24v21.rbs    perseus95k24v31.rbs


firmwaredir = $(libdir)/

firmware_DATA = perseus125k24v21.rbs  perseus250k24v21.rbs  perseus500k24v21.rbs \
                perseus1m24v21.rbs    perseus2m24v21.rbs    perseus95k24v31.rbs

