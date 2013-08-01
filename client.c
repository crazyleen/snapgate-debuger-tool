#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"
#include "client_functions.h"

void print_usage(char *progname) {
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "list usb device:\n");
	fprintf(stderr, "\t%s <ip address> list\n", progname);
	fprintf(stderr, "start gdb proxy:\n");
	fprintf(stderr, "\t%s <ip address> <usb serial> <port>\n", progname);
	fprintf(stderr, "download program and start gdb proxy:\n");
	fprintf(stderr, "\t%s <ip address> <usb serial> <port> <filename>\n", progname);
	fprintf(stderr, "download program:\n");
	fprintf(stderr, "\t%s <ip address> <usb serial> <port> <filename> prog\n", progname);
}

typedef enum {
	CMD_LIST = 1, CMD_PROG, CMD_GDB
} CLIENT_COMMOND_TYPE;

struct client_commond {
	CLIENT_COMMOND_TYPE type;
	char *ip;
	char *file;
	char *serial;
	int port;
};

int main(int argc, char* argv[]) {
	struct client_commond cmd;
	memset(&cmd, 0, sizeof(cmd));
	struct sockaddr_in sin_server;
	int sfd_client;
	int x = 0;
	size_t size_sockaddr = sizeof(struct sockaddr);
	struct packet chp;

	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	if (argc == 3 && strcmp(argv[2], "list") == 0) {
		printf("list usb device...\n");
		cmd.type = CMD_LIST;
		cmd.ip = argv[1];
	} else if (argc == 4) {
		cmd.type = CMD_GDB;
		cmd.ip = argv[1];
		cmd.serial = argv[2];
		cmd.port = atoi(argv[3]);
	} else if (argc == 5) {
		cmd.type = CMD_GDB;
		cmd.ip = argv[1];
		cmd.serial = argv[2];
		cmd.port = atoi(argv[3]);
		cmd.file = argv[4];
	} else if (argc == 6 && strcmp(argv[5], "prog") == 0) {
		cmd.type = CMD_PROG;
		cmd.ip = argv[1];
		cmd.serial = argv[2];
		cmd.port = atoi(argv[3]);
		cmd.file = argv[4];
	} else {
		print_usage(argv[0]);
		exit(1);
	}

	if ((sfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		er("socket()", x);

	memset((char*) &sin_server, 0, sizeof(struct sockaddr_in));
	sin_server.sin_family = AF_INET;
	sin_server.sin_addr.s_addr = inet_addr(cmd.ip);
	sin_server.sin_port = htons(PORTSERVER);

	if ((x = connect(sfd_client, (struct sockaddr*) &sin_server, size_sockaddr)) < 0) {
		perror("connect error");
		close(sfd_client);
		return EXIT_FAILURE;
	}

	printf("Attempting communication with server @ %s:%d...\n\n", cmd.ip, PORTSERVER);
	//END: initialization

	clear_packet(&chp);
	chp.conid = -1;
	//printf("ip(%s) serial(%s) Port(%d) file(%s)\n", cmd.ip, cmd.serial, cmd.port, cmd.file);
	switch (cmd.type) {
	case CMD_LIST:
		client_command_list(sfd_client, &chp);
		break;
	case CMD_GDB:
		if (cmd.file != NULL)
			client_command_put(sfd_client, &chp, cmd.file);
		client_command_gdb(sfd_client, &chp, cmd.serial, cmd.port, cmd.file);
		break;
	case CMD_PROG:
		client_command_put(sfd_client, &chp, cmd.file);
		client_command_prog(sfd_client, &chp, cmd.serial, cmd.file);
		break;
	default:
		break;
	}
	close(sfd_client);
	return 0;
}

