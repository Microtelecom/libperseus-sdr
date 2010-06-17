// ------------------------------------------------------------------------------
// Define the Perseus SDR Library interface
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

#ifndef _perseus_sdr_h
#define _perseus_sdr_h

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>
#include <time.h>

#include "perseusfx2.h"
#include "perseus-in.h"

// Microtelecom product code for the Perseus receiver
#define PERSEUS_PRODCODE    0x8014

// Nominal ADC sampling freq
#define PERSEUS_ADC_CLK_FREQ 80000000

#define PERSEUS_DDC_FREQ_MIN 0
#define PERSEUS_DDC_FREQ_MAX (PERSEUS_ADC_CLK_FREQ/2)

// Attenuator definitions 
#define PERSEUS_ATT_0DB		0
#define PERSEUS_ATT_10DB	1
#define PERSEUS_ATT_20DB	2
#define PERSEUS_ATT_30DB	3


// Microtelecom product id data structure
typedef struct __attribute__((__packed__)) {
	uint16_t 		sn;				// Receiver Serial Number
	uint16_t	 	prodcode;		// Microtelecom Product Code
	uint8_t			hwrel;			// Product release
	uint8_t			hwver;			// Product version
	uint8_t   		signature[6];	// Microtelecom Original Product Signature
} eeprom_prodid;

typedef struct perseus_descr_ds {
		int						index;
		libusb_device		 	*device;
	    libusb_device_handle 	*handle;
		uint8_t					bus;
		uint8_t					devaddr;
		int						is_cypress_ezusb;
		int						is_preserie;
		int						firmware_downloaded;
		int						fpga_configured;
		eeprom_prodid 			product_id;
		uint8_t					frontendctl;
		double					adc_clk_freq;
		double					ddc_lo_freq;
		uint8_t					presel_flt_id;
		fpga_sioctl				sioctl;
		perseus_input_queue		input_queue;
} perseus_descr;

#ifdef __cplusplus
extern "C" {
#endif

void	perseus_set_debug(int level);
int		perseus_init(void);
int		perseus_exit(void);
perseus_descr *perseus_open(int nDev);
int 	perseus_close(perseus_descr *descr);
int		perseus_firmware_download(perseus_descr *descr, char *fname);
int 	perseus_get_product_id(perseus_descr *descr, eeprom_prodid *prodid);
int		perseus_fpga_config(perseus_descr *descr, char *fname);
int		perseus_set_attenuator(perseus_descr *descr, uint8_t atten_id);
int		perseus_set_adc(perseus_descr *descr, int enableDither, int enablePreamp);
int		perseus_set_ddc_center_freq(perseus_descr *descr, double center_freq_hz, int enablePresel);
int		perseus_start_async_input(perseus_descr *descr, uint32_t buffersize, 
								perseus_input_callback callback, void *cb_extra);
int		perseus_stop_async_input(perseus_descr *descr);
int		perseus_set_sampling_rate(perseus_descr *descr, int new_sample_rate);
int		perseus_set_attenuator_in_db (perseus_descr *descr, int new_level_in_db);

#ifdef __cplusplus
}
#endif

#endif	// _perseus_sdr_h
