// ------------------------------------------------------------------------------
// Interface to the Fx2 MCU 
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "perseusfx2.h"
#include "fpga_data.h"
#include "perseus-sdr.h"

#define FX2_TIMEOUT 	1000

// Intel Hex File Format Record Types
#define IHEX_DATA			0x00
#define IHEX_EOF			0x01
#define IHEX_EXTSEGADDR		0x02
#define IHEX_EXTLINADDR		0x03
#define IHEX_INVALID		0xFF

// Parsed Intel Hex File Record
typedef struct {
	uint8_t		cRecType;
	uint16_t	wAddress;
	uint16_t	wBytes;
	uint8_t   	cData[128];
} IHexRecord;

typedef struct __attribute__((__packed__)) {
	char		eeprom_op;
	uint16_t	addr;
	uint8_t		count;
} eeprom_cmd;

typedef	struct __attribute__((__packed__)) {
	char 	eeprom_op;
	char	op_retcode;
} eeprom_reply_header;

typedef	struct __attribute__((__packed__)) {
	eeprom_reply_header	header;
	char	data;
} eeprom_reply;

#if !HAVE_LIBUSB_STRERROR
char *libusb_strerror (int x){return "libusb_strerror not available in installed libusb 1.0 library.";}
#endif

// Perseus standard firmware for the Fx2 MCU
// NP20141111 -- This was the firmware version with 510 bytes endpoints
//#include "perseus24v11_512.c"

// NP20141111 ++ This is the lastest firmware version (24v41_512) with 512 bytes endpoints
#include "perseus24v41_512.c"

int perseus_fx2_reset(libusb_device_handle *handle, uint8_t reset) 
{
	// Control the Fx2 CPU Reset line
	// Issue a FX2_BM_VENDOR_REQUEST requestType, FX2_REQUEST_FIRMWARE_LOAD request 
	// with wValue FX2_ADDR_CPUCS
	// and data = reset line status (1:hold CPU reset, 0: Leave CPU reset)
	int rc;

	rc = libusb_control_transfer (handle, 
								FX2_BM_VENDOR_REQUEST,
								FX2_REQUEST_FIRMWARE_LOAD,
								FX2_ADDR_CPUCS,
								0,
								&reset,
								1,
								FX2_TIMEOUT);
	if (rc<0) 
		return errorset(PERSEUS_IOERROR, "fx2 reset(%d) failed, (%d-%s-%s).",(uint16_t)reset, 
						rc, libusb_error_name(rc), libusb_strerror(rc));

	return errornone(0);
}

int perseus_fx2_ram_write(libusb_device_handle *handle, uint16_t wAddr, char *buf, int buflen) 
{
	// Download data to Fx2 RAM at address wAddr
	// wAddr:	start address (must be even)
	// pBuf:	bytes to write
	// iBufLen: number of bytes to write

	// send a FX2_BM_VENDOR_REQUEST requestType, FX2_REQUEST_FIRMWARE_LOAD request with value = wAddr 
	if (libusb_control_transfer (handle, 
								FX2_BM_VENDOR_REQUEST,
								FX2_REQUEST_FIRMWARE_LOAD,
								wAddr,
								0,
								(unsigned char *)buf,
								buflen,
								FX2_TIMEOUT)<0) 
		return errorset(PERSEUS_IOERROR, "fx2 ram write failed at addr=%04hX",wAddr);

	return errornone(0);
}

int perseus_fx2_read_eeprom(libusb_device_handle *handle, uint16_t addr, void *buf, int bufsize) 
{
	int rc;
	int transferred = 0;
	eeprom_cmd 	 cmd = { PERSEUS_CMD_EEPROMREAD, addr, bufsize };
	int replysize;
	eeprom_reply *reply;

	// Issue a EEPROMREAD command to the Fx2
	if ((rc=libusb_bulk_transfer(handle, 
									PERSEUS_EP_CMD, 
									(unsigned char*)&cmd, sizeof(cmd), 
									&transferred, 
									FX2_TIMEOUT))<0)
		return errorset(PERSEUS_IOERROR, "EEPROMREAD command failed (%d-%s-%s). Written %d bytes", rc, libusb_error_name(rc), libusb_strerror(rc), transferred);

	// Read EEPROM status
	replysize = sizeof(eeprom_reply_header)+bufsize;
	reply = (eeprom_reply*)malloc(replysize);

	if ((rc=libusb_bulk_transfer(handle, 
									PERSEUS_EP_STATUS, 
									(unsigned char *)reply, replysize, 
									&transferred, 
									FX2_TIMEOUT))<0) {
			free(reply);
			return errorset(PERSEUS_IOERROR, "eeprom status read failed (%d-%s-%s). Read %d bytes", rc, libusb_error_name(rc), libusb_strerror(rc), transferred);
			}

	if (reply->header.op_retcode!=TRUE) {
		free(reply);
		return errorset(PERSEUS_EEPROMREAD, "Fx2 eeprom read command error");
		}

	memcpy(buf, &reply->data, bufsize);
	free(reply);
	return errornone(0);
}

int perseus_fx2_download_std_firmware(libusb_device_handle *handle)
{
	int k, rc;

	dbgprintf(3,"downloading std firmware to fx2 cpu");

	// Enter Fx2 CPU reset state
	if ((rc=perseus_fx2_reset(handle,TRUE))<0)
		return rc;

	dbgprintf(3,"fx2 cpu reset(1) ok...");

	// Scan Perseus standard firmware intel hex records
	for (k=0; k<FW_NRECORDS; k++) {
		// and write to Fx2 RAM at specified address
		if ((rc=perseus_fx2_ram_write(handle, 
								fwHexRecTable[k].wAddress, 
								(char *)fwHexRecTable[k].cData, 
								fwHexRecTable[k].wBytes))<0) {
				dbgprintf(1,"fx2 ram write failed at record %d/%d",k,FW_NRECORDS);
				return rc;
				}
		}

	dbgprintf(3,"fx2 cpu ram write ok...");

// NP20141111 -- We dont check the return status anymore
// as it looks that new USB3.0 Platforms do not handle the call as in previous versions
	// Leave Fx2 CPU reset state
	//	if ((rc=perseus_fx2_reset(handle,FALSE))<0)
	//	return rc;

// NP20141111 -- We simply make the call and ignore the result
	perseus_fx2_reset(handle,FALSE);

	dbgprintf(3,"fx2 cpu reset(0) ok...");

	return errornone(0);
}

int perseus_fx2_shutdown(libusb_device_handle *handle)
{
	int transf;
	int rc;
	// reset FPGA, disconnect preselection filters and set 30 dB attenuation

	char cmd = PERSEUS_CMD_SHUTDOWN;
	// Issue a SHUTDOWN command 
	if ((rc=libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)&cmd,1, &transf, FX2_TIMEOUT))<0) 
		return errorset(PERSEUS_IOERROR, "shutdown command failed (%d-%s-%s)", rc, libusb_error_name(rc), libusb_strerror(rc));

    return errornone(0);
}

int	perseus_fx2_set_porte(libusb_device_handle *handle, uint8_t porte)
{
	int transferred;
	int rc;
	char cmd[2] = { PERSEUS_CMD_FX2PORTE, porte };

	// Issue a FX2PORTE command
	if ((rc=libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)cmd, 2, &transferred, FX2_TIMEOUT))<0) 
		return errorset(PERSEUS_IOERROR, "FX2PORTE command failed (%d-%s-%s)", rc, libusb_error_name(rc), libusb_strerror(rc));

	return errornone(0);
}

int	perseus_fx2_sio(libusb_device_handle *handle, fpga_sioctl *sioctl, fpga_sioctl *siostatus)
{
	int transferred;
	char cmd[1+sizeof(fpga_sioctl)] = { PERSEUS_CMD_FPGASIO };

	memcpy(&cmd[1], sioctl, sizeof(fpga_sioctl));

	// Issue a FPGASIO command
	if (libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)cmd, 1+sizeof(fpga_sioctl), &transferred, FX2_TIMEOUT)<0) 
		return errorset(PERSEUS_IOERROR, "FPGASIO command failed");

	// Read FPGASIO status
	if (libusb_bulk_transfer(handle, PERSEUS_EP_STATUS, (unsigned char *)cmd, 1+sizeof(fpga_sioctl), &transferred, FX2_TIMEOUT)<0) 
		return errorset(PERSEUS_IOERROR, "FPGASIO status read failed");

	if (siostatus)
		memcpy(siostatus, &cmd[1],sizeof(fpga_sioctl));
 
	return errornone(0);
}
 

int	perseus_fx2_sioex(libusb_device_handle *handle, void* dataout, int16_t dataoutsize, void *datain, int16_t datainsize) 
{
	char *cmd;
	int transferred;

	if (datain==NULL)
		datainsize = dataoutsize;
	else
		if (datainsize>dataoutsize) 
			return errorset(PERSEUS_IOERROR, "error calling perseus_fx2_sio (datainsize>dataoutsize)");

	cmd = (char*)malloc(1+dataoutsize);
	if (!cmd) 
			return errorset(PERSEUS_NOMEM, "can't allocate buffer for FPGASIO command");

	cmd[0] = PERSEUS_CMD_FPGASIO;		
	memcpy(&cmd[1], dataout, dataoutsize);

	// Issue a FPGASIO command
	if (libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)cmd, 1+dataoutsize, &transferred, FX2_TIMEOUT)<0) {
		free(cmd);
		return errorset(PERSEUS_IOERROR, "FPGASIO command failed");
		}

	// Read FPGASIO status
	if (libusb_bulk_transfer(handle, PERSEUS_EP_STATUS, (unsigned char *)cmd, 1+datainsize, &transferred, FX2_TIMEOUT)<0) {
		free(cmd);
		return errorset(PERSEUS_IOERROR, "FPGASIO status read failed");
		}

	if (datain)
		memcpy(datain, &cmd[1],datainsize);

	free(cmd);

	return errornone(0);
}

int perseus_fx2_fpga_config_sr (libusb_device_handle *handle, int sample_rate)
{
	char cmdrst = PERSEUS_CMD_FPGARESET;
	char cmdconfig[PERSEUS_CMD_CONFIG_TRANSFER_SIZE] = { PERSEUS_CMD_FPGACONFIG };
	char *pcmdconfigdata = cmdconfig+1;
	char cmdcheck = PERSEUS_CMD_FPGACHECK;
	char cmdcheck_status[2];
	int	 transf;
	clock_t endclock;
    int  n;

    for (n=0; n<nFpgaImages;++n) {
        
        if (sample_rate == fpgaImgTbl [n].speed) {
            const unsigned char *p = fpgaImgTbl [n].code;
            int left = fpgaImgTbl [n].size;
            int ntowrite;

            dbgprintf(3,"perseus_fx2_fpga_config_speed(%p,): %d %s %d",(void *)handle, 
                      fpgaImgTbl [n].speed, fpgaImgTbl [n].name, fpgaImgTbl [n].size);

			// Issue a FPGA reset command
			if (libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)&cmdrst,1, &transf, FX2_TIMEOUT)<0) 
				return errorset(PERSEUS_IOERROR, "FPGA reset command failed");

			// Write FPGA configuration data
			//while ((nread = fread(pcmdconfigdata, 1, PERSEUS_CMD_CONFIG_TRANSFER_SIZE-1, fbitstream)) !=0 ) {

            while (left != 0) {
				//ntowrite = nread+1;

                ntowrite = ((PERSEUS_CMD_CONFIG_TRANSFER_SIZE-1) < left ? (PERSEUS_CMD_CONFIG_TRANSFER_SIZE-1): left);

				memcpy (pcmdconfigdata, p, ntowrite);
				left     -= ntowrite;
				p        += ntowrite;
				ntowrite += 1; 

				if (libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)cmdconfig, ntowrite, &transf, FX2_TIMEOUT)<0) 
					return errorset(PERSEUS_IOERROR, "io error writing FPGA configuration data");
 
			}

			// Issue a FPGA check command
			if (libusb_bulk_transfer(handle, PERSEUS_EP_CMD, (unsigned char *)&cmdcheck,1, &transf, FX2_TIMEOUT)<0)
				return errorset(PERSEUS_IOERROR, "FPGA check command failed");

			// wait 50 ms
		    endclock = clock() + 50*CLOCKS_PER_SEC/1000;
		    while (clock() < endclock) { };

			// Read FPGA check status
			if (libusb_bulk_transfer(handle, PERSEUS_EP_STATUS, (unsigned char *)cmdcheck_status,2, &transf, FX2_TIMEOUT)<0) 
				return errorset(PERSEUS_IOERROR, "FPGA check status failed");

			// Check FPGA DONE line high
			if (cmdcheck_status[1]!=TRUE) 
				return errorset(PERSEUS_FPGACFGERROR, 
								"FPGA configuration failed rc=[%hd,%hd]",
								(uint16_t)cmdcheck_status[0],
								(uint16_t)cmdcheck_status[1]);

			return errornone(0);
        }

    }

	return errorset(PERSEUS_FPGACFGERROR, "FPGA configuration failed, invalid speed.");
}

