#ifndef SERVER_FUNCTIONS_H_
#define SERVER_FUNCTIONS_H_

void server_command_get(int sfd_client, struct packet* shp);
void server_command_put(int sfd_client, struct packet* shp);
void server_command_prog(int sfd_client, struct packet* shp);
void server_command_gdb(int sfd_client, struct packet* shp);
void server_command_list(int sfd_client, struct packet* shp);

void serve_client(int sfd_client);

#endif
