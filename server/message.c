#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <stdbool.h>
#include <threads.h>
#include "globals.h"
#include "types.h"
#include "Clients.h"
#include "Channels.h"

static volatile bool localKeepRunning = true;

void handleSIGINT(int signalType) {
    (void) signalType;
    localKeepRunning = false;
}

void readAllChannels(int client_position) {
    int messagePos = -1;
    int channelPos = -1;

    if (isClientSubscribed(client_position)) {
        char *message = "Not subscribed to any channels\n";
        sendMessageClient(client_position, message);
    } else {
        for (int i = 0; i < CHANNELS_LEN; i++) {
            int messageMarker = globalStorage->client[client_position].channels[i];
            if (messageMarker >= 0) {
                if ((channelPos == -1 && messagePos == -1) && (globalStorage->channels[i][messageMarker].timestamp != 0 && globalStorage->channels[i][messageMarker].message[0] != 0)) {
                    channelPos = i;
                    messagePos = messageMarker;
                } else if (messagePos != -1 && channelPos != -1) {
                    if (globalStorage->channels[i][messageMarker].timestamp < globalStorage->channels[channelPos][messagePos].timestamp && globalStorage->channels[i][messageMarker].message[0] != 0) {
                        channelPos = i;
                        messagePos = messageMarker;
                    }
                }
            }
        }

        if (messagePos != -1 && channelPos != -1) {
            char message[MAX_MESSAGE_LEN];

            char *channelMessage = readFromChannel(channelPos, globalStorage->client[client_position].channels[channelPos]);
            sprintf(message, "%i:%s", channelPos, channelMessage);

            if (globalStorage->backlogPosition[channelPos] > globalStorage->client[client_position].channels[channelPos]) {
                globalStorage->client[client_position].channels[channelPos]++;
            }

            sendMessageClient(client_position, message);
        }
    }
}

int findPid(pid_t thread, int client_position) {
    for (int i = 0; i < globalStorage->client[client_position].clientCount; i++) {
        if (globalStorage->client[client_position].clientThreads[i].thread_id == thread) {
            return i;
        }
    }

    return -1;
}

void sendMessage(char *message, int channelNum, int client_position) {
    if (channelNum >= 0 && channelNum < CHANNELS_LEN) {
        char messageDupe[MAX_MESSAGE_LEN] = {0};
        strcpy(messageDupe, message);
        char *cleanedMessage;
        cleanedMessage = strchr(messageDupe, ' ');
        cleanedMessage++;
        cleanedMessage = strchr(cleanedMessage, ' ');
        cleanedMessage++;

        addToChannel(cleanedMessage, channelNum);
    } else {
        char errMessage[MAX_MESSAGE_LEN];
        sprintf(errMessage, "Invalid channel: %i \n", channelNum);
        sendMessageClient(client_position, errMessage);
    }
}

void readMessage(int channelNum, int client_position)
{
    if (channelNum >= 0 && channelNum <= CHANNELS_LEN-1){ 
        if (globalStorage->client[client_position].channels[channelNum] == -1) {
            char message[MAX_MESSAGE_LEN];
            sprintf(message, "Not subscribed to channel %i \n", channelNum);
            sendMessageClient(client_position, message);
        } else {
            char *message = readFromChannel(channelNum, globalStorage->client[client_position].channels[channelNum]);
            if (globalStorage->backlogPosition[channelNum] > globalStorage->client[client_position].channels[channelNum]) {
                globalStorage->client[client_position].channels[channelNum]++;
            }
            sendMessageClient(client_position, message);
        }
    } else {
        char message[MAX_MESSAGE_LEN];
        sprintf(message, "Invalid channel: %i \n", channelNum);
        sendMessageClient(client_position, message);
    }
}

void executePayload(int client_position) {
    char command[BUFFER_LEN];
    char argument[BUFFER_LEN];
    char message[MAX_MESSAGE_LEN];

    int arguments = globalStorage->client[client_position].args.argCount;
    strcpy(command, globalStorage->client[client_position].args.command);
    strcpy(argument, globalStorage->client[client_position].args.argument);
    strcpy(message, globalStorage->client[client_position].args.message);

    pid_t pidID = getpid();
    int threadPos = findPid(pidID, client_position);

    if (arguments > 2) {
        if (strcmp(command, "SEND") == 0) {
            int numChannel = atoi(argument);
            sendMessage(message, numChannel, client_position);
        } else {
            char *errorMessage = "Invalid Command \n";
            sendMessageClient(client_position, errorMessage);
        }
    } else if (arguments == 2) {
        if (strcmp(command, "SUB") == 0) {
            int numChannel = atoi(argument);
            subscribe(numChannel, client_position);
        } else if (strcmp(command, "UNSUB") == 0) {
            int numChannel = atoi(argument);
            unsubscribe(numChannel, client_position);
        } else if (strcmp(command, "NEXT") == 0) {
            int numChannel = atoi(argument);
            strcpy(globalStorage->client[client_position].clientThreads[threadPos].command, command);
            readMessage(numChannel, client_position);
            strcpy(globalStorage->client[client_position].clientThreads[threadPos].command, "");
        } else {
            char *message = "Invalid Command \n";
            sendMessageClient(client_position, message);
        }
    } else {
        if (strcmp(command, "NEXT") == 0) {
            strcpy(globalStorage->client[client_position].clientThreads[threadPos].command, command);
            readAllChannels(client_position);
            strcpy(globalStorage->client[client_position].clientThreads[threadPos].command, "");
        } else if (strcmp(command, "STOP") == 0) {
            for (int i = 0; i < globalStorage->client[client_position].clientCount; i++) {
                if (strcmp(globalStorage->client[client_position].clientThreads[i].command, "NEXT") == 0) {
                    kill(globalStorage->client[client_position].clientThreads[i].thread_id, SIGINT);
                }
            }
        } else {
            char *invalidMessage = "Invalid Command \n";
            sendMessageClient(client_position, invalidMessage);
        }
    }

    globalStorage->client[client_position].clientCount--;
}

void acceptPayload(int client_position) {
    char message[MAX_MESSAGE_LEN] = {0};

    signal(SIGINT, handleSIGINT);
    while (globalStorage->client[client_position].keepAlive && localKeepRunning) {
        fd_set readFD, writeFD, exceptFD;
        FD_ZERO(&readFD);
        FD_ZERO(&writeFD);
        FD_ZERO(&exceptFD);
        FD_SET(globalStorage->client[client_position].sock, &readFD);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(globalStorage->client[client_position].sock + 1, &readFD, &writeFD, &exceptFD, &timeout) >= 1) {
            pthread_mutex_lock(&globalStorage->client[client_position].local_mutex);
            globalStorage->client[client_position].readSize = read(globalStorage->client[client_position].sock, message, MAX_MESSAGE_LEN);
            pthread_mutex_unlock(&globalStorage->client[client_position].local_mutex);

            if (globalStorage->client[client_position].readSize > 0) {

                char command[BUFFER_LEN] = {0};
                char argument[BUFFER_LEN] = {0};
                char messageDump[MAX_MESSAGE_LEN] = {0};

                int splitArgs = sscanf(message, " %s %s %s ", command, argument, messageDump);

                globalStorage->client[client_position].args.argCount = splitArgs;
                strcpy(globalStorage->client[client_position].args.command, command);
                strcpy(globalStorage->client[client_position].args.argument, argument);
                strcpy(globalStorage->client[client_position].args.message, message);

                pid_t clientCommandPID = fork();
                if (clientCommandPID == -1) {
                    perror("Failed to fork client on socket %d.\n");
                    continue;
                } else if (clientCommandPID == 0) {
                    globalStorage->client[client_position].clientThreads[globalStorage->client[client_position].clientCount].thread_id = getpid();
                    globalStorage->client[client_position].clientCount++;
                    executePayload(client_position);

                    return;
                } else {
                    memset(message, 0, MAX_MESSAGE_LEN);
                    memset(messageDump, 0, BUFFER_LEN);
                    memset(command, 0, BUFFER_LEN);
                    memset(argument, 0, BUFFER_LEN);
                    memset(messageDump, 0, BUFFER_LEN);
                }
            } else if (globalStorage->client[client_position].readSize == 0) {
                globalStorage->client[client_position].keepAlive = false;
            }
        }
        sleep(1);
    }

    for (int i = 0; i < globalStorage->client[client_position].clientCount; i++) {
        kill(globalStorage->client[client_position].clientThreads[globalStorage->client[client_position].clientCount].thread_id, SIGKILL);
    }

    // Cleanup
    shutdown(globalStorage->client[client_position].sock, SHUT_WR);
    close(globalStorage->client[client_position].sock);
    globalStorage->client[client_position].isFree = true;
}