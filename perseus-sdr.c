// ------------------------------------------------------------------------------
// Perseus SDR Library Interface
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#define __USE_GNU
#if !defined(_WIN32)
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include "perseus-sdr.h"
#include "perseuserr.h"
#include "fpga_data.h"

#define PERSEUS_MAX_DESCR	8

// local data --------------------------------------------------------

static perseus_descr	perseus_list[PERSEUS_MAX_DESCR];
static int				perseus_list_entries 	= 0;
#if !defined(_WIN32)
static pthread_t		poll_libusb_thread = { 0 };
#else
static HANDLE			poll_libusb_thread = NULL;
#endif
static int 				poll_libusb_thread_flag = 0;
static int				poll_libusb_thread_stop = FALSE;

static char myPath [PATH_MAX+1] = {0};
static char appName[PATH_MAX+1] = {0}; 
static char appPath[PATH_MAX+1] = {0};

// local function decl -----------------------------------------------

#if !defined _WIN32
static void *poll_libusb_thread_fn(void *pparams);
#else
DWORD WINAPI poll_libusb_thread_fn(void *pparams);
#endif
static int	 perseus_set_presel(perseus_descr *descr, uint8_t presel_flt_id);

// API implementation ------------------------------------------------

void perseus_set_debug(int level)
{
	perseus_dbg_level = level;
}

int	perseus_init(void)
{
	
	ssize_t i, num_devs, num_perseus = 0;
	libusb_device **list;
	libusb_device *device;

	dbgprintf(3,"perseus_init()");

    // try to detect the directory from which the library has been loaded
#if !defined _WIN32
	{
		Dl_info info;
		char sep = '/';
		
		if (dladdr( &perseus_init, &info ) != 0) {
			dbgprintf(3,"Executable: %s", info.dli_fname); 
			strcpy (myPath, info.dli_fname);
		}
#elif defined _WIN32
	{
		char sep = '\\';

		if (GetModuleFileName(NULL, myPath, sizeof(myPath)) == 0) {
			return errorset(PERSEUS_CANTCREAT, "can't read executable location");
		} else {
			dbgprintf(3, "EXE: [%s]", myPath); 
		}
#else
	#error "UNKNOWN ENVIRONMENT"
#endif
		if (strlen(myPath)) {
			char *pos;
			pos = strrchr (myPath, sep);

			if (pos) {
				*pos = '\0';
				strcpy (appPath, myPath);
				appPath[strlen(myPath)] = sep;
				strcpy (appName, pos+1);
			}
		}
		dbgprintf(3, "path: [%s] name: [%s]", appPath, appName);
	}
	
	libusb_init(NULL);
	libusb_set_debug(NULL, 3 /* LIBUSB_LOG_LEVEL_INFO*/ );

	// find all perseus devices
	num_devs = libusb_get_device_list(NULL, &list);

	for (i=0;i<num_devs;i++) {
		struct libusb_device_descriptor descr;
		device = list[i];
		libusb_get_device_descriptor (device, &descr);
		perseus_list[num_perseus].index  	 = num_perseus;
		perseus_list[num_perseus].device 	 = device;
		perseus_list[num_perseus].bus        = libusb_get_bus_number(device);
		perseus_list[num_perseus].devaddr    = libusb_get_device_address(device);
		dbgprintf(2, "Found device with VID/PID %04X:%04X on BUS%d ADDR%d", 
					descr.idVendor, 
					descr.idProduct,
					perseus_list[num_perseus].bus,
					perseus_list[num_perseus].devaddr);
		if (descr.idVendor==PERSEUS_VID) {
			if (descr.idProduct==PERSEUS_PID_BLANKEEPROM) {
				perseus_list[num_perseus].is_cypress_ezusb = TRUE;
				libusb_ref_device(device);
				num_perseus++;
				} 
            else
			if (descr.idProduct==PERSEUS_PID) {
				libusb_ref_device(device);
				num_perseus++;
				}
        }
		if (num_perseus==PERSEUS_MAX_DESCR)
			break;
		}

	perseus_list_entries = num_perseus;

	libusb_free_device_list(list, 1);

	// start the libusb polling thread
	if (perseus_list_entries > 0 && poll_libusb_thread_flag == 0) {
		poll_libusb_thread_stop = FALSE;
		#if !defined _WIN32
		if (pthread_create(&poll_libusb_thread, NULL, &poll_libusb_thread_fn, NULL)!= 0) {
		#else
		HANDLE h =
		CreateThread(
			/* _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes */ NULL,
			/* _In_       SIZE_T dwStackSize*/ 102400,
			/* _In_       LPTHREAD_START_ROUTINE lpStartAddress */ poll_libusb_thread_fn,
			/* _In_opt_   LPVOID lpParameter *) */ NULL,
			/* _In_       DWORD dwCreationFlags */ 0,
			/* _Out_opt_  LPDWORD lpThreadId */ NULL
		);
		poll_libusb_thread = h;
		
		if (h == NULL) {
		#endif
			return errorset(PERSEUS_CANTCREAT, "can't create poll libusb thread");
		} else {
			poll_libusb_thread_flag = 1;
		}
	}

	return errornone(perseus_list_entries);
}

int	perseus_exit(void)
{
	dbgprintf(3,"perseus_exit(): poll_libusb_thread_flag=%d", poll_libusb_thread_flag);

	if (poll_libusb_thread_flag != 0) {
		poll_libusb_thread_stop = TRUE;
		#if !defined _WIN32
		pthread_join(poll_libusb_thread, NULL);
		#else
		WaitForSingleObject ( poll_libusb_thread, INFINITE);
		#endif
		poll_libusb_thread_flag = 0;
	}
	
	libusb_exit(NULL);
	dbgprintf(5,"after libusb_exit");
	perseus_list_entries 	= 0;
	poll_libusb_thread_flag = 0;
	poll_libusb_thread_stop = FALSE;

	return errornone(0);
}

perseus_descr *perseus_open(int nDev)
{
	perseus_descr *descr;
	int rc;
	int rc1, rc2, rc3;

	dbgprintf(3,"perseus_open(%d)",nDev);

	if (nDev<0 || nDev>=perseus_list_entries) {
		errorset(PERSEUS_INVALIDDEV, "invalid device id %d", nDev);
		return NULL;
		}

	descr = &perseus_list[nDev];

	if (descr->handle!=NULL) {
		errorset(PERSEUS_ALREADYOPEN, "device %d already open", nDev);
		return NULL;
		}

	if((rc = libusb_open(descr->device, &descr->handle))!=0) {
		errorset(PERSEUS_LIBUSBERR, "libusb_open error %d", rc);
		return NULL;
		}

	if (descr->is_cypress_ezusb) {
		// remove any kernel driver for Cypress EzUSB devices 
		int iface;
		for (iface=0;iface<4;iface++) 
			if (libusb_kernel_driver_active(descr->handle,iface)==1) {
				dbgprintf(1,"Detaching kernel driver (interface %d) for device %04X:%04X)",
							iface,
							PERSEUS_VID,
							PERSEUS_PID_BLANKEEPROM);
				if (libusb_detach_kernel_driver(descr->handle,iface)!=0)
					dbgprintf(1,"Interface %d detach failed",iface);
				}
		}

	if((rc = libusb_set_configuration(descr->handle,1))!=0) {
		errorset(PERSEUS_LIBUSBERR, "libusb_set_configuration error %d", rc);
		return NULL;
		}

	if((rc = libusb_claim_interface(descr->handle,0))!=0) {
		errorset(PERSEUS_LIBUSBERR, "libusb_claim_interface error %d", rc);
		return NULL;
		}

	if((rc = libusb_set_interface_alt_setting(descr->handle,0,0))!=0) {
		errorset(PERSEUS_LIBUSBERR, "libusb_set_interface_alt_setting error %d", rc);
		return NULL;
		}
	
	rc1 = libusb_clear_halt(descr->handle, PERSEUS_EP_CMD);
	rc2 = libusb_clear_halt(descr->handle, PERSEUS_EP_STATUS);
	rc3 = libusb_clear_halt(descr->handle, PERSEUS_EP_DATAIN);

	descr->is_preserie 			= FALSE;
	
	if (rc1 == 0 && rc2 == 0 && rc3 == 0)
		descr->firmware_downloaded  = TRUE;
	else
		descr->firmware_downloaded  = FALSE;
	
	descr->fpga_configured  	= FALSE;
	descr->adc_clk_freq			= PERSEUS_ADC_CLK_FREQ;

	return errornone(descr);

}

int perseus_close(perseus_descr *descr)
{
	int rc;

	dbgprintf(3,"perseus_close(%p)",descr);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errornone(0);

	if (descr->firmware_downloaded)
		perseus_fx2_shutdown(descr->handle);

	dbgprintf(3,"releasing interface...");

	rc = libusb_release_interface(descr->handle,0);
	if (rc<0) {
		dbgprintf(3,"libusb_release_interface rc =%d",rc);
		}
	else {
		dbgprintf(3,"done");
		}
		
	// AM 20141112
	// need to decrement the reference counter in order to completely free
	// libusb-1.0 internal resources
	libusb_unref_device(descr->device);
	
	dbgprintf(3,"closing device handle...");
	libusb_close(descr->handle);

	descr->handle=NULL;

	return errornone(0);
}

int	perseus_firmware_download(perseus_descr *descr, char *fname)
{
	int rc;
	ssize_t i, num_devs;
	libusb_device **list;
	libusb_device *device;
	int bus, devaddr;
	int index;

	dbgprintf(3,"perseus_firmware_download(%p,%s)",descr,(fname?fname:"Null"));

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded == TRUE) {
	
		descr->presel_flt_id        = PERSEUS_FLT_UNDEF;

		dbgprintf(3,"Firmware found !!!! Reading device eeprom...");

		if (perseus_fx2_read_eeprom(descr->handle, 
									PERSEUS_EEPROMADR_PRODID,
									&descr->product_id,
									sizeof(eeprom_prodid))<0) 
			return errorset(PERSEUS_EEPROMREAD, "failed to read device eeprom");

		dbgprintf(3,"read eeprom successful");

		if (descr->product_id.prodcode!=PERSEUS_PRODCODE) {
			descr->is_preserie = TRUE;
			dbgprintf(3,"Pre-production unit found");
		}
		return errornone(0);
	}

	dbgprintf(3,"perseus_firmware_download(%p,%s)",descr,(fname?fname:"Null"));
	
	descr->firmware_downloaded  = FALSE;
	descr->fpga_configured  	= FALSE;

	if (fname==NULL) {
		if ((rc=perseus_fx2_download_std_firmware(descr->handle))<0) 
			return rc;
		}
	else 
		return errorset(PERSEUS_FNNOTAVAIL, "Firmware download from files not implemented");

	dbgprintf(3,"done");

	perseus_close(descr);

	dbgprintf(3,"re-enumerating usb devices, please wait...");

	sleep(4);

	index = descr->index;				// get our index in the perseus descriptor list

	// scan usb devices after reenumeration
	// we assume that the device we have programmed is the last on list
    // in the same bus as it was prior to reenumeration
	perseus_list[index].device = NULL;	
	num_devs = libusb_get_device_list(NULL, &list);
	for (i=0;i<num_devs;i++) {
		struct libusb_device_descriptor usbdescr;
		device = list[i];
		libusb_get_device_descriptor (device, &usbdescr);
		bus 	= libusb_get_bus_number(device);
		devaddr = libusb_get_device_address(device);
		dbgprintf(2, "Found device with VID/PID %04X:%04X on BUS%d ADDR%d", 
				usbdescr.idVendor,
				usbdescr.idProduct,
				bus,
				devaddr);
		if (bus == perseus_list[index].bus)  
			if (usbdescr.idVendor==PERSEUS_VID && usbdescr.idProduct==PERSEUS_PID) {
				dbgprintf(2, "Perseus found on BUS%d ADDR%d after re-enumeration", bus, devaddr);
				perseus_list[index].device 	 = device;	// update usb device descriptor
				perseus_list[index].devaddr  = devaddr;	// and address
				perseus_list[index].is_cypress_ezusb = FALSE; // it's a PERSEUS_PID
				}
		}

	if (perseus_list[index].device==NULL) {
		// we could not find it
		libusb_free_device_list(list, 1);
		return errorset(PERSEUS_DEVNOTFOUND, "failed to find device");
		}

	// free all descriptor but our
	libusb_ref_device(perseus_list[index].device);

	// AM20141112
	// libusb_free_device_list moved after the open 
	// as per http://libusb.sourceforge.net/api-1.0/group__dev.html#details// 
	//libusb_free_device_list(list, 1);

	dbgprintf(3,"Please wait...");

	dbgprintf(3,"try to open the device after re-enumeration...");
	
	// reopen our device
	if (perseus_open(descr->index)==NULL)
		return errorset(PERSEUS_LIBUSBERR, "perseus_open error %d, reopening.", rc);

	libusb_free_device_list(list, 1);

	dbgprintf(3,"open successful");
	
	descr->firmware_downloaded  = TRUE;
	descr->presel_flt_id        = PERSEUS_FLT_UNDEF;

	dbgprintf(3,"reading device eeprom...");

	if (perseus_fx2_read_eeprom(descr->handle, 
							PERSEUS_EEPROMADR_PRODID,
							&descr->product_id,
							sizeof(eeprom_prodid))<0) 
		return errorset(PERSEUS_EEPROMREAD, "failed to read device eeprom");

	dbgprintf(3,"read eeprom successful");

	if (descr->product_id.prodcode!=PERSEUS_PRODCODE) {
		descr->is_preserie = TRUE;
		dbgprintf(3,"Pre-production unit found");
		}

	return errornone(0);

}

int perseus_get_product_id(perseus_descr *descr, eeprom_prodid *prodid)
{
	dbgprintf(3,"perseus_get_product_id(%p,...)",descr);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (prodid==NULL)
		return errorset(PERSEUS_NULLDESCR, "null eeprom_prodid pointer");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	memcpy(prodid, &descr->product_id, sizeof(eeprom_prodid));

	return errornone(0);
}


int	perseus_set_attenuator(perseus_descr *descr, uint8_t atten_id)
{
	int rc;

	dbgprintf(3,"perseus_set_attenuator(%p,%d)",descr, atten_id);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->is_preserie)
		atten_id ^= PERSEUS_ATT_30DB;

	descr->frontendctl = (atten_id<<4)|(descr->frontendctl&0x0F);

	if ((rc=perseus_fx2_set_porte(descr->handle, descr->frontendctl))<0)
		return rc;

	return errornone(0);
}

int	perseus_set_adc(perseus_descr *descr, int enableDither, int enablePreamp)
{
	int rc;

	dbgprintf(3,"perseus_set_adc(%p,%d,%d)",descr, enableDither, enablePreamp);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->fpga_configured==FALSE)
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured");

	if (enableDither)
		descr->sioctl.ctl |=PERSEUS_SIO_DITHER;
	else
		descr->sioctl.ctl &=~PERSEUS_SIO_DITHER;

	if (enablePreamp)
		descr->sioctl.ctl |=PERSEUS_SIO_GAINHIGH;
	else
		descr->sioctl.ctl &=~PERSEUS_SIO_GAINHIGH;

	if ((rc=perseus_fx2_sio(descr->handle, &descr->sioctl, NULL))<0)
		return rc;

	return errornone(0);
}

int	perseus_set_ddc_center_freq(perseus_descr *descr, double center_freq_hz, int enablePresel)
{
	uint8_t presel_flt_id;
	int rc;

	dbgprintf(3,"perseus_set_ddc_center_freq(%p,%.3f,%d)",descr, center_freq_hz, enablePresel);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->fpga_configured==FALSE)
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured");

	if (center_freq_hz<PERSEUS_DDC_FREQ_MIN || center_freq_hz>PERSEUS_DDC_FREQ_MAX)
		return errorset(PERSEUS_ERRPARAM, 
					"center_freq not in the range [%d..%d]", 
					PERSEUS_DDC_FREQ_MIN, 
					PERSEUS_DDC_FREQ_MAX);

	// Compute and set the value of the frequency register of the
	// Perseus 32 bit phase accumulator with the usual formula:
	// FREG = (Flo/Fclk)*(2^32)
	descr->sioctl.freg = (uint32_t)(center_freq_hz/descr->adc_clk_freq*4.294967296E9);
	if ((rc=perseus_fx2_sio(descr->handle, &descr->sioctl, NULL))<0)
		return rc;

	// Set preselection filter accordingly to the tuning freq
	if (enablePresel==FALSE) 
		presel_flt_id = PERSEUS_FLT_WB;
	else 
		if (center_freq_hz<PERSEUS_FLT_1_FC)
			presel_flt_id = PERSEUS_FLT_1;
		else if (center_freq_hz<PERSEUS_FLT_2_FC)
			presel_flt_id = PERSEUS_FLT_2;
		else if (center_freq_hz<PERSEUS_FLT_3_FC)
			presel_flt_id = PERSEUS_FLT_3;
		else if (center_freq_hz<PERSEUS_FLT_3_FC)
			presel_flt_id = PERSEUS_FLT_3;
		else if (center_freq_hz<PERSEUS_FLT_4_FC)
			presel_flt_id = PERSEUS_FLT_4;
		else if (center_freq_hz<PERSEUS_FLT_5_FC)
			presel_flt_id = PERSEUS_FLT_5;
		else if (center_freq_hz<PERSEUS_FLT_6_FC)
			presel_flt_id = PERSEUS_FLT_6;
		else if (center_freq_hz<PERSEUS_FLT_7_FC)
			presel_flt_id = PERSEUS_FLT_7;
		else if (center_freq_hz<PERSEUS_FLT_8_FC)
			presel_flt_id = PERSEUS_FLT_8;
		else if (center_freq_hz<PERSEUS_FLT_9_FC)
			presel_flt_id = PERSEUS_FLT_9;
		else if (center_freq_hz<PERSEUS_FLT_10_FC)
			presel_flt_id = PERSEUS_FLT_10;
		else
			presel_flt_id = PERSEUS_FLT_WB;

	return perseus_set_presel(descr, presel_flt_id);

}

static int	perseus_set_presel(perseus_descr *descr, uint8_t presel_flt_id)
{
	int rc;

	if (presel_flt_id==descr->presel_flt_id) 
		return errornone(0);

	descr->frontendctl = (descr->frontendctl&0xF0)|(presel_flt_id&0x0F);

	if ((rc=perseus_fx2_set_porte(descr->handle, descr->frontendctl))<0)
		return rc;

	descr->presel_flt_id = presel_flt_id;

	return errornone(0);
}

int	perseus_start_async_input(perseus_descr *descr, 
								uint32_t buffersize, 
								perseus_input_callback callback, 
								void *cb_extra)
{
	int rc, maxps;

	dbgprintf(3,"perseus_start_async_input(%p,%d,...)",descr, buffersize);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->fpga_configured==FALSE)
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured");

	if (descr->input_queue.transfer_queue!=NULL) 
		return errorset(PERSEUS_ASYNCSTARTED, "async input already started");

	// AM 30/01/2015 - Computed on the fly due to very wide range of values for the buffers sizes
	//if (buffersize>16320) 
	//	return errorset(PERSEUS_ERRPARAM, "max libusb bulk buffer size is 16320 bytes");

	maxps = libusb_get_max_packet_size (descr->device, PERSEUS_EP_DATAIN);	

    dbgprintf(3,"Max packet size on endpoint 0x%02x: %d", PERSEUS_EP_DATAIN, maxps);

	switch (maxps) {
		case 512: 
			if ((buffersize%6144)!=0) 
				return errorset(PERSEUS_BUFFERSIZE, "buffer size should be an integer multiple of 6144 bytes (1024 I/Q samples)");
			break;
		case 510:
			if ((buffersize%510)!=0)
				return errorset(PERSEUS_BUFFERSIZE, "buffer size should be an integer multiple of 510 bytes (85 IQ samples)");
			break;
		default:
			return errorset(PERSEUS_ERRPARAM, "Unexpected max packet size: %d", maxps);
    }

	// create and submit the data in transfer queue
	if ((rc=perseus_input_queue_create(&descr->input_queue, 8, descr->handle, buffersize, callback, cb_extra))<0)
		dbgprintf(0,"input transfer queue creation failed");

	// enable FPGA fifo
	descr->sioctl.ctl |=PERSEUS_SIO_FIFOEN;
	if ((rc=perseus_fx2_sio(descr->handle, &descr->sioctl, NULL))<0)
		dbgprintf(0,"FPGA fifo enable failed");

	return errornone(0);
}

int	perseus_stop_async_input(perseus_descr *descr)
{
	int rc;
	double elapsed;
	perseus_input_queue *queue = &descr->input_queue;

	dbgprintf(3,"perseus_stop_async_input(%p)",descr);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");
	
	// cancel the input transfer queue
	if ((rc=perseus_input_queue_cancel(&descr->input_queue))<0)
		dbgprintf(0,"datain transfer queue cancel failed");

	// wait for queue cancel to complete
	while (perseus_input_queue_completed(queue)==FALSE) ;

	// print some statistics...
	elapsed = 1.0E-6 *(queue->stop.tv_usec-queue->start.tv_usec) + 
					   queue->stop.tv_sec-queue->start.tv_sec; 
	dbgprintf(3, "Elapsed time: %f s - kSamples read: %ld - Rate: %.1f kS/s\n", elapsed, 
			queue->bytes_received/6000, 1.0*queue->bytes_received/elapsed/6000);

	// free the input transfer queue
	if ((rc=perseus_input_queue_free(queue))<0)
		dbgprintf(0,"datain transfer queue free failed");

	// Disable the FPGA fifo
	descr->sioctl.ctl &=~PERSEUS_SIO_FIFOEN;
	if ((rc=perseus_fx2_sio(descr->handle, &descr->sioctl, NULL))<0)
		dbgprintf(0,"FPGA fifo disable failed");

	return errornone(0);
}
#if !defined _WIN32
static void *poll_libusb_thread_fn(void *pparams)
#else
DWORD WINAPI poll_libusb_thread_fn(void *pparams)
#endif
{
	#if !defined _WIN32
	int maxpri, rc;
	#endif
	
	dbgprintf(3,"poll libusb thread started...");

	#if !defined _WIN32

	if ((maxpri = sched_get_priority_max(SCHED_FIFO))>=0) {
		struct sched_param sparam;
		dbgprintf(3,"setting thread priority to %d...", maxpri);
		sparam.sched_priority = maxpri;
		rc = pthread_setschedparam(poll_libusb_thread, SCHED_FIFO, &sparam);
		if (rc<0) {
			dbgprintf(0,"pthread_setschedparam failed. rc=%d",rc);
			}
		else {
			dbgprintf(3,"done");
			}
		}
		
	#else
	
	#endif
	// handle libusb events until perseus_exit is called
	while (poll_libusb_thread_stop==FALSE) {
			static struct timeval tv;
            tv.tv_sec = 1, tv.tv_usec = 0;
			libusb_handle_events_timeout(NULL, &tv);
	}
	dbgprintf(3,"poll libusb thread terminating...");

	return 0;
}

static int getFpgaFile (int xsr)
{
	unsigned int i;
    //const char *fn = 0;
    int prev = 0;
    int index = -1;
    int vs = nFpgaImages;

	for (i=0; i<vs; ++i) {

        if ( xsr > fpgaImgTbl[i].speed) {

            if ( i < (vs-1)) {
                prev = fpgaImgTbl[i].speed;
                continue;

            } else {
                //fn = fpgaImgTbl[i].name;
                index = i;
                break;
            }

        } else {
            int m = (fpgaImgTbl[i].speed + prev) / 2;
            if (xsr <= m) {
                if (i==0) {
                    return i;
                } else {
                    return i-1;
                }
            } else 
                return i;
        }
	}
	return index;
}


int  perseus_get_sampling_rates (perseus_descr *descr, int *buf, unsigned int size)
{
    if (size > 0) {
        unsigned int i, j;

        for (i=0; i < size; ++i) { buf[i] = 0; }

        for (i=0,j=0; i< nFpgaImages; ++i) {
            if (j < size) { 
               buf[j++] = fpgaImgTbl[i].speed; 
            } else {
               return errorset (PERSEUS_BUFFERSIZE, "Insufficient buffer size");
            }
        }
        return errornone(0);
    } else {
        return errorset (PERSEUS_ERRPARAM, "Zero lenght buffer");
    }
}




int		perseus_set_sampling_rate(perseus_descr *descr, int new_sample_rate)
{
	int index;

	dbgprintf(3,"perseus_set_sampling_rate(%p,%d)",descr, new_sample_rate);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	index = getFpgaFile (new_sample_rate);

    if ( index >= 0 ) {
        if (perseus_fx2_fpga_config_sr(descr->handle, fpgaImgTbl[index].speed) < 0) {
            dbgprintf(0,"fpga configuration error\n");
            return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured: error in perseus_set_sampling_rate. [%d]", index);
        }
    } else {
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured: sampling rate not found");
    }
    descr->fpga_configured          = TRUE;

    dbgprintf(3,"fpga config successfull");

	return errornone(0);
}


int		perseus_set_sampling_rate_n(perseus_descr *descr, unsigned int nso)
{
	dbgprintf(3,"perseus_set_sampling_rate_n(%p,%d)",descr, nso);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

    //
    // Compute the size of the vector
    int vs = nFpgaImages;

    if (nso >= vs) {
        return errorset (PERSEUS_ERRPARAM, "Invalid index in vector");
    } else {
        return perseus_set_sampling_rate(descr, fpgaImgTbl[nso].speed);
    }
}


//
// Static data for attenuation fromn numerical value in dB to constants conversion
//

typedef struct _atten {
	int id;
	int valInDb;
} AttenuatorValues ;


static AttenuatorValues av [] = {
	{ PERSEUS_ATT_0DB , 0  } ,
	{ PERSEUS_ATT_10DB, 10 } ,
	{ PERSEUS_ATT_20DB, 20 } ,
	{ PERSEUS_ATT_30DB, 30 } ,
} ;

int perseus_set_attenuator_in_db (perseus_descr *descr, int new_level_in_db)
{
	int i;

	dbgprintf(3,"perseus_set_attenuator_in_db(%p,%d)",descr, new_level_in_db);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->fpga_configured==FALSE)
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured");

   	for (i=0; i< (sizeof(av)/sizeof(av[0])); ++i)
   		if (new_level_in_db == av[i].valInDb) {
   			return perseus_set_attenuator(descr, av[i].id);
        }
    return errorset(PERSEUS_ATTERROR, "set attenuator error, bad value: %d", new_level_in_db);
}


int		perseus_get_attenuator_values (perseus_descr *descr, int *buf, unsigned int size)
{
    if (size > 0) {
        unsigned int i, j;

        for (i=0; i < size; ++i) { buf[i] = -1; }

        for (i=0,j=0; i< (sizeof(av)/sizeof(av[0])); ++i) {
            if (j < size) { 
               buf[j++] = av[i].valInDb; 
            } else {
               return errorset (PERSEUS_BUFFERSIZE, "Insufficient buffer size");
            }
        }
        return errornone(0);
    } else {
        return errorset (PERSEUS_ERRPARAM, "Zero lenght buffer");
    }

}


int perseus_set_attenuator_n (perseus_descr *descr, int nlo)
{
	dbgprintf(3,"perseus_set_attenuator_n(%p,%d)",descr, nlo);

	if (descr==NULL)
		return errorset(PERSEUS_NULLDESCR, "null descriptor");

	if (descr->handle==NULL) 
		return errorset(PERSEUS_DEVNOTOPEN, "device not open");

	if (descr->firmware_downloaded==FALSE)
		return errorset(PERSEUS_FWNOTLOADED, "firmware not loaded");

	if (descr->fpga_configured==FALSE)
		return errorset(PERSEUS_FPGANOTCFGD, "FPGA not configured");

    //
    // Compute the size of the vector
    int vs = (sizeof(av)/sizeof(av[0]));

    if (nlo >= vs) {
        return errorset (PERSEUS_ERRPARAM, "Invalid index in vector");
    } else {
        if (perseus_set_attenuator(descr, av[nlo].id) < 0) {
            return errorset(PERSEUS_ATTERROR, "set attenuator error");
        } else {
            return errornone(0);
        }
    }
}


//
// https://github.com/lorf/csr-spi-ftdi/blob/master/compat.c
//
#if defined _WIN32
/*
* This fixes linking with precompiled libusb-1.0.18-win and
* libusb-1.0.19-rc1-win: "undefined reference to __ms_vsnprintf". See:
* http://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/4F8CA26A.70103@users.sourceforge.net/
*/
#include <stdio.h>
#include <stdarg.h>
int (*__ms_vsnprintf)(char *, size_t, const char *, va_list) = &vsnprintf;
#endif
