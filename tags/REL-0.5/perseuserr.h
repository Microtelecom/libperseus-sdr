// ------------------------------------------------------------------------------
// Error codes and macros
// 
// Copyright (c) 2010 Nicolangelo Palermo / IV3NWV 
// This file is part of the Perseus SDR Library
//
// The Perseus SDR Library is free software; you can redistribute 
// it and/or modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either version 
// 2.1 of the License, or (at your option) any later version.

// The Perseus SDR Library is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
// See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with the Perseus SDR Library; 
// if not, see <http://www.gnu.org/licenses/>.

// Creation date:	10 Jan 2010
// Version:			0.1
// Author: 			Nicolangelo Palermo / IV3NWV 
//                  (nicopal at microtelecom dot it)
// ------------------------------------------------------------------------------

#ifndef _perseuserr_h
#define _perseuserr_h

#define PERSEUS_NOERROR			 0
#define PERSEUS_INVALIDDEV		-1
#define PERSEUS_NULLDESCR		-2
#define PERSEUS_ALREADYOPEN		-3
#define PERSEUS_LIBUSBERR		-4
#define PERSEUS_DEVNOTOPEN		-5
#define PERSEUS_DEVCONF			-6
#define PERSEUS_DEVCLAIMINT		-7
#define PERSEUS_DEVALTINT		-8
#define PERSEUS_FNNOTAVAIL		-9
#define PERSEUS_DEVNOTFOUND		-10
#define PERSEUS_EEPROMREAD		-11
#define PERSEUS_FILENOTFOUND	-12
#define PERSEUS_IOERROR			-13
#define PERSEUS_INVALIDHEXREC	-14
#define PERSEUS_INVALIDEXTREC	-15
#define PERSEUS_FWNOTLOADED		-16
#define PERSEUS_FPGACFGERROR	-17
#define PERSEUS_FPGANOTCFGD	    -18
#define PERSEUS_ASYNCSTARTED	-19
#define PERSEUS_NOMEM			-20
#define PERSEUS_CANTCREAT		-21
#define PERSEUS_ERRPARAM		-22
#define PERSEUS_MUTEXIN			-23
#define PERSEUS_BUFFERSIZE		-24
#define PERSEUS_ATTERROR		-25

#define dbgprintf(level, format, args...) \
	{ \
      if (perseus_dbg_level >= level) {\
           fprintf(stderr, "perseus: "); \
           fprintf(stderr, format, ## args); \
           fprintf(stderr, "\n"); \
		} \
	}
#define errorset(x, format, args...) \
    ( \
  	  snprintf(perseus_error_str, sizeof(perseus_error_str) - 1, format, ## args), \
	  ((perseus_dbg_level >= 1)? fprintf(stderr, "perseus error: %s\n", perseus_error_str):0),\
	  (perseus_error = x)\
	)

#define errornone(x) (perseus_error=0, x)

extern int  perseus_dbg_level;
extern char perseus_error_str[1024];
extern int  perseus_error;

#ifdef __cplusplus
extern "C" {
#endif

char *perseus_errorstr(void);

#ifdef __cplusplus
}
#endif

#endif // _perseuserr_h

