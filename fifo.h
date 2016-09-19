// ------------------------------------------------------------------------------
// FIFO command channel
// 
// Copyright (c) 2016 Andrea Montefusco / IW0HDV
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

// Creation date:	Sept 2016
// Version:			0.1
// Author: 			Andrea Montefusco IW0HDV
// ------------------------------------------------------------------------------

#ifndef _fifo_h
#define _fifo_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fifo_th {
	perseus_descr *descr;
	FILE *fd;
	pthread_t th;
	char fifo_name[256];
} FIFO_TH;

int		make_fifo (const char *fifo_name, perseus_descr *pd);
int		run_fifo ();
void	stop_fifo ();


#ifdef __cplusplus
}
#endif

#endif // _perseuserr_h
