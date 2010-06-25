// ------------------------------------------------------------------------------
// Perseus <-> Fx2 MCU interface
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

#ifndef _perseusfx2_h
#define _perseusfx2_h

#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

#define FALSE	0
#define TRUE	1

// Perseus VID/PID
#define PERSEUS_VID				0x04B4
#define PERSEUS_PID_BLANKEEPROM 0x8613
#define PERSEUS_PID       		0x325C

// Perseus Fx2 firmware end points
#define PERSEUS_EP_CMD			0x01
#define PERSEUS_EP_STATUS		0x81
#define PERSEUS_EP_DATAIN	  	0x82

// Fx2 MCU specific vendor command defines
#define FX2_BM_VENDOR_REQUEST		0x40
#define FX2_REQUEST_FIRMWARE_LOAD	0xA0
#define FX2_ADDR_CPUCS				0xE600

// Defines and commands interpreted by the Perseus Fx2 firmware
#define PERSEUS_CMD_CONFIG_TRANSFER_SIZE 64
#define PERSEUS_EEPROMADR_PRODID 8

#define PERSEUS_CMD_FPGACONFIG	0x00
#define PERSEUS_CMD_FPGARESET	0x01
#define PERSEUS_CMD_FPGACHECK	0x02
#define PERSEUS_CMD_FPGASIO		0x03
#define PERSEUS_CMD_FX2PORTE	0x04
#define PERSEUS_CMD_EEPROMREAD	0x06
#define PERSEUS_CMD_SHUTDOWN	0x08

// Receiver Control through Fx2<->FPGA Serial IO interface
#define PERSEUS_SIO_FIFOEN		0x01
#define PERSEUS_SIO_DITHER		0x02
#define PERSEUS_SIO_GAINHIGH	0x04

// Preselection filter bank defines
#define PERSEUS_FLT_1		0
#define PERSEUS_FLT_2		1
#define PERSEUS_FLT_3		2
#define PERSEUS_FLT_4		3
#define PERSEUS_FLT_5		4
#define PERSEUS_FLT_6		5
#define PERSEUS_FLT_7		6
#define PERSEUS_FLT_8		7
#define PERSEUS_FLT_9		8
#define PERSEUS_FLT_10		9
#define PERSEUS_FLT_WB		10
#define PERSEUS_FLT_UNDEF	255

// Upper cutoff frequency of the preselection filters (in Hz)
#define PERSEUS_FLT_1_FC	 1700000
#define PERSEUS_FLT_2_FC	 2100000
#define PERSEUS_FLT_3_FC	 3000000
#define PERSEUS_FLT_4_FC	 4200000
#define PERSEUS_FLT_5_FC	 6000000
#define PERSEUS_FLT_6_FC	 8400000
#define PERSEUS_FLT_7_FC	12000000
#define PERSEUS_FLT_8_FC	17000000
#define PERSEUS_FLT_9_FC	24000000
#define PERSEUS_FLT_10_FC	32000000

// Fx2 MCU <-> FPGA serial io data structure
typedef struct __attribute__((__packed__)) {
	uint8_t 	ctl;
	uint32_t	freg;
} fpga_sioctl;

#ifdef __cplusplus
extern "C" {
#endif

int perseus_fx2_reset(libusb_device_handle *handle, uint8_t reset);
int perseus_fx2_ram_write(libusb_device_handle *handle, uint16_t wAddr, char *buf, int buflen);
int perseus_fx2_download_std_firmware(libusb_device_handle *handle);
int perseus_fx2_read_eeprom(libusb_device_handle *handle, uint16_t addr, void *buf, int bufsize);
int perseus_fx2_fpga_config(libusb_device_handle *handle, FILE* fbitstream);
int	perseus_fx2_set_porte(libusb_device_handle *handle, uint8_t porte);
int	perseus_fx2_sio(libusb_device_handle *handle, fpga_sioctl *sioctl, fpga_sioctl *siostatus);
int	perseus_fx2_sioex(libusb_device_handle *handle, void* dataout, int16_t dataoutsize, void *datain, int16_t datainsize);
int perseus_fx2_shutdown(libusb_device_handle *handle);
int perseus_fx2_fpga_config_sr(libusb_device_handle *handle, int sample_rate);

#ifdef __cplusplus
}
#endif


#endif // _perseusfx2_h
