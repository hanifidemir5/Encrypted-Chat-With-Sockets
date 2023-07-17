#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to handle receiving data from the server
unsigned __stdcall ReceiveThread(void* arg) {
    SOCKET clientSocket = *(SOCKET*)arg;
    char buffer[BUFFER_SIZE] = { 0 };
    char key[BUFFER_SIZE];

    if (recv(clientSocket, buffer, BUFFER_SIZE - 1, 0) == SOCKET_ERROR) {
        perror("receive failed");
        exit(EXIT_FAILURE);
    }

    // Store the received key-value pair for further processing
    strncpy(key, buffer, BUFFER_SIZE);


    printf("Received key: %s\n", key);

    memset(buffer, 0, BUFFER_SIZE);

    while (1) {
        // Receive response from the server
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            perror("receive failed");
            exit(EXIT_FAILURE);
        }

        printf("Server response: %s\n", buffer);

        // Clear the buffer
        memset(buffer, 0, BUFFER_SIZE);
    }

    _endthreadex(0);
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serv_addr;
    HANDLE receiveThread;
    unsigned threadID;
    char message[BUFFER_SIZE] = { 0 };

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    // Create a socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread for receiving data from the server
    receiveThread = (HANDLE)_beginthreadex(NULL, 0, &ReceiveThread, (void*)&clientSocket, 0, &threadID);
    if (receiveThread == NULL) {
        perror("failed to create receive thread");
        exit(EXIT_FAILURE);
    }

    // Communication loop
    while (1) {
        printf("Enter a message: ");
        fgets(message, BUFFER_SIZE, stdin);

        // Remove the newline character from the message
        message[strcspn(message, "\n")] = '\0';

        if ((message[0] == 'q' || message[0] == 'Q') && strlen(message) == 1) 
        {
            break;
        }


        // Send the message to the server
        if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        // Clear the buffer
        memset(message, 0, BUFFER_SIZE);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
