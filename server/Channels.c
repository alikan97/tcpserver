#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "types.h"
#include "globals.h"
#include "channels.h"

void channelPushLeft(int channelNum)
{
    for(int i = 0; i < BACKLOG-2; i++)
    {
        strcpy(globalStorage->channels[channelNum][i].message, globalStorage->channels[channelNum][i+1].message);
        globalStorage->channels[channelNum][i].timestamp = globalStorage->channels[channelNum][i+1].timestamp;
    }
}

char *readFromChannel(int channelNum, int position) {

    char *message = malloc(MAX_MESSAGE_LEN);
    sem_wait(&globalStorage->channelRead_sem[channelNum]);

    globalStorage->channelReaderCount[channelNum] += 1;

    if (globalStorage->channelReaderCount[channelNum] == 1) {
        sem_wait(&globalStorage->channelWrite_sem[channelNum]);
    }

    sem_post(&globalStorage->channelRead_sem[channelNum]);
    strcpy(message, globalStorage->channels[channelNum][position].message);
    sem_wait(&globalStorage->channelRead_sem[channelNum]);

    globalStorage->channelReaderCount[channelNum] -= 1;
    if (globalStorage->channelReaderCount[channelNum] == 0) {
        sem_post(&globalStorage->channelWrite_sem[channelNum]);
    }

    sem_post(&globalStorage->channelRead_sem[channelNum]);

    return message;
}


/*
    Add a message to the channel, if channel is full then this will start to remove older messages and push newer messages.
    Critical sections are guarded by semaphore
*/
void addToChannel(char *message, int channelNum) {
    sem_wait(&globalStorage->channelWrite_sem[channelNum]);
    int position = globalStorage->backlogPosition[channelNum];

    if (position == BACKLOG-1) { // in case channel reaches its limit
        memset(globalStorage->channels[channelNum][0].message, 0, MAX_MESSAGE_LEN);
        globalStorage->channels[channelNum][0].timestamp = (time_t) (-1);
        channelPushLeft(channelNum);
        strcpy(globalStorage->channels[channelNum][position].message, message);
        globalStorage->channels[channelNum][position].timestamp = time(NULL);
    } else {
        strcpy(globalStorage->channels[channelNum][position].message, message);
        globalStorage->channels[channelNum][position].timestamp = time(NULL);
        globalStorage->backlogPosition[channelNum]++;
    }

    sem_post(&globalStorage->channelWrite_sem[channelNum]);
}