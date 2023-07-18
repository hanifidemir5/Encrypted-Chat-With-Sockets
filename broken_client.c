#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>
#include "aes.h"


#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

char recipientNick[BUFFER_SIZE];
unsigned char key[BUFFER_SIZE];
char accualMessage[BUFFER_SIZE];


// Function to handle receiving data from the server
unsigned __stdcall ReceiveThread(void* arg) {
    int i,j;
    SOCKET clientSocket = *(SOCKET*)arg;
    char buffer[BUFFER_SIZE] = { 0 };
    char decryptedText[BUFFER_SIZE] = { 0 };
    char counter[AES_BLOCK_SIZE] = { 0 };
    char accualMessage[BUFFER_SIZE] = { 0 };


    if (recv(clientSocket, buffer, BUFFER_SIZE - 1, 0) == SOCKET_ERROR) {
        perror("receive failed");
        exit(EXIT_FAILURE);
    }

    // Store the received key-value pair for further processing
    strncpy(key, buffer, BUFFER_SIZE);

    memset(buffer, 0, BUFFER_SIZE);

    while (1) {
        j = 0;
        // Receive response from the server
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            perror("receive failed");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < strlen(buffer); i++) 
        {
            if (buffer[i] == '{' && buffer[i + 1] == '[') 
            {
                accualMessage[i] == '\0';
                while (buffer[i] != NULL) 
                {
                    counter[j] = buffer[i];
                    j++;
                }
                counter[j] = '\0';
                break;
            }
            else 
            {
                accualMessage[i] = buffer[i];
            }
        }

        
        decryptMessage(accualMessage, decryptedText, key, counter);

        printf("\n%s\n", decryptedText);

        // Clear the buffer
        memset(buffer, 0, BUFFER_SIZE);
        memset(decryptedText, 0, BUFFER_SIZE);
        memset(counter, 0, AES_BLOCK_SIZE);
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
    unsigned char cipherText[BUFFER_SIZE];
    unsigned char counter[AES_BLOCK_SIZE + 1];



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

    char username[BUFFER_SIZE] = { 0 };
    printf("Enter your username: ");
    fgets(username, BUFFER_SIZE, stdin);

    // Remove the newline character from the message
    username[strcspn(username, "\n")] = '\0';

    if (send(clientSocket, username, strlen(username), 0) == SOCKET_ERROR) {
        perror("send failed");
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

        //Encrypt the message
        encryptMessage(message, cipherText, key, counter);

        // if its a private message get the user
        if (message[0] == '@') 
        {
            char* recipient = strtok(message, " ");  // Get receipients nick with '@'
            char* nextChar = recipient + strlen(recipient);
            if (recipient != NULL) 
            {
                // Get the rest of the message
                char* messageText = strtok(NULL, "");       

                strcpy(recipientNick, recipient);
                if (messageText != NULL)
                {
                    strcpy(accualMessage, messageText);
                    //Encrypt the message without recipient's nick
                    encryptMessage(accualMessage, cipherText, key, counter);
                }
                else 
                {
                    printf("Message is empty!!\n");
                    continue;
                }
                // Copy recipient's nick into message array
                strcpy(message, recipientNick);
                // Add space character to indicate ciphertext is starting
                strcat(message, " ");
                // Add cipherText to the end of message array
                if (cipherText != NULL) 
                {
                    strcat(message, cipherText);
                }
            }
        }
        else 
        {
            //Encrypt the message
            encryptMessage(message, cipherText, key, counter);
            strcpy(message, cipherText);      
        }

        // Put '{[' caracters to indicate message is over
        strcat(message, "{[");
        // Add Counter 
        strcat(message, counter);
        

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


/*int main()
{
    unsigned char plainText[BUFFER_SIZE];
    unsigned char cipherText[BUFFER_SIZE];
    unsigned char decreptedText[BUFFER_SIZE];
    unsigned char key[] = "0123456789012345";
    unsigned char counter[AES_BLOCK_SIZE + 1];
    unsigned char tmp[AES_BLOCK_SIZE];

    fgets(plainText, BUFFER_SIZE, stdin);

    // Remove the newline character from the message
    plainText[strcspn(plainText, "\n")] = '\0';

    encryptMessage(plainText, cipherText, key, counter);

    for (int j = 0; j < AES_BLOCK_SIZE; j++)
    {
        printf("%2.2x%c", counter[j], ((j + 1) % 16) ? ' ' : '\n');
    }

    printf("\n Counter length is : %d \n", strlen(counter));

    //printf("asda %d\n", strlen(cipherText));

    decryptMessage(cipherText, decreptedText, key, counter);

    printf("asda %s and length:%d\n", decreptedText, strlen(decreptedText));


}*/
