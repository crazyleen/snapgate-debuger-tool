/*
 * mspdebug.h
 *
 *  Created on: Jul 29, 2013
 *      Author: heavey
 */

#ifndef MSPDEBUG_H_
#define MSPDEBUG_H_

FILE *mspdebug_prog_start(char *serial, char *filepath);
void mspdebug_prog_stop(FILE *pfd);

/**
 * prog device and start gdb proxy
 * return pipe file pointer
 * NOTE:
 * 		use mspdebug_prog_gdb_stop to close fp.
 */
FILE* mspdebug_prog_gdb_start(char *serial, int port, char *filepath);
void mspdebug_prog_gdb_stop(FILE *xfp);

/**
 * reset telosb, attach device to ttyUSBx
 */
void mspdebug_reset(char *serial);

/**
 * list usb device
 * return fp open by popen, NULL for error
 */
FILE *mspdebug_list_device_start(void);
void mspdebug_list_device_stop(FILE *pfd);

#endif /* SERVER_MSPDEBUG_FUNCTIONS_H_ */
