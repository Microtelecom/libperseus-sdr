// perseus-sdr library test
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#if defined _WIN32
#define	sleep(x) Sleep((x)*1000)
#else
#include <unistd.h>
#endif
#include "perseus-sdr.h"
#include "fifo.h"

#if defined GIT_REVISION
const char *git_revision = GIT_REVISION;
#else
const char *git_revision ="GIT_REVISION_UNKNOWN";
#endif

/* TODO:

- Verify that control functions are thread safe (no mutex used yet)

*/


void print_usage ()
{
	fprintf (stderr, "Usage:\n");
	fprintf (stderr, "-s ............ sample rate (");

	{

		int buf[BUFSIZ];

		if (perseus_get_sampling_rates (0, buf, sizeof(buf)/sizeof(buf[0])) < 0) {
			printf("get sampling rates error: %s\n", perseus_errorstr());
		} else {
			int i = 0;
			while (buf[i]) {
				fprintf(stderr, " %d", buf[i]);
				i++;
			}
		}
		fprintf (stderr, ")\n");
	}
	fprintf (stderr, "-n ............ number of buffers\n");
	fprintf (stderr, "-b ............ buffer size in bytes\n");
	fprintf (stderr, "-d ............ debug level (0..9, -1 no debug)\n");
	fprintf (stderr, "-a ............ don't test attenuators\n");
	fprintf (stderr, "-t ............ acquisition test duration in seconds (default 10, 0 infinite)\n");
	fprintf (stderr, "-o ............ file name, '-' standard output (default perseusdata)\n");
	fprintf (stderr, "-f ............ frequency value in Hz, default 7.050 MHz\n");
	fprintf (stderr, "-p ............ I/Q samples emitted as floating "
					 "point instead of 24 bits unsigned int.\n");
	fprintf (stderr, "-F ............ FIFO command channel name\n");
	fprintf (stderr, "-u ............ attenuator value [-10 -20 -30 db]\n");
	fprintf (stderr, "-h ............ this help\n");
}

// The function that will be called by the perseus library when a 
// data buffer from the Perseus input data endpoint is available
static int user_data_callback_c_u(void *buf, int buf_size, void *extra);
static int user_data_callback_c_f(void *buf, int buf_size, void *extra);

int main(int argc, char **argv)
{
	int k, num_perseus;
	perseus_descr *descr;
	eeprom_prodid prodid;
	FILE *fout = stdout;
    int i;
	int sr = 95000;
	int nb = 6;
	int bs = 1024;
	int dbg_lvl = 3;
	int no_ta = 0;
	int test_time = 10;
	char out_file_name[128] = "perseusdata";
	long freq = 7000000;
	char fifo_name[128] = "";
	int atten_db = - 30;
	int fp_output = 0;

	for (i=0; i < argc; ++i) {
		dbgprintf (3,"%d: %s\n", i, argv[i]);
		if (!strcmp(argv[i],"-s") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &sr) == 1) ;
			else sr = 95000;
		}
		if (!strcmp(argv[i],"-n") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &nb) == 1) ;
			else nb = 6;
		}
		if (!strcmp(argv[i],"-b") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &bs) == 1) ; else bs = 1024;
		}
		if (!strcmp(argv[i],"-d") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &dbg_lvl) == 1) ; else dbg_lvl = 3;
		}
		if (!strcmp(argv[i],"-a")) {
			no_ta = 1;
		}
		if (!strcmp(argv[i],"-t") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &test_time) == 1) ;
			else test_time = 10;
		}
		if (!strcmp(argv[i],"-o") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%s", out_file_name) == 1) ;
			else strcpy(out_file_name, "perseusdata");
		}
		if (!strcmp(argv[i],"-f") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%ld", &freq) == 1) ;
			else freq = 7000000;
		}
		if (!strcmp(argv[i],"-F") && (i+1)<argc) {
		    ++i;
			sscanf(argv[i], "%s", fifo_name);
		}
		if (!strcmp(argv[i],"-u") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &atten_db) == 1) ; else atten_db = -30;
			fprintf (stderr, "**************** Attenuator: %d\n", atten_db);
		}
		if (!strcmp(argv[i],"-h")) {
		    print_usage ();
			exit (255);
		}
		if (!strcmp(argv[i],"-p")) {
			fp_output = 1;
		}		
	}
	
	// Set debug info dumped to stderr to the maximum verbose level
	perseus_set_debug(dbg_lvl);
	
    fprintf (stderr, "Revision: %s\n", git_revision);
	fprintf (stderr, "SAMPLE RATE: %d\n", sr);
	fprintf (stderr, "NBUF: %d BUF SIZE: %d TOTAL BUFFER LENGTH: %d\n", nb, bs, nb*bs);

	// Check how many Perseus receivers are connected to the system
	num_perseus = perseus_init();
	fprintf(stderr, "%d Perseus receivers found\n",num_perseus);

	if (num_perseus==0) {
		fprintf(stderr, "No Perseus receivers detected\n");
		perseus_exit();
		return 0;
	}

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
	// Dump some information about the receiver (S/N and HW rev)
	if (perseus_is_preserie(descr, 0) ==  PERSEUS_SNNOTAVAILABLE)
		fprintf(stderr, "The device is a preserie unit");
	else
		if (perseus_get_product_id(descr,&prodid)<0) 
			fprintf(stderr, "get product id error: %s", perseus_errorstr());
		else
			fprintf(stderr, "Receiver S/N: %05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW Release:%hd.%hd\n",
					(uint16_t) prodid.sn, 
					(uint16_t) prodid.signature[5],
					(uint16_t) prodid.signature[4],
					(uint16_t) prodid.signature[3],
					(uint16_t) prodid.signature[2],
					(uint16_t) prodid.signature[1],
					(uint16_t) prodid.signature[0],
					(uint16_t) prodid.hwrel,
					(uint16_t) prodid.hwver);

    // Printing all sampling rates available .....
    {
        int buf[BUFSIZ];

        if (perseus_get_sampling_rates (descr, buf, sizeof(buf)/sizeof(buf[0])) < 0) {
			fprintf(stderr, "get sampling rates error: %s\n", perseus_errorstr());
			goto main_cleanup;
        } else {
            int i = 0;
            while (buf[i]) {
                fprintf(stderr, "#%d: sample rate: %d\n", i, buf[i]);
                i++;
            }
        }
    }

	// Configure the receiver for 2 MS/s operations
	fprintf(stderr, "Configuring FPGA...\n");
	if (perseus_set_sampling_rate(descr, sr) < 0) {  // specify the sampling rate value in Samples/second
	//if (perseus_set_sampling_rate_n(descr, 0)<0)        // specify the sampling rate value as ordinal in the vector
		fprintf(stderr, "fpga configuration error: %s\n", perseus_errorstr());
		goto main_cleanup;
	}


    // Printing all attenuator values available .....
    {
        int buf[BUFSIZ];

        if (perseus_get_attenuator_values (descr, buf, sizeof(buf)/sizeof(buf[0])) < 0) {
			fprintf(stderr, "get attenuator values error: %s\n", perseus_errorstr());
        } else {
            int i = 0;
            while (buf[i] != -1) {
                fprintf(stderr, "#%d: att val in dB: %d\n", i, buf[i]);
                i++;
            }
        }
    }

	// Cycle attenuator leds on the receiver front panel
	// just to see if they indicate what they shoud
//  perseus_set_attenuator(descr, PERSEUS_ATT_0DB);
//  sleep(1);
//  perseus_set_attenuator(descr, PERSEUS_ATT_10DB);
//  sleep(1);
//  perseus_set_attenuator(descr, PERSEUS_ATT_20DB);
//  sleep(1);
//  perseus_set_attenuator(descr, PERSEUS_ATT_30DB);
//  sleep(1);
//  perseus_set_attenuator(descr, PERSEUS_ATT_0DB);
//  sleep(1);
//
//   perseus_set_attenuator_in_db(descr, 0);
//   sleep(1);
//   perseus_set_attenuator_in_db(descr, 10);
//   sleep(1);
//   perseus_set_attenuator_in_db(descr, 20);
//   sleep(1);
//   perseus_set_attenuator_in_db(descr, 30);
//   sleep(1);
//   perseus_set_attenuator_in_db(descr, 0);
//   sleep(1);


	//
	// set attenuator
	//
	switch (atten_db) {
		case 0:
		case -10:
		case -20:
		case -30:
			perseus_set_attenuator_n(descr, (int)(atten_db / -10));
			break;
	}
	if (no_ta == 0) {
		perseus_set_attenuator_in_db(descr, 33); // Bad value !!!

		perseus_set_attenuator_n(descr, 0);
		sleep(1);
		perseus_set_attenuator_n(descr, 1);
		sleep(1);
		perseus_set_attenuator_n(descr, 2);
		sleep(1);
		perseus_set_attenuator_n(descr, 3);
		sleep(1);
		perseus_set_attenuator_n(descr, 0);
		sleep(1);
	}
	// Enable ADC Dither, Disable ADC Preamp
	perseus_set_adc(descr, true, false);

	// Do the same cycling test with the WB front panel led.
	// Enable preselection filters (WB_MODE Off)
	perseus_set_ddc_center_freq(descr, freq, 1);
	sleep(1);
	// Disable preselection filters (WB_MODE On)
	perseus_set_ddc_center_freq(descr, freq, 0);
	sleep(1);
	// Re-enable preselection filters (WB_MODE Off)
	perseus_set_ddc_center_freq(descr, freq, 1);

	// We open a file for binary output.
	// IQ samples will be saved in binary format to this file
	if (strcmp(out_file_name, "-")) {
		fout = fopen(out_file_name,"wb");
		if (!fout) {
			fprintf(stderr, "Can't open output file (%s)\n", strerror(errno));
			fout = 0;
		}
	}
	
	// We start the acquisition passing our callback and its params 
	// (in this case the fout file handle we just open)	
	fprintf(stderr, "Starting async data acquisition... \n");
	if (fp_output) {
		if (perseus_start_async_input(descr,nb*bs,user_data_callback_c_f,fout)<0) {
			fprintf(stderr, "start async input error: %s\n", perseus_errorstr());
			goto main_cleanup;
		}
	} else {
		if (perseus_start_async_input(descr,nb*bs,user_data_callback_c_u,fout)<0) {
			fprintf(stderr, "start async input error: %s\n", perseus_errorstr());
			goto main_cleanup;
		}
	}
	
	//
	// 20160919 AM FIFO command stream management 
	//
	if (strlen(fifo_name)) {
		make_fifo(fifo_name, descr);
		run_fifo ();
		fprintf (stderr, "Control FIFO created. [%s]\n", fifo_name);
	}
	
	fprintf(stderr, "Collecting input samples... ");

	// wait indefinitely in case test_time is 0
	// useful when we are not testing but acting as a source
	// for another command line utility
	if (test_time == 0) {
		while (!sleep(100)) ;
	} else {
		// We wait a 10 s time interval.
		// The user data callback we supplied to perseus_start_async_input
		// is being called meanwhile.
		for (k=0;k<test_time;k++) {
			fprintf(stderr, ".");
			sleep(1);
		}
	}
	fprintf(stderr, "\ndone\n");
	
	// We stop the acquisition...
	fprintf(stderr, "Stopping async data acquisition...\n");
	perseus_stop_async_input(descr);

	// We can safely close the output file handle here. Acquisition has stopped.
	if (fout) fclose(fout);

main_cleanup:

	if (strlen(fifo_name)) {
		stop_fifo();
	}

	// And we can finally quit the test application

	fprintf(stderr, "Quitting...\n");
	perseus_close(descr);
	perseus_exit();

	fprintf(stderr, "Bye\n");

	return 1;
}

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

		if (fout) fwrite(&s.iq, 1, sizeof(iq_sample), fout);
	}
    return 0;
}

//
// callback that writes in the output stream I-Q values as 32 bits
// floating point in -1.0 ... +1.0 range
//
int user_data_callback_c_f(void *buf, int buf_size, void *extra)
{
	// The buffer received contains 24-bit IQ samples (6 bytes per sample)
	// Here as a demonstration we save the received IQ samples as 32 bit 
	// (msb aligned) integer IQ samples.

	// At the maximum sample rate (2 MS/s) the hard disk should be capable
	// of writing data at a rate of at least 16 MB/s (almost 1 GB/min!)

	uint8_t	*samplebuf 	= (uint8_t*)buf;
	FILE *fout 		= (FILE*)extra;
	int nSamples 		= buf_size/6;
	int k;
	iq_sample s;

	// the 24 bit data is scaled to a 32bit value (so that the machine's
	// natural signed arithmetic will work), and then use a simple
	// ratio of the result with the maximum possible value
	// which is INT_MAX less 256 because of the vacant lower 8 bits
	for (k=0;k<nSamples;k++) {
		s.i1 = s.q1 = 0;
		s.i2 = *samplebuf++;
		s.i3 = *samplebuf++;
		s.i4 = *samplebuf++;
		s.q2 = *samplebuf++;
		s.q3 = *samplebuf++;
		s.q4 = *samplebuf++;

		float iq_f [2];
		// convert to float in [-1.0 - +1.0] range
		iq_f[0] = (float)(s.iq.i) / (INT_MAX - 256);
		iq_f[1] = (float)(s.iq.q) / (INT_MAX - 256);
		
		if (fout) fwrite(iq_f, 2, sizeof(float), fout);
	}
    return 0;
}
