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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "util.h"
#include "packet.h"
#include "file_transfer.h"
#include "server_functions.h"

#define DEBUG_FUNCTIONS_Trace
#define DEBUG_FUNCTIONS_Info
#include "debug.h"

#define MAX_CLIENT 32

struct client_args {
	int sockfd;
	struct sockaddr_in addr;
};

static sem_t client_sem; //max connections
static int sockfd = 0; //server socket

/**
 * malloc args for thread
 * return args, NULL for malloc error
 */
static void *thread_alloc_args(struct client_args *client) {
	void *args = malloc(sizeof(struct client_args));
	if (args == NULL) {
		Error("malloc");
		return NULL;
	}
	memcpy(args, client, sizeof(struct client_args));
	return args;
}

static void thread_free_args(void *args) {
	if (args != NULL)
		free(args);
}

static void thread_client_cleanup(void *args) {
	struct client_args *client = (struct client_args*) args;
	Info("client fd(%d) exit", client->sockfd);
	//client thread exit
	if (sem_post(&client_sem) < 0) {
		perror("sem_wait");
	}
	close(client->sockfd);
	thread_free_args(args);
}

static void *thread_client(void *args) {
	struct client_args *client = (struct client_args*) args;
	pthread_cleanup_push(thread_client_cleanup, args);

	serve_client(client->sockfd);

	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

static void at_exit_cleanup(void) {
	sem_destroy(&client_sem);
	close(sockfd);
}

/**
 * become a TCP server, bind to port, listen for maxconnect connections
 * return socket fd, -1 on error
 */
int socket_server(int port, int maxconnect) {
	int sockfd;
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
		perror("listen");
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, maxconnect) < 0) {
		perror("listen");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int main(int argc, char**argv) {
	const char *pidfile = "/var/run/ftp_multithread_server.pid";
	if (write_pid_lock(pidfile) < 0) {
		printf("%s: already running\n", argv[0]);
		return 0;
	}

	sockfd = socket_server(PORTSERVER, MAX_CLIENT);
	if (sockfd < 0) {
		Error("socket server");
		return -1;
	}

	if (sem_init(&client_sem, 0, MAX_CLIENT) < 0) {
		close(sockfd);
		Error("exit now");
		return -1;
	}

	signal(SIGPIPE, SIG_IGN);
	atexit(at_exit_cleanup);
	while (1) {
		size_t client_addr_len;
		struct client_args client;
		pthread_t thread;
		int ret;

		if (sem_wait(&client_sem) < 0) {
			perror("sem_wait");
			break;
		}

		bzero(&client, sizeof(client));
		client_addr_len = sizeof(client.addr);
		client.sockfd = accept(sockfd, (struct sockaddr *) &client.addr, &client_addr_len);
		if (client.sockfd < 0) {
			Error("accept");
			break;
		}
		//client count

		Info( "client connect from %s, fd(%d)", inet_ntoa(client.addr.sin_addr), client.sockfd);
		void *thread_args = thread_alloc_args(&client);
		ret = pthread_create(&thread, NULL, thread_client, thread_args);
		if (ret != 0) {
			Error("pthread_create");
			if (thread_args != NULL)
				free(thread_args);
			if (sem_post(&client_sem) < 0) {
				Error("sem_wait");
			}
			continue;
		}
		pthread_detach(thread);
	}

	return 0;
}

