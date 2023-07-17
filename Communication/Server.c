#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

unsigned char key[BUFFER_SIZE];


// Structure to hold client information
typedef struct {
    SOCKET socket;
    char username[BUFFER_SIZE];
} Client;

// Global variables
Client clients[MAX_CLIENTS];
int clientCount = 0;

// Function to handle communication with a client
unsigned __stdcall ClientThread(void* arg) {
    SOCKET clientSocket = *(SOCKET*)arg;
    char buffer[BUFFER_SIZE] = { 0 };
    char response[BUFFER_SIZE] = { 0 };
    char message[BUFFER_SIZE + BUFFER_SIZE] = { 0 };

    // Find the client's username
    char* username = NULL;
    for (int i = 0; i < clientCount; i++) {
        if (clients[i].socket == clientSocket) {
            username = clients[i].username;
            break;
        }
    }

    snprintf(key, BUFFER_SIZE, "%s", key);
    if (send(clientSocket, key, strlen(key), 0) == SOCKET_ERROR) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    // Send the list of connected users to the client
    char userList[BUFFER_SIZE] = { 0 };
    for (int i = 0; i < clientCount; i++) {
        strncat(userList, clients[i].username, sizeof(userList) - strlen(userList) - 1);
        strncat(userList, "\n", sizeof(userList) - strlen(userList) - 1);
    }
    send(clientSocket, userList, strlen(userList), 0);


    while (1) {
        // Receive client message
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            perror("receive failed");
            break;
        }

        printf("%s's message: %s\n", username, buffer);

        // Process client message (e.g., perform some operation)

        // Check if it is a private message
        if (buffer[0] == '@') {
            char* recipient = strtok(buffer + 1, " ");  // Skip the '@' symbol
            char* messageText = strtok(NULL, "");       // Get the rest of the message

            // Construct the private message
            snprintf(message, sizeof(message), "%s (private): %s", username, messageText);

            // Find the recipient in the client list
            int recipientFound = 0;
            for (int i = 0; i < clientCount; i++) {
                if (strcmp(clients[i].username, recipient) == 0) {
                    // Send the private message to the recipient
                    send(clients[i].socket, message, strlen(message), 0);
                    recipientFound = 1;
                    break;
                }
            }

            // If recipient not found, send an error message to the sender
            if (!recipientFound) {
                snprintf(response, BUFFER_SIZE, "Error: User '%s' not found.", recipient);
                send(clientSocket, response, strlen(response), 0);
            }
        }
        else {
            // Construct the broadcast message
            snprintf(message, sizeof(message), "%s (broadcast): %s", username, buffer);

            // Send the broadcast message to all clients except the sender
            for (int i = 0; i < clientCount; i++) {
                if (clients[i].socket != clientSocket) {
                    send(clients[i].socket, message, strlen(message), 0);
                }
            }
        }

        // Clear the buffers
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);
    }

    // Remove the client from the client list
    for (int i = 0; i < clientCount; i++) {
        if (clients[i].socket == clientSocket) {
            for (int j = i; j < clientCount - 1; j++) {
                clients[j] = clients[j + 1];
            }
            clientCount--;
            break;
        }
    }

    closesocket(clientSocket);
    _endthreadex(0);
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    HANDLE clientThreads[MAX_CLIENTS];
    unsigned threadID;
    int i, n;

    while (1)
    {
        printf("Please enter your key(keys max lentgh must be 16): \n");
        fgets(key, BUFFER_SIZE, stdin);

        // Remove the newline character from the message
        key[strcspn(key, "\n")] = '\0';

        if (strlen(key) > 16)
        {
            printf("\nKey length must be less than 16!!\n");
            continue;
        }
        else if (strlen(key) == 0) {
            printf("\nYou must enter at least one character!!\n");
            continue;
        }
        else if (strlen(key) == 16) {
            break;
        }
        else {
            for (i = strlen(key); i < 16; i++)
            {
                key[i] = 0x00;
            }
            break;
        }

    }

    printf("key is %s\n", key);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept a new connection
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
            perror("accept failed");
            break;
        }

        // Receive the client's username
        char username[BUFFER_SIZE] = { 0 };
        if (recv(newSocket, username, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            perror("receive failed");
            break;
        }

        // Add the client to the client list
        clients[clientCount].socket = newSocket;
        strncpy(clients[clientCount].username, username, sizeof(clients[clientCount].username) - 1);
        clientCount++;

        // Create a thread for the new client
        clientThreads[clientCount - 1] = (HANDLE)_beginthreadex(NULL, 0, &ClientThread, (void*)&newSocket, 0, &threadID);
        if (clientThreads[clientCount - 1] == NULL) {
            perror("failed to create client thread");
            break;
        }

        printf("New client connected: %s\n", username);
    }

    // Wait for all client threads to exit
    WaitForMultipleObjects(clientCount, clientThreads, TRUE, INFINITE);

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
