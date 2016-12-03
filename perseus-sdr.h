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
#if !defined(_WIN32)
#include <pthread.h>
#endif
#include <libusb-1.0/libusb.h>
#include <time.h>

/** @file */

//! Microtelecom product code for the Perseus receiver
#define PERSEUS_PRODCODE    0x8014

//! Nominal ADC sampling freq
#define PERSEUS_ADC_CLK_FREQ 80000000

//! Minimum nominal frequency
#define PERSEUS_DDC_FREQ_MIN 0

//! Maximum nominal frequency
#define PERSEUS_DDC_FREQ_MAX (PERSEUS_ADC_CLK_FREQ/2)

//! Attenuator definition macros

//! 0dB, attenuator disabled
#define PERSEUS_ATT_0DB		0
//! 10dB attenuation
#define PERSEUS_ATT_10DB	1
//! 20dB attenuation
#define PERSEUS_ATT_20DB	2
//! 30dB attenuation
#define PERSEUS_ATT_30DB	3


/*!
 *  \brief Microtelecom product id data structure
 *
 */
typedef struct __attribute__((__packed__)) {
	uint16_t 		sn;				/**< Receiver Serial Number */
	uint16_t	 	prodcode;		/**< Microtelecom Product Code */
	uint8_t			hwrel;			/**< Product release */
	uint8_t			hwver;			/**< Product version */
	uint8_t   		signature[6];	/**< Microtelecom Original Product Signature */
} __attribute__((packed, aligned(1)))
eeprom_prodid;

/*!
 * 
 */
 
typedef int (*perseus_input_callback)(void *buf, int buf_size, void *extra);

struct perseus_descr_ds;
typedef struct perseus_descr_ds perseus_descr;

//#include "perseusfx2.h"
//#include "perseus-in.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Initialize the debug level of library
 *
 * \param debug_level an integer 0..9 , 0 less verbose, 9 more verbose
 */
void	perseus_set_debug(int debug_level);

/*!
 * Initilize the library and discover all Perseus radio attached
 *
 * \return number of device attached
 */
int		perseus_init(void);

/*!
 * Deinitilize the library
 *
 * Should be the last library function to be invoked
 */
int		perseus_exit(void);

/*!
 * Open the selected device
 * and return a descriptor to be used in all subsequent calls
 * The pointer returned points to a library data structure and has to 
 * be considered as a pointer to an opaque type
 * 
 * \param nDev ordinal of device to be opened
 * \return pointer to a device descriptor, NULL if no device is found
 */
perseus_descr *perseus_open(int nDev);

/*!
 * Close the device associated to descriptor
 *
 * \param descr pointer to descriptor
 * \return 0 if successfull
 * \return < 0 in case of error 
 */
int 	perseus_close(perseus_descr *descr);

/*!
 * Load the FPGA bitstream into the hardware
 *
 * This function has to be called mandatorily after the perseus_open
 * 
 * \param descr pointer to descriptor
 * \param fname bitstream file name, used only for test, set to NULL
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_firmware_download(perseus_descr *descr, char *fname);

/*!
 * Read product id and serial number from the hardware
 *
 * \param descr pointer to descriptor
 * \param prodid pointer to eeprom_prodid structure allocated in client
 *        program data segment
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int 	perseus_get_product_id(perseus_descr *descr, eeprom_prodid *prodid);

/*!
 * Set the attenuator (as index)
 *
 * \param descr pointer to descriptor
 * \param atten_id attenuator index
 * - \ref PERSEUS_ATT_0DB
 * - \ref PERSEUS_ATT_10DB
 * - \ref PERSEUS_ATT_20DB
 * - \ref PERSEUS_ATT_30DB
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_attenuator(perseus_descr *descr, uint8_t atten_id);

/*!
 * Set the attenuator (as value in dB)
 *
 * \param descr pointer to descriptor
 * \param att_level_in_db attenuator value in dB
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_attenuator_in_db (perseus_descr *descr, int att_level_in_db);

/*!
 * Get attenuator values available (in dB)
 *
 * \param descr pointer to descriptor
 * \param buf pointer to an integer array (to be allocated in client data
 *        segment
 * \param size size of array
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_get_attenuator_values (perseus_descr *descr, int *buf, unsigned int size);

/*!
 * Set attenuator values as index into the array read in \ref perseus_get_attenuator_values
 *
 * \param descr pointer to descriptor
 * \param nlo index in the array read by \ref perseus_get_attenuator_values
 *
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_attenuator_n (perseus_descr *descr, int nlo);

/*!
 * Set ADC parameters
 *
 * \param descr pointer to descriptor
 * \param enableDither
 *        1 enable hardware dithering in ADC
 *        0 disable hardware dithering in ADC
 * \param enablePreamp
 *        1 enable preamplifier in ADC
 *        0 disable preamplifier in ADC
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_adc(perseus_descr *descr, int enableDither, int enablePreamp);

/*!
 * Set the center digital down converter (DDC) and automatic
 * analog preselector
 *
 * \param descr pointer to descriptor
 * \param center_freq_hz central frequency of DDC in Hz
 * \param enablePresel
 *        1 enables analog preselector
 *        0 disables analog preselector
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_ddc_center_freq(perseus_descr *descr, double center_freq_hz, int enablePresel);

/*!
 * Start asynch input
 *
 * \param descr pointer to descriptor
 * \param buffersize size of buffers used inside the library when collecting data
 *        from hardware, 6*1024 suggested
 * \param callback pointer to the callback function \ref perseus_input_callback
 * \param cb_extra pointer to a data block that will be passed on each callback
 *        recall
 *        can be used as context inside the callback
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_start_async_input(perseus_descr *descr, uint32_t buffersize, 
								perseus_input_callback callback, void *cb_extra);

/*!
 * Stop data collection from hardware
 *
 * \param descr pointer to descriptor
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_stop_async_input(perseus_descr *descr);

/*!
 * Set sampling rate (as value in S/s)
 *
 * \param descr pointer to descriptor
 * \param sample_rate_value sample rate as S/s value
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_sampling_rate(perseus_descr *descr, int sample_rate_value);

/*!
 * Set sample rate (as ordinal)
 *
 * \param descr pointer to descriptor
 * \param sample_rate_ordinal index in the list achieved by \ref perseus_get_sampling_rates
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_set_sampling_rate_n(perseus_descr *descr, unsigned int sample_rate_ordinal);

/*!
 * Get available sampling rate as vector of values
 *
 * that list is automagically determined during 
 * the build process from the FPGA images available in the build directory.
 * All the images provided (ten at the moment) are embedded into the shared 
 * library, so the .so is the only file needed at runtime.
 * 
 * \param descr pointer to descriptor
 * \param buf pointer to an integer array, to be allocated in client
 *        data segment
 * \param size size of array
 * \return 0 if successfull
 * \return < 0 in case of error
 */
int		perseus_get_sampling_rates(perseus_descr *descr, int *buf, unsigned int size);

int		perseus_is_preserie(perseus_descr *descr, int *flag);

#ifdef __cplusplus
}
#endif


// Error codes and macros

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

#endif	// _perseus_sdr_h
