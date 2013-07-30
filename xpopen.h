#ifndef XPOPEN_H_
#define XPOPEN_H_

#include <stdio.h>

FILE *xpopen(const char *cmdstring, const char *type);

/**
 * xpclose - send signal to child process before wait
 */
int xpclose(FILE *fp);

#endif /* MYPOPEN_H_ */
