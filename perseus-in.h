// ------------------------------------------------------------------------------
// Define the usb input data transfers queue
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

#ifndef	_perseusin_h
#define	_perseusin_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "perseuserr.h"

#include <libusb-1.0/libusb.h>
#include <time.h>

typedef int (*perseus_input_callback)(void *buf, int buf_size, void *extra);

typedef struct perseus_input_queue_ds *perseus_input_queue_ptr;

typedef struct {
	int						 idx;
	perseus_input_queue_ptr  queue;
	struct libusb_transfer 	*transfer;
	int			 			 cancelled;
} perseus_input_transfer;

typedef struct perseus_input_queue_ds {
	int						size;
	perseus_input_transfer	*transfer_queue;
	volatile int 			cancelling;
	char					*buf;
	int						transfer_buf_size;
	int						timeout;
	perseus_input_callback 	callback_fn;
	void					*callback_extra;
	int						idx_expected;
	volatile int			completed;
	struct timeval 			start;
	struct timeval			stop;
	unsigned long int		bytes_received;
} perseus_input_queue;

#ifdef __cplusplus
extern "C" {
#endif

int  perseus_input_queue_create(
					perseus_input_queue *queue, 
					int queue_size, 
					libusb_device_handle *handle,
					int transfer_buf_size, 
					perseus_input_callback callback_fn, 
					void *callback_extra);

int  perseus_input_queue_cancel(perseus_input_queue *queue);
int  perseus_input_queue_free(perseus_input_queue *queue);
int  perseus_input_queue_check_completion(perseus_input_queue *queue);
int  perseus_input_queue_completed(perseus_input_queue *queue);

#ifdef __cplusplus
}
#endif

#if defined _WIN32
#define	sleep(x) Sleep((x)*1000)
#endif

#endif // _perseus_in

