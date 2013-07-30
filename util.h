/*
 * util.h
 *
 *  Created on: Jul 30, 2013
 *      Author: heavey
 */

#ifndef UTIL_H_
#define UTIL_H_

/**
 * write lock
 * @return: 0 on success,  <0 and (errno == EACCES || errno == EAGAIN) is locked file
 */
int lockfile(int fd);

/**
 * open and set write lock
 * @return: return fd, -1 on error, -2 on locked file(failed to set lock)
 */
int open_file_lock(const char *file);

/**
 * write_pid_lock - lock file and write pid
 * @return: return fd, -1 on locked file, -2 on error
 */
int write_pid_lock(const char *file);

#endif /* UTIL_H_ */
