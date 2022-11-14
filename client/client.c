#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_MESSAGE_LEN 1024

pthread_t commandThread, serverThread;
static volatile int keepAlive = 1;
int socketID;

void SIGINT_handler(int sigType) {
    if (sigType == SIGINT) {
        keepAlive = 0;
    }
}

void *commands(void)
{
    struct sigaction signalAction;
    signalAction.sa_flags = 0;
    signalAction.sa_handler = SIGINT_handler;
    sigemptyset(&signalAction.sa_mask);
    sigfillset(&signalAction.sa_mask);
    if (sigaction(SIGINT, &signalAction, NULL) < 0) {
        perror("Error handling SIGINT.\n");
    }

    while (keepAlive) {
        fd_set readFD, writeFD, exceptFD;
        FD_ZERO(&readFD);
        FD_ZERO(&writeFD);
        FD_ZERO(&exceptFD);

        FD_SET(STDIN_FILENO, &readFD);

        struct timeval selectTimeout;
        selectTimeout.tv_sec = 1;
        selectTimeout.tv_usec = 500000;

        char userInput[MAX_MESSAGE_LEN] = {0};
        char command[120] = {0};
        size_t userInputSize = MAX_MESSAGE_LEN;

        if (select(STDIN_FILENO+1, &readFD, &writeFD, &exceptFD, &selectTimeout) == 1) {
            fgets(userInput, userInputSize, stdin);
            sscanf(userInput, "%s ", command);
            write(socketID, userInput, strlen(userInput));
        }
    }

    return 0;
}

void *serverResponse(void) {
    struct sigaction signalAction;
    signalAction.sa_flags = 0;
    signalAction.sa_handler = SIGINT_handler;
    sigemptyset(&signalAction.sa_mask);
    sigfillset(&signalAction.sa_mask);
    if (sigaction(SIGINT, &signalAction, NULL) < 0) {
        perror("Error handling SIGINT.\n");
    }

    while (keepAlive)
    {
        fd_set readFD, writeFD, exceptFD;
        FD_ZERO(&readFD);
        FD_ZERO(&writeFD);
        FD_ZERO(&exceptFD);

        FD_SET(socketID, &readFD);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 500000;

        char serverResponse[MAX_MESSAGE_LEN] = {0};

        if (select(socketID+1, &readFD, &writeFD, &exceptFD, &timeout) >= 1)
        {
            size_t connectionStatus = read(socketID, serverResponse, MAX_MESSAGE_LEN);

            if(connectionStatus > 0 && strcmp(serverResponse, "") != 0) {
                printf("%s", serverResponse);
                memset(serverResponse, 0, sizeof(serverResponse));
            } else if (connectionStatus == 0) {
                printf("Connection Closed");
                keepAlive = 0;
            }
        }
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serverConnection;

    if ((socketID = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error \n");
        return EXIT_FAILURE;
    }

    int port = argc > 2 ? atoi(argv[2]) : 6789;

    serverConnection.sin_family = AF_INET;
    serverConnection.sin_port = htons(port);

    if(inet_pton(AF_INET, argv[1], &serverConnection.sin_addr) <= 0)
    {
        printf("Invalid address\n");
        return EXIT_FAILURE;
    }

    if (connect(socketID, (struct sockaddr *)&serverConnection, sizeof(serverConnection)) < 0)
    {
        printf("Connection Failed \n");
        return EXIT_FAILURE;
    }

    pthread_create(&commandThread, NULL, (void *(*)(void *)) commands, NULL);
    pthread_create(&serverThread, NULL, (void *(*)(void *)) serverResponse, NULL);

    pthread_join(commandThread, NULL);
    pthread_join(serverThread, NULL);

    return EXIT_SUCCESS;
}
