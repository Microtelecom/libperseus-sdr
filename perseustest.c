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
#if defined _WIN32
#define	sleep(x) Sleep((x)*1000)
#else
#include <unistd.h>
#endif
#include "perseus-sdr.h"

#if defined SVN_REVISION
const char *svn_revision = SVN_REVISION;
#else
const char *svn_revision ="UNKNOWN";
#endif

/* TODO:

- Verify that control functions are thread safe (no mutex used yet)

*/

// The function that will be called by the perseus library when a 
// data buffer from the Perseus input data endpoint is available
static int user_data_callback(void *buf, int buf_size, void *extra);

int main(int argc, char **argv)
{
	int k, num_perseus;
	perseus_descr *descr;
	eeprom_prodid prodid;
	FILE *fout;
    int i;
	int sr = 95000;
	int nb = 4;
	int bs = 1024;
	int dbg_lvl = 3;

	for (i=0; i < argc; ++i) {
		dbgprintf (3,"%d: %s\n", i, argv[i]);
		if (!strcmp(argv[i],"-s") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &sr) == 1) ; else sr = 95000;
		}
		if (!strcmp(argv[i],"-n") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &nb) == 1) ; else nb = 4;
		}
		if (!strcmp(argv[i],"-b") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &bs) == 1) ; else bs = 1024;
		}
		if (!strcmp(argv[i],"-d") && (i+1)<argc) {
		    ++i;
			if(sscanf(argv[i], "%d", &dbg_lvl) == 1) ; else dbg_lvl = 3;
		}
	}
	
	// Set debug info dumped to stderr to the maximum verbose level
	perseus_set_debug(dbg_lvl);
	
    printf ("Revision: %s\n", svn_revision);
	printf ("SAMPLE RATE: %d\n", sr);
	printf ("NBUF: %d BUF SIZE: %d TOTAL BUFFER LENGTH: %d\n", nb, bs, nb*bs);

	// Check how many Perseus receivers are connected to the system
	num_perseus = perseus_init();
	printf("%d Perseus receivers found\n",num_perseus);

	if (num_perseus==0) {
		printf("No Perseus receivers detected\n");
		perseus_exit();
		return 0;
	}

	// Open the first one...
	if ((descr=perseus_open(0))==NULL) {
		printf("error: %s\n", perseus_errorstr());
		return 255;
	}

	// Download the standard firmware to the unit
	printf("Downloading firmware...\n");
	if (perseus_firmware_download(descr,NULL)<0) {
		printf("firmware download error: %s", perseus_errorstr());
		return 255;
	}
	// Dump some information about the receiver (S/N and HW rev)
	if (descr->is_preserie == TRUE) 
		printf("The device is a preserie unit");
	else
		if (perseus_get_product_id(descr,&prodid)<0) 
			printf("get product id error: %s", perseus_errorstr());
		else
			printf("Receiver S/N: %05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW Release:%hd.%hd\n",
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
			printf("get sampling rates error: %s\n", perseus_errorstr());
			goto main_cleanup;
        } else {
            int i = 0;
            while (buf[i]) {
                printf("#%d: sample rate: %d\n", i, buf[i]);
                i++;
            }
        }
    }

	// Configure the receiver for 2 MS/s operations
	printf("Configuring FPGA...\n");
	if (perseus_set_sampling_rate(descr, sr) < 0) {  // specify the sampling rate value in Samples/second
	//if (perseus_set_sampling_rate_n(descr, 0)<0)        // specify the sampling rate value as ordinal in the vector
		printf("fpga configuration error: %s\n", perseus_errorstr());
		goto main_cleanup;
	}



    // Printing all attenuator values available .....
    {
        int buf[BUFSIZ];

        if (perseus_get_attenuator_values (descr, buf, sizeof(buf)/sizeof(buf[0])) < 0) {
			printf("get attenuator values error: %s\n", perseus_errorstr());
        } else {
            int i = 0;
            while (buf[i] != -1) {
                printf("#%d: att val in dB: %d\n", i, buf[i]);
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

	// Enable ADC Dither, Disable ADC Preamp
	perseus_set_adc(descr, TRUE, FALSE);

	// Do the same cycling test with the WB front panel led.
	// Enable preselection filters (WB_MODE Off)
	perseus_set_ddc_center_freq(descr, 7000000.000, 1);
	sleep(1);
	// Disable preselection filters (WB_MODE On)
	perseus_set_ddc_center_freq(descr, 7000000.000, 0);
	sleep(1);
	// Re-enable preselection filters (WB_MODE Off)
	perseus_set_ddc_center_freq(descr, 7000000.000, 1);

	// We open a file for binary output.
	// IQ samples will be saved in binary format to this file 
	fout = fopen("perseusdata","wb");
	if (!fout) {
		printf("Can't open output file\n");
		perseus_close(descr);
		perseus_exit();
		}
	
	// We start the acquisition passing our callback and its params 
	// (in this case the fout file handle we just open)	
	printf("Starting async data acquisition... \n");
	if (perseus_start_async_input(descr,16320,user_data_callback,fout)<0) {
		printf("start async input error: %s\n", perseus_errorstr());
		goto main_cleanup;
		}

	fprintf(stderr, "Collecting input samples... ");

	// We wait a 10 s time interval.
	// The user data callback we supplied to perseus_start_async_input 
	// is being called meanwhile.
	for (k=0;k<10;k++) {
		fprintf(stderr, ".");
		sleep(1);
	}

	fprintf(stderr, "\ndone\n");
	
	// We stop the acquisition...
	printf("Stopping async data acquisition...\n");
	perseus_stop_async_input(descr);

	// We can safely close the output file handle here. Acquisition has stopped.
	fclose(fout);

main_cleanup:

	// And we can finally quit the test application

	printf("Quitting...\n");
	perseus_close(descr);
	perseus_exit();

	printf("Bye\n");

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

int user_data_callback(void *buf, int buf_size, void *extra)
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
