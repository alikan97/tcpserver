#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>
#include <mutex>
#include <queue>
#include "List.h"
#include <algorithm>
// Use when reading from raspberrypi GPIO Ports
// #include <wiringPi.h>
using namespace std;

std::mutex globalMtx;

// TODO
// Create central data store
// Multithreading / multiClient
// Resource allocation / memory mgmt
// Source data from sensors

void interrupt_handler(int clientSocket)
{
    std::cout << "Recevied Intertupt: Shutting Down!..." << endl;
    sleep(1);
    close(clientSocket);
    exit(0);
}

void handleConnection(int clientSocket, sockaddr_in client, char *host, char *svc, std::queue<std::string> *messageQueue);

int main()
{
    int PORT = 5005;

    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr << "Can't create socket" << endl;
        return -1;
    }

    // Bind the socket to an IP
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1)
    {
        cerr << "Can't bind to IP/Port" << endl;
        return -2;
    }

    // Mark the scoket for listening
    if (listen(listening, SOMAXCONN) == -1)
    {
        cerr << "Can't listen";
        return -3;
    }

    cout << "Server has started listening on Port: " << PORT << endl;
    signal(SIGINT, interrupt_handler); // Handle System Terminations gracefully

    while (1)
    {
        // Accept a caller
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);
        char host[NI_MAXHOST];
        char svc[NI_MAXSERV];

        std::queue<std::string> messageQueue;
        
        int clientSocket;

        while ((clientSocket = accept(listening, (sockaddr *)&client, &clientSize)) != -1)
        {
            std::thread th1(handleConnection, clientSocket, client, std::ref(host), std::ref(svc), &messageQueue);
            th1.detach();
        }
        close(clientSocket);
    }
    close(listening);
    return 0;
}

void handleConnection(int clientSocket, sockaddr_in client, char *host, char *svc, std::queue<std::string> *messageQueue)
{
    std::cout << "New thread initiated..." << std::endl;

    char newClient[26] = "Welcome to the server!";
    send(clientSocket, newClient, 21, 0);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if (result)
    {
        cout << host << " connected on " << svc << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }
    char buffer[4096];

    while (1)
    {   
        memset(buffer, 0, 4096);
        int bytesRecv = recv(clientSocket, buffer, 4096, 0);
        if (bytesRecv == -1)
        {
            std::cout << "Connection issue" << std::endl;
            break;
        }
        if (bytesRecv == 0)
        {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        std::string receivedString = string(buffer);
        cout << receivedString.length() << endl;
        if (receivedString.find("Read", 0) != string::npos)
        {
            cout << "HITT" << endl;
            while(!messageQueue->empty())
            {
                int bytes = send(clientSocket, messageQueue->front().c_str(), messageQueue->front().size(), 0);
                messageQueue->pop();
            }
        } else
        {
            messageQueue->push(receivedString);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    close(clientSocket);
}


