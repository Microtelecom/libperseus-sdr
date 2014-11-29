// ------------------------------------------------------------------------------
// Input data transfers queue implementation
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

#include <sys/time.h>     // for gettimeofday
#include "perseus-in.h"
#include "perseusfx2.h"
#include "perseuserr.h"

// The timeout for max bulk transfer size (16320 Bytes) at minimum sample rate (48 KS/s)
// is 16320 / (48*6) = 0.0566 ms
// We set it at 80 ms so that input transfers (should) never timeout
#define PERSEUS_INPUT_TIMEOUT	80

static void LIBUSB_CALL input_queue_callback(struct libusb_transfer *transfer);

int  perseus_input_queue_create(
					perseus_input_queue *queue, 
					int queue_size, 
					libusb_device_handle *handle,
					int transfer_buf_size, 
					perseus_input_callback callback_fn, 
					void *callback_extra)
{
	int k,j;
	perseus_input_transfer *transfer_queue;
	char *transfer_buf;
	
	queue->size 		  	 = queue_size;
	queue->transfer_buf_size = transfer_buf_size;
	queue->idx_expected   	 = 0;
	queue->cancelling 		 = FALSE;
	queue->completed 	  	 = FALSE;
	queue->bytes_received 	 = 0;
	queue->callback_fn		 = callback_fn;
	queue->callback_extra	 = callback_extra;
	queue->timeout			 = queue_size*PERSEUS_INPUT_TIMEOUT;


	// allocate a perseus_input_transfer array of given size
	transfer_queue = queue->transfer_queue	= (perseus_input_transfer*)malloc(queue_size*sizeof(perseus_input_transfer));
	if (!queue->transfer_queue) 
		return errorset(PERSEUS_NOMEM,"can't allocate input transfer queue");

	// allocate a buffer for queue_size transfers, each transfer_buf_size bytes wide 
	transfer_buf = queue->buf = (char*)malloc(queue_size*transfer_buf_size);
	if (!queue->buf) {
		free(queue->transfer_queue);
		return errorset(PERSEUS_NOMEM,"can't allocate datain buffer");
		}

	// allocate and fill the transfers
	for (k=0; k<queue_size; k++) {
		transfer_queue[k].queue		= queue;
		transfer_queue[k].idx 		= k;
//		transfer_queue[k].cancel	= FALSE;
		transfer_queue[k].cancelled	= FALSE;
		transfer_queue[k].transfer	= libusb_alloc_transfer(0);
		if (!transfer_queue[k].transfer) 
			goto create_cleanup;
		libusb_fill_bulk_transfer(transfer_queue[k].transfer, 
									handle,
									PERSEUS_EP_DATAIN,
									(unsigned char*)transfer_buf,
									transfer_buf_size,
									input_queue_callback,
									&transfer_queue[k],
									queue->timeout);
		transfer_buf += transfer_buf_size;
		}

	// submit the transfers
	for (k=0; k<queue_size; k++) 
		libusb_submit_transfer(transfer_queue[k].transfer);

	gettimeofday(&queue->start, NULL);

	return errornone(0);

create_cleanup:

	// free allocated transfers
	for (j=0; j<k; j++) 
		if (transfer_queue[j].transfer)
			libusb_free_transfer(transfer_queue[k].transfer);

	// and data buffers
	free(queue->buf);
	free(queue->transfer_queue);

	queue->transfer_queue = NULL;
	queue->buf = NULL;

	return errorset(PERSEUS_IOERROR,"can't allocate transfer %d", k);

}

int  perseus_input_queue_cancel(perseus_input_queue *queue)
{
	perseus_input_transfer *transfer_queue = queue->transfer_queue;
	int k;

	if (transfer_queue==NULL)
		return errornone(0);

	gettimeofday(&queue->stop, NULL);

	queue->callback_fn		 = NULL;	// should not be really needed

	if (perseus_input_queue_check_completion(queue)==TRUE)
		return errornone(0);

	queue->cancelling = TRUE;

	for (k=0; k<queue->size; k++) 
		libusb_cancel_transfer(transfer_queue[k].transfer);

	return errornone(0);
}

int  perseus_input_queue_check_completion(perseus_input_queue *queue)
{
	perseus_input_transfer *transfer_queue = queue->transfer_queue;

	int k;

	for (k=0; k<queue->size; k++) 
		if (transfer_queue[k].cancelled==FALSE)
			return FALSE;

	queue->completed = TRUE;

	dbgprintf(3,"all transfers cancelled successfully"); 

	return TRUE;
}

int  perseus_input_queue_completed(perseus_input_queue *queue)
{
	return queue->completed;
}


int  perseus_input_queue_free(perseus_input_queue *queue)
{
	perseus_input_transfer *transfer_queue = queue->transfer_queue;

	int k;

	if (transfer_queue==NULL)
		return errornone(0);

	for (k=0; k<queue->size; k++) 
		libusb_free_transfer(transfer_queue[k].transfer);

	free(queue->buf);
	free(queue->transfer_queue);

	queue->buf = NULL;
	queue->transfer_queue = NULL;

	return errornone(0);
}

static void LIBUSB_CALL input_queue_callback(struct libusb_transfer *transfer)
{
	perseus_input_transfer *input_transfer = (perseus_input_transfer *)transfer->user_data;
	perseus_input_queue_ptr input_queue    = input_transfer->queue;

	if (input_queue->cancelling==TRUE) {
		input_transfer->cancelled = TRUE;
		dbgprintf(3,"input transfer(%d) cancelled", input_transfer->idx); 
		perseus_input_queue_check_completion(input_queue);
		return;
		}

	switch (transfer->status) {
		case LIBUSB_TRANSFER_COMPLETED:
			// Transfer completed without error.
			input_queue->bytes_received += transfer->actual_length;

			if (input_transfer->idx==input_queue->idx_expected) 
				if (transfer->actual_length == transfer->length) {
					if (input_queue->callback_fn)
						input_queue->callback_fn(transfer->buffer, transfer->length, input_queue->callback_extra);
					}
				else {
					dbgprintf(0,"input transfer actual length< expected length. actual=%d expected=%d",
								transfer->actual_length, transfer->length);
					}
			else {
					dbgprintf(0,"input transfer out of sequence. received=%d expected=%d",
							input_transfer->idx,input_queue->idx_expected); 
				}
			break;
		case LIBUSB_TRANSFER_TIMED_OUT:
			// Transfer timed out.
			dbgprintf(1,"input transfer(%d) timeout. actual length=%d", input_transfer->idx,transfer->actual_length);
			break;
		case LIBUSB_TRANSFER_ERROR:
			// Transfer failed.
			dbgprintf(0,"input transfer failed"); 
			input_transfer->cancelled = TRUE;
			perseus_input_queue_check_completion(input_queue);
			return;
		case LIBUSB_TRANSFER_CANCELLED:
			// Transfer was cancelled.
			input_transfer->cancelled = TRUE;
			dbgprintf(3,"input transfer(%d) cancelled", input_transfer->idx); 
			perseus_input_queue_check_completion(input_queue);
			return;
		case LIBUSB_TRANSFER_STALL: 	
			// For bulk/interrupt endpoints: halt condition detected (endpoint stalled)
			dbgprintf(0,"input transfer(%d) stall", input_transfer->idx); 
			input_transfer->cancelled = TRUE;
			perseus_input_queue_check_completion(input_queue);
			return;
		case LIBUSB_TRANSFER_NO_DEVICE:
			// Device was disconnected.
			dbgprintf(0,"input transfer(%d) error: device disconnected", input_transfer->idx); 
			input_transfer->cancelled = TRUE;
			perseus_input_queue_check_completion(input_queue);
			return;
		case LIBUSB_TRANSFER_OVERFLOW:
			// Device sent more data than requested. 
			dbgprintf(0,"input transfer(%d) overflow", input_transfer->idx); 
			input_transfer->cancelled = TRUE;
			perseus_input_queue_check_completion(input_queue);
			return;
		default:
			dbgprintf(0,"input transfer unexpected status"); 
			input_transfer->cancelled = TRUE;
			perseus_input_queue_check_completion(input_queue);
			return;
		}

	// update our transfer sequence sanity check index
	input_queue->idx_expected = (input_transfer->idx+1)%input_queue->size;

	// resubmit transfer
	libusb_submit_transfer(transfer);
}

