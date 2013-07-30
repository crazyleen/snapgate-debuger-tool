#ifndef PACKET_H_
#define PACKET_H_

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define PORTSERVER	6709
#define CONTROLPORT	PORTSERVER
#define DATAPORT	(PORTSERVER + 1)

enum TYPE {
	REQU, DONE, INFO, TERM, DATA, EOT
};

#define	NP		0
#define	HP		1

#define	er(e, x)					\
	do						\
	{						\
		perror("ERROR IN: " #e "\n");		\
		fprintf(stderr, "%d\n", x);		\
		pthread_exit("error"); \
	}						\
	while(0)

#define LENBUFFER	496		// so as to make the whole packet well-rounded ( = 512 bytes)
struct packet {
	int32_t conid;
	int32_t type;
	int32_t comid;
	int32_t datalen;
	char buffer[LENBUFFER];
}__attribute__((__packed__));

void clear_packet(struct packet*);

struct packet* ntohp(struct packet*);
struct packet* htonp(struct packet*);

void printpacket(struct packet*, int);

/**
 * if error occur, pthread_exit
 */
void send_packet(int sfd, struct packet* data);

/**
 * 0 on success, or -1 for errors.
 */
int send_packet_ret(int sfd, struct packet* hp);

/**
 * if error occur, pthread_exit
 */
void recv_packet(int sfd, struct packet* pkt);

/**
 * 0 on success, or -1 for errors.
 */
int recv_packet_ret(int sfd, struct packet* pkt);

#define NCOMMANDS 19
enum COMMAND {
	GET = 1, PUT, EXIT, PROG, GDB, PROG_GDB, LIST, CONID
};

#endif

