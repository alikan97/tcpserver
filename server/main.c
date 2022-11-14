#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "globals.h"
#include "types.h"
#include "Clients.h"
#include "message.h"

static volatile bool keepRunning = true;
static volatile bool SIGTERMED = false;

void handleSigInt(int signalType) {
    SIGTERMED = true;
    keepRunning = false;
}

void initialGlobalStorage(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        globalStorage->client[i].isFree = true;
    }
    for (int i = 0; i < CHANNELS_LEN; i++) {
        sem_init(&globalStorage->channelRead_sem[i],0,1); // initialize the semaphores to be shared between threads of a process with initial value of 1
        sem_init(&globalStorage->channelWrite_sem[i],0,1); // initialize the semaphores to be shared between threads of a process with initial value of 1
    }
    for (int i = 0; i < CHANNELS_LEN; i++){
        globalStorage->backlogPosition[i] = 0;  // Set the backlog position to 0
        globalStorage->channelReaderCount[i] = 0;
    }
}

// Limit connections from client
int freeArraySlot(int clientSocket, struct sockaddr_in clientAddress) {
    int clientNo = -1;
    while (clientNo == -1) {
        printf("Client %s connected on socket %d\n", inet_ntoa(clientAddress.sin_addr), clientSocket);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (globalStorage->client[i].isFree) {
                clientNo = i;
            }
        }
        if (clientNo == -1) {
            char *message = "Server is full\n";
            printf("Client %s rejected as capacity is currently full\n", inet_ntoa(clientAddress.sin_addr));
            send(clientSocket, message, strlen(message), 0);
            break;
        }
    }
    return clientNo;
}

int main(int argc, const char *argv[]) {
    int masterSocket;
    struct sockaddr_in serverAddress;
    socklen_t serverAddressSize = sizeof(serverAddress);

    int port = argc > 1 ? atoi(argv[1]) : 6789;
    if (!port) {
        fprintf(stdout, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    memset(&serverAddress, 0, serverAddressSize);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if ((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Failed to initiate socket. \n");
        return -1;
    }

    if (bind(masterSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Failed to bind socket. \n");
        return -1;
    }

    if (listen(masterSocket, MAX_CLIENTS) == -1) {
        printf("Failed to listen on socket. \n");
        return -1;
    }
    // Allocate memory for globalStorage
    globalStorage = mmap(NULL, sizeof(sharedMemory_t), PROT_WRITE | PROT_READ, MAP_SHARED| MAP_ANONYMOUS, -1, 0);
    if (globalStorage == MAP_FAILED) {
        perror("Unable to configure shared memory.\n");
        return -1;
    }
    initialGlobalStorage();

    printf("Server started at %d \n", port);

    struct sigaction signalAction;
    signalAction.sa_flags = 0;
    signalAction.sa_handler = handleSigInt;
    sigemptyset(&signalAction.sa_mask);
    sigfillset(&signalAction.sa_mask);
    if (sigaction(SIGINT, &signalAction, NULL) < 0) {
        perror("Error handling SIGINT");
    }

    while (keepRunning) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        memset(&clientAddress, 0, clientAddressSize);

        int clientSocket = accept(masterSocket, (struct sockaddr *) &clientAddress, &clientAddressSize);
        if (clientSocket == -1) {
            if (!SIGTERMED) {
                printf("Failed to accept client on socket %d.\n", clientSocket);
            }
            continue;
        } else {
            pid_t clientPID = fork(); // Create a new thread for each client
            if(clientPID == -1) {
                perror("Failed to create new thread\n");
                continue;
            } else if (clientPID == 0) {
                int clientNo = freeArraySlot(clientSocket, clientAddress);
                if (clientNo == -1) {
                    char rejectionMsg[MAX_MESSAGE_LEN];
                    sprintf(rejectionMsg, "Closing connection, you have been rejected\n");
                    sendMessageClient(clientNo, rejectionMsg);
                    return -1;
                }
                globalStorage->client[clientNo].sock = clientSocket;
                for (int i = 0; i < CHANNELS_LEN; i++) {
                    globalStorage->client[clientNo].channels[i] = -1;
                }
                globalStorage->client[clientNo].keepAlive = true;
                globalStorage->client[clientNo].readSize = 0;
                globalStorage->client[clientNo].clientCount = 0;
                globalStorage->client[clientNo].args.argCount = 0;
                globalStorage->client[clientNo].isFree = false;
                
                char welcomeMessage[MAX_MESSAGE_LEN];
                sprintf(welcomeMessage, "Welcome, %s, your client ID is: %d\n",inet_ntoa(clientAddress.sin_addr), clientNo);
                sendMessageClient(clientNo, welcomeMessage);

                acceptPayload(clientNo);

                printf("Disconnecting client %s on socket %d\n", inet_ntoa(clientAddress.sin_addr), clientSocket);
                return 0;
            } else {
                close(clientSocket);
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        globalStorage->client[i].keepAlive = false;
    }

    // Remove the global Storage
    int unmap_result = munmap(globalStorage, sizeof(sharedMemory_t));
    if (unmap_result == -1) {
        perror("Failed to unmap shared memory.\n");
        return -1;
    }

    return 0;
}