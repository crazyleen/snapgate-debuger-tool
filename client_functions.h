#ifndef CLIENT_FUNCTIONS_H_
#define CLIENT_FUNCTIONS_H_

void client_command_get(int, struct packet*, char*);
void client_command_put(int, struct packet*, char*);
void client_command_prog(int sfd_client, struct packet* chp, char *serial, char *filename);
void client_command_gdb(int sfd_client, struct packet* chp, char *serial, int port, char *filename);
void client_command_list(int sfd_client, struct packet* chp);

#endif
