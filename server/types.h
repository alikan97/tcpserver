#ifndef __TYPES_H__
#define __TYPES_H__

#include <threads.h>
#include <semaphore.h>
#include <stdbool.h>

#define MAX_CLIENTS 10 
#define MAX_MESSAGE_LEN 1024
#define CHANNELS_LEN 64
#define BACKLOG 1024 // Total messages capacity per channel
#define BUFFER_LEN 156

typedef struct Message {
    char message[MAX_MESSAGE_LEN];
    time_t timestamp;
} message_t;

typedef struct threadManager {
    pid_t thread_id;
    char command[BUFFER_LEN];
} threadManager_t;

typedef struct clientArgs { // Argument struct for the clients, eg COMMAND ARG MESSAGE
    int argCount;
    char command[BUFFER_LEN];
    char argument[BUFFER_LEN];
    char message[MAX_MESSAGE_LEN];
} clientArgs_t;

typedef struct client { // Client details
    char name[70];
    int sock; // Client socket number
    int channels[CHANNELS_LEN];
    pthread_mutex_t local_mutex;
    bool keepAlive;
    size_t readSize;
    threadManager_t clientThreads[MAX_CLIENTS];
    int clientCount;
    clientArgs_t args;
    bool isFree;
} client_t;

typedef struct sharedMemory {
    int backlogPosition[CHANNELS_LEN]; 
    int channelReaderCount[CHANNELS_LEN];
    sem_t channelRead_sem[CHANNELS_LEN];
    sem_t channelWrite_sem[CHANNELS_LEN];
    message_t channels[CHANNELS_LEN][BACKLOG];
    client_t client[MAX_CLIENTS];
} sharedMemory_t;

#endif /*__TYPES_H__*/