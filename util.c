/*
 * util.c
 *
 *  Created on: Jul 30, 2013
 *      Author: heavey
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * write lock
 * @return: 0 on success,  <0 and (errno == EACCES || errno == EAGAIN) is locked file
 */
int lockfile(int fd) {
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

/**
 * open and set write lock
 * @return: return fd, -1 on error, -2 on locked file(failed to set lock)
 */
int open_file_lock(const char *file) {
	int fd;

	//open for lock
	fd = open(file, O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
	if (fd < 0) {
		printf("%s: can't open %s: %s", __FUNCTION__, file, strerror(errno));
		//open error
		return -1;
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			//locked file, can't lock file again
			close(fd);
			return -2;
		}
		//lock error
		printf("can't lock %s: %s", file, strerror(errno));
		return -1;
	}

	return fd;
}

/**
 * write_pid_lock - lock file and write pid
 * @return: return fd, -1 on locked file, -2 on error
 */
int write_pid_lock(const char *file) {
	int fd;
	char buf[18];

	fd = open_file_lock(file);
	if (fd < 0) {
		//fail, -1 for locked, -2 on error
		return fd;
	}

	//write pid to file
	if (ftruncate(fd, 0) < 0) {
		;
	}
	snprintf(buf, 18, "%ld\n", (long) getpid());
	if (write(fd, buf, strlen(buf) + 1) != strlen(buf) + 1) {
		printf("write %s to pid file error\n", buf);
	}

	return fd;
}
