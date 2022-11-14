#ifndef __CLIENTS_H__
#define __CLIENTS_H__

void unsubscribe(int channelNum, int client_position);
void sendMessageClient(int client_position, char *message);
void subscribe(int channelNum, int client_position);
int isClientSubscribed(int client_position);

#endif
