#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "packet.h"
#include "file_transfer.h"
#include "server_functions.h"
#include "mspdebug.h"

//#define DEBUG_FUNCTIONS_Trace
#define DEBUG_FUNCTIONS_Info
#include "debug.h"

static char *get_file_path(char *filepath, int len, int sfd_client, char *filename) {
	const char *TMP_DIR = "/tmp";
	snprintf(filepath, len, "%s/%d-%s", TMP_DIR, sfd_client, filename);
	Trace("filepath=%s", filepath);
	return filepath;
}

static void server_tmpfile_replace(void *file) {
	static char tmp_file[1024] = "";
	Trace("new file(%s), rm %s", (char *)file, tmp_file);
	if (strlen(tmp_file) > 0)
		unlink(tmp_file);

	if ((char *) file == NULL)
		tmp_file[0] = '\0';
	else
		snprintf(tmp_file, sizeof(tmp_file), "%s", (char *) file);
}

void server_command_get(int sfd_client, struct packet* shp) {
	FILE* f = fopen(shp->buffer, "rb"); // Yo!
	shp->type = INFO;
	shp->comid = GET;
	strcpy(shp->buffer, f ? "File found; processing" : "Error opening file.");
	//printpacket(shp, HP);
	send_packet(sfd_client, shp);
	if (f) {
		shp->type = DATA;
		send_file(sfd_client, shp, f);
		fclose(f);
	}
	send_EOT(sfd_client, shp);
}

void server_command_put(int sfd_client, struct packet* shp) {
	char filepath[1024];
	get_file_path(filepath, sizeof(filepath), sfd_client, shp->buffer);
	server_tmpfile_replace(filepath);
	FILE* f = fopen(filepath, "wb");
	shp->type = INFO;
	shp->comid = PUT;
	strcpy(shp->buffer, f ? "Everything in order; processing" : "Error opening file for writing on server side.");
	//printpacket(shp, HP);
	send_packet(sfd_client, shp);
	if (f) {
		receive_file(sfd_client, shp, f);
		fclose(f);
	}
}

void server_command_prog(int sfd_client, struct packet* shp) {
	int access_ok;
	char filename[128] = "";
	char filepath[1024] = "";
	char serial[64] = "";

	sscanf(shp->buffer, "%60s %100s", serial, filename);
	get_file_path(filepath, sizeof(filepath), sfd_client, filename);
	access_ok = access(filepath, R_OK);
	shp->type = INFO;
	shp->comid = PROG;
	strcpy(shp->buffer, access_ok == 0 ? "Everything in order; processing" : "Error opening file for programming device.");
	send_packet(sfd_client, shp);
	if (access_ok == 0) {
		FILE *fp = NULL;
		shp->type = DATA;
		shp->comid = PROG;
		fp = mspdebug_prog_start(serial, filepath);
		while (fgets(shp->buffer, sizeof(shp->buffer), fp)) {
			send_packet(sfd_client, shp);
		}
		mspdebug_prog_stop(fp);
	}

	send_EOT(sfd_client, shp);
}

void server_command_list(int sfd_client, struct packet* shp) {
	shp->type = DATA;
	FILE *fp = mspdebug_list_device_start();
	while (fgets(shp->buffer, sizeof(shp->buffer), fp)) {
		send_packet(sfd_client, shp);
	}
	mspdebug_list_device_stop(fp);
	send_EOT(sfd_client, shp);
}

static void server_commond_gdb_stop(void *args) {
	FILE *fp = (FILE *) args;
	mspdebug_prog_gdb_stop(fp);
}

void server_command_gdb(int sfd_client, struct packet* shp) {
	int access_ok;
	char filepath[1024];
	char filename[128] = "";
	char serial[64] = "";
	int port = 0;
	sscanf(shp->buffer, "%60s %d %100s", serial, &port, filename);
	get_file_path(filepath, sizeof(filepath), sfd_client, filename);
	access_ok = access(filepath, R_OK);
	shp->type = INFO;
	shp->comid = GDB;
	strcpy(shp->buffer, access_ok == 0 ? "Access file OK" : "Error accessing file for programming device.");
	send_packet(sfd_client, shp);
	if (access_ok != 0) {
		send_EOT(sfd_client, shp);
		return;
	}
	Trace("serial(%s), port(%d) file(%s)", serial, port, filepath);

	int fd;
	FILE *fp = NULL;
	Trace("serial(%s), port(%d) file(%s)", serial, port, filepath);
	fp = mspdebug_prog_gdb_start(serial, port, filepath);
	shp->type = INFO;
	shp->comid = GDB;
	strcpy(shp->buffer, fp != NULL ? "Start GDB PROXY..." : "Error in starting  GDB PROXY");
	send_packet(sfd_client, shp);
	if (fp == NULL) {
		send_EOT(sfd_client, shp);
		return;
	}

	fd = fileno(fp);
	pthread_cleanup_push(server_commond_gdb_stop, fp);
	while (1) {
		int ret;
		fd_set input;
		struct timeval tv;
		FD_ZERO(&input);
		FD_SET(fd, &input);
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		ret = select(fd + 1, &input, NULL, NULL, &tv);
		if (ret > 0) {
			if (FD_ISSET(fd, &input)) {
				//read output
				shp->type = DATA;
				shp->comid = GDB;
				if (!fgets(shp->buffer, sizeof(shp->buffer), fp))
					break;
				send_packet(sfd_client, shp);
			}
		} else if (ret == 0) {
			//check if client exit
			shp->buffer[0] = '\0';
			send_packet(sfd_client, shp);
		} else if (ret < 0) {
			perror("select");
			break;
		}
	}
	pthread_cleanup_pop(1);
	send_EOT(sfd_client, shp);
}

void serve_client(int sfd_client) {
	int connection_id;
	struct packet shp;
	connection_id = sfd_client; //sockfd is unique
	Trace("conid = %d", connection_id);
	pthread_cleanup_push(server_tmpfile_replace, NULL);
	while (1) {
		if (recv_packet_ret(sfd_client, &shp) < 0) {
			fprintf(stderr, "client ID(%d) fd(%d) closed/terminated. closing connection.\n", connection_id, sfd_client);
			break;
		}

		if (shp.type == TERM)
			break;

		shp.conid = connection_id;

		if (shp.type == REQU) {
			switch (shp.comid) {
			case PUT:
				server_command_put(sfd_client, &shp);
				break;
			case PROG:
				server_command_prog(sfd_client, &shp);
				break;
			case GDB:
				server_command_gdb(sfd_client, &shp);
				break;
			case LIST:
				server_command_list(sfd_client, &shp);
				break;
			default:
				// print error
				break;
			}
		} else {
			//show error, send TERM and break
			fprintf(stderr, "packet incomprihensible. closing connection.\n");
			send_TERM(sfd_client, &shp);
			break;
		}
	}

	close(sfd_client);
	fflush(stdout);
	pthread_cleanup_pop(1);
}

