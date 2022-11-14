#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#include "globals.h"
#include "types.h"
#include "Clients.h"

void sendMessageClient(int client_position, char *message)
{
    pthread_mutex_lock(&globalStorage->client[client_position].local_mutex);
    write(globalStorage->client[client_position].sock, message, strlen(message));
    pthread_mutex_unlock(&globalStorage->client[client_position].local_mutex);
}

int isClientSubscribed(int client_position)
{
    for (int i = 0; i < CHANNELS_LEN; i++)
    {
        if (globalStorage->client[client_position].channels[i] >= 0)
        {
            return 0;
        }
    }
    return 1;
}

void subscribe(int channelNum, int client_position)
{
    if (channelNum >= 0 && channelNum < CHANNELS_LEN)
    {
        globalStorage->client[client_position].channels[channelNum] = globalStorage->backlogPosition[channelNum];
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "Subscribed to channel %i \n", channelNum);
        sendMessageClient(client_position, message);
    }
    else
    {
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "Invalid channel: %i \n", channelNum);
        sendMessageClient(client_position, message);
    }
}

void unsubscribe(int channelNum, int client_position)
{
    if (channelNum >= 0 && channelNum < CHANNELS_LEN)
    {
        globalStorage->client[client_position].channels[channelNum] = -1;
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "Unsubscribed from channel %i \n", channelNum);
        sendMessageClient(client_position, message);
    }
    else
    {
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "Invalid channel: %i \n", channelNum);
        sendMessageClient(client_position, message);
    }
}
