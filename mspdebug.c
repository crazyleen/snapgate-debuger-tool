#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "xpopen.h"
#include "mspdebug.h"

//#define DEBUG_FUNCTIONS_Trace
//#define DEBUG_FUNCTIONS_Info
#include "debug.h"

FILE *mspdebug_prog_start(char *serial, char *filepath) {
	char commond[1024];
	if (filepath == NULL)
		return NULL;

	else
		snprintf(commond, sizeof(commond),
				"mspdebug -s %s -j ft232h \"prog %s\"", serial, filepath);

	return popen(commond, "r");
}

void mspdebug_prog_stop(FILE *pfd) {
	pclose(pfd);
}

/**
 * prog device and start gdb proxy
 * return pipe file pointer
 * NOTE:
 * 		use mspdebug_prog_gdb_stop to close fp.
 */
FILE* mspdebug_prog_gdb_start(char *serial, int port, char *filepath) {
	char commond[1024];
	if (filepath == NULL)
		snprintf(commond, sizeof(commond),
				"mspdebug -s %s -j ft232h \"reset\" \"gdb %d\"", serial, port);
	else
		snprintf(commond, sizeof(commond),
				"mspdebug -s %s -j ft232h \"prog %s\" \"reset\" \"gdb %d\"",
				serial, filepath, port);
	Trace("xpopen: %s", commond);
	return xpopen(commond, "r");
}

void mspdebug_prog_gdb_stop(FILE *xfp) {
	xpclose(xfp);
}

/**
 * reset telosb, attach device to ttyUSBx
 */
void mspdebug_reset(char *serial) {
	char commond[1024];
	snprintf(commond, sizeof(commond), "mspdebug -s %s -j ft232h reset", serial);
	system(commond);
}

/**
 * list usb device
 * return fp open by popen, NULL for error
 */
FILE *mspdebug_list_device_start(void) {
	const char *commond = "mspdebug --usb-list";
	return popen(commond, "r");
}

void mspdebug_list_device_stop(FILE *pfd) {
	pclose(pfd);
}
