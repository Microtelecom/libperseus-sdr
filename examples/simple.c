/*
 *    gcc -Wall simple.c -lperseus-sdr -o simple
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "perseus-sdr.h"

// The function that will be called by the perseus library when a 
// data buffer from the Perseus input data endpoint is available

typedef union {
	struct {
		int32_t	i;
		int32_t	q;
		} __attribute__((__packed__)) iq;
	struct {
		uint8_t		i1;
		uint8_t		i2;
		uint8_t		i3;
		uint8_t		i4;
		uint8_t		q1;
		uint8_t		q2;
		uint8_t		q3;
		uint8_t		q4;
		} __attribute__((__packed__)) ;
} iq_sample;
//
// callback that writes in the output stream I-Q values as 32 bits
// integers
//
int user_data_callback_c_u(void *buf, int buf_size, void *extra)
{
	// The buffer received contains 24-bit IQ samples (6 bytes per sample)
	// Here as a demonstration we save the received IQ samples as 32 bit 
	// (msb aligned) integer IQ samples.

	// At the maximum sample rate (2 MS/s) the hard disk should be capable
	// of writing data at a rate of at least 16 MB/s (almost 1 GB/min!)

	uint8_t	*samplebuf 	= (uint8_t*)buf;
	FILE *fout 			= (FILE*)extra;
	int nSamples 		= buf_size/6;
	int k;
	iq_sample s;
	
	s.i1 = s.q1 = 0;

	for (k=0;k<nSamples;k++) {
		s.i2 = *samplebuf++;
		s.i3 = *samplebuf++;
		s.i4 = *samplebuf++;
		s.q2 = *samplebuf++;
		s.q3 = *samplebuf++;
		s.q4 = *samplebuf++;

		fwrite(&s.iq, 1, sizeof(iq_sample), fout);
	}
    return 0;
}


int main(int argc, char **argv)
{
	int k, num_perseus;
	perseus_descr *descr;
	FILE *fout = stdout;
	char out_file_name[128] = "perseusdata.bin";	
	
	// Check how many Perseus receivers are connected to the system
	num_perseus = perseus_init();

	if (num_perseus==0) {
		fprintf(stderr, "No Perseus receivers detected\n");
		perseus_exit();
		return 0;
	}
	fprintf(stderr, "%d Perseus receiver(s) found\n",num_perseus);

	// Open the first one...
	if ((descr=perseus_open(0))==NULL) {
		fprintf(stderr, "error: %s\n", perseus_errorstr());
		return 255;
	}

	// Download the standard firmware to the unit
	fprintf(stderr, "Downloading firmware...\n");
	if (perseus_firmware_download(descr,NULL)<0) {
		fprintf(stderr, "firmware download error: %s", perseus_errorstr());
		return 255;
	}
	// Configure the receiver for 96 kHz bw operations
	fprintf(stderr, "Configuring FPGA...\n");
	if (perseus_set_sampling_rate(descr, 96000) < 0) {  // specify the sampling rate value in Samples/second
		fprintf(stderr, "fpga configuration error: %s\n", perseus_errorstr());
		goto main_cleanup;
	}

	// set rx frequency to 7.050 MHz
	perseus_set_ddc_center_freq(descr, 7050000, 0);

	// We open a file for binary output.
	// IQ samples will be saved in binary format to this file
	fout = fopen(out_file_name,"wb");
	if (!fout) {
		fprintf(stderr, "Can't open output file\n");
		perseus_close(descr);
		perseus_exit();
	}
	
	// We start the acquisition passing our callback and its params 
	// (in this case the fout file handle we just open)	
	fprintf(stderr, "Starting async data acquisition... \n");
	if (perseus_start_async_input(descr, 6*1024, user_data_callback_c_u, fout)<0) {
		fprintf(stderr, "start async input error: %s\n", perseus_errorstr());
		goto main_cleanup;
	}
	
	fprintf(stderr, "Collecting input samples... ");

	// We wait a 10 s time interval.
	// The user data callback we supplied to perseus_start_async_input 
	// is being called meanwhile.
	for (k=0; k < 10; k++) {
		fprintf(stderr, ".");
		sleep(1);
	}

	fprintf(stderr, "\ndone\n");
	
	// We stop the acquisition...
	fprintf(stderr, "Stopping async data acquisition...\n");
	perseus_stop_async_input(descr);

	// We can safely close the output file handle here. Acquisition has stopped.
	fclose(fout);

main_cleanup:

	// And we can finally quit the test application

	fprintf(stderr, "Quitting...\n");
	perseus_close(descr);
	perseus_exit();

	fprintf(stderr, "Bye\n");

	return 1;
}


