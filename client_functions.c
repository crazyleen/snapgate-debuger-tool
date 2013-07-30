#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "packet.h"
#include "file_transfer.h"
#include "client_functions.h"

void client_command_list(int sfd_client, struct packet* chp) {
	clear_packet(chp);
	chp->type = REQU;
	chp->comid = LIST;
	send_packet(sfd_client, chp);
	while (chp->type != EOT) {
		if (chp->type == DATA && chp->comid == LIST && strlen(chp->buffer))
			printf("\t%s", chp->buffer);

		recv_packet(sfd_client, chp);
	}
}

void client_command_prog(int sfd_client, struct packet* chp, char *serial, char *filename) {
	clear_packet(chp);
	chp->type = REQU;
	chp->comid = PROG;
	snprintf(chp->buffer, sizeof(chp->buffer), "%s %s", serial, filename);
	send_packet(sfd_client, chp);
	while (chp->type != EOT) {
		if (chp->type == DATA && chp->comid == PROG && strlen(chp->buffer))
			printf("\t%s", chp->buffer);

		recv_packet(sfd_client, chp);
	}
}

void client_command_gdb(int sfd_client, struct packet* chp, char *serial, int port, char *filename) {
	clear_packet(chp);
	chp->type = REQU;
	chp->comid = GDB;
	snprintf(chp->buffer, sizeof(chp->buffer), "%s %d %s", serial, port, filename);
	send_packet(sfd_client, chp);
	while (chp->type != EOT) {
		if (chp->type == DATA && chp->comid == GDB && strlen(chp->buffer))
			printf("\t%s", chp->buffer);
		if (chp->type == INFO && chp->comid == GDB && strlen(chp->buffer))
			printf("\t%s\n", chp->buffer);
		recv_packet(sfd_client, chp);
	}
}

void client_command_get(int sfd_client, struct packet* chp, char* filename) {
	FILE* f = fopen(filename, "wb");
	if (!f) {
		fprintf(stderr, "File could not be opened for writing. Aborting...\n");
		return;
	}
	clear_packet(chp);
	chp->type = REQU;
	chp->comid = GET;
	strcpy(chp->buffer, filename);
	send_packet(sfd_client, chp);
	recv_packet(sfd_client, chp);
	//printpacket(chp, HP);
	if (chp->type == INFO && chp->comid == GET && strlen(chp->buffer)) {
		printf("\t%s\n", chp->buffer);
		receive_file(sfd_client, chp, f);
		fclose(f);
	} else
		fprintf(stderr, "Error getting remote file : <%s>\n", filename);
}

void client_command_put(int sfd_client, struct packet* chp, char* filename) {
	FILE* f = fopen(filename, "rb"); // Yo!
	if (!f) {
		fprintf(stderr, "File could not be opened for reading. Aborting...\n");
		return;
	}
	clear_packet(chp);
	chp->type = REQU;
	chp->comid = PUT;
	strcpy(chp->buffer, filename);
	send_packet(sfd_client, chp);
	recv_packet(sfd_client, chp);
	//printpacket(chp, HP);
	if (chp->type == INFO && chp->comid == PUT && strlen(chp->buffer)) {
		printf("\t%s\n", chp->buffer);
		chp->type = DATA;
		send_file(sfd_client, chp, f);
		fclose(f);
	} else
		fprintf(stderr, "Error sending file.\n");
	send_EOT(sfd_client, chp);
}

