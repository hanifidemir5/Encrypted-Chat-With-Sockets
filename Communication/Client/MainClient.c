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
int deneme = 0;

// Function to handle receiving data from the server
unsigned __stdcall ReceiveThread(void* arg) {
    int i,j;
    SOCKET clientSocket = *(SOCKET*)arg;
    unsigned char buffer[BUFFER_SIZE] = { '\0' };
    unsigned char* decryptedText = malloc(BUFFER_SIZE);
    unsigned char counter[AES_BLOCK_SIZE + 1] = { '\0' };
    unsigned char accualMessage[BUFFER_SIZE] = { '\0' };
    unsigned char temp[BUFFER_SIZE] = { '\0' };
    unsigned char username[BUFFER_SIZE] = { 0 };
    unsigned char usernameLengthChar;
    unsigned int usernameLengthInt;

    while (1) {

        j = 0;
        // Receive response from the server
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            perror("receive failed");
            exit(EXIT_FAILURE);
        }
        // Take buffer's first char which represents username length as char
        usernameLengthChar = buffer[0];
        // Turn it int value
        usernameLengthInt = (int)(usernameLengthChar);
        // Make buffer NULL terminated in order to copy correctly
        buffer[strlen(buffer)] = '\0';
        // Use strcpy function which copies the string untill a null char
        strcpy(temp, buffer);
        // Use memcpy function which you can indicate where to start and stop copying process
        memcpy(accualMessage, buffer + (1 + usernameLengthInt), (strlen(buffer) - (17 + usernameLengthInt)));
        // Make accualMessage NULL terminated because memcpy function doesn't do it
        memcpy(username, buffer + 1,usernameLengthInt);
        // Again NULL terminate
        username[usernameLengthInt] = '\0';
        // Take counter value from end of the buffer
        for (i = 0; i < 16; i++)
        {
            counter[15 - i] = temp[(strlen(temp) - i - 1)];
        }
        // Make it NULL terminated
        counter[16] = '\0';
        // Make output array to use it correctly
        unsigned char decryptedText[BUFFER_SIZE] = {'\0'};
        // Check if decryptedText is NULL
        printf("\nmessage : %s message length: %d counter %s counter length %d key %s keylength %d\n",accualMessage,strlen(accualMessage),counter, strlen(counter),key, strlen(key));
        if (decryptedText[0] == '\0')
        {
            // Decrypt the message 
            decryptMessage(accualMessage, decryptedText, key, counter);
            // Print decrypted message
            printf("%s:%s\n", username, decryptedText);
        }
        else {
            // If output array is not null give error
            printf("Error, output array is not empty");
        }

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
    unsigned char buffer[BUFFER_SIZE] = { 0 };




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

    // Receive key
    if (recv(clientSocket, buffer, BUFFER_SIZE - 1, 0) == SOCKET_ERROR) {
        perror("receive failed");
        exit(EXIT_FAILURE);
    }

    // Store the received key for further processing
    strncpy(key, buffer, BUFFER_SIZE);

    memset(buffer, 0, BUFFER_SIZE);

    // Receive user list
    if (recv(clientSocket, buffer, BUFFER_SIZE - 1, 0) == SOCKET_ERROR) {
        perror("receive failed");
        exit(EXIT_FAILURE);
    }

    // Store the received key for further processing
    printf(" User list is :\n %s", buffer);

    memset(buffer, 0, BUFFER_SIZE);

    // Create a thread for receiving data from the server
    receiveThread = (HANDLE)_beginthreadex(NULL, 0, &ReceiveThread, (void*)&clientSocket, 0, &threadID);
    if (receiveThread == NULL) {
        perror("failed to create receive thread");
        exit(EXIT_FAILURE);
    }

    printf("Welcome to the chat room!!\n ");


    // Communication loop
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);

        // Remove the newline character from the message
        message[strcspn(message, "\n")] = '\0';

        if ((message[0] == 'q' || message[0] == 'Q') && strlen(message) == 1)
        {
            break;
        }

        // if its a private message get the user
        if (message[0] == '@') 
        {
            // Get receipients nick with '@'
            char* recipient = strtok(message, " ");  
            // Get the rest of the message
            char* messageText = strtok(NULL, "");

            if (recipient != NULL) 
            {
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
                // Copy recipient's nick into message array with @ of course
                strcpy(message, recipientNick);
                // Add space caracter to indicate target user
                strcat(message, " ");
                // Add cipherText to the end of message array
                if (cipherText != NULL) 
                {
                    strcat(message, cipherText);
                }
            }
            // Add Counter 
            strcat(message, counter);
        }
        else 
        {
            //Encrypt the message
            encryptMessage(message, cipherText, key, counter);
            // Add cipherText to message
            strcpy(message, cipherText); 
            // Add Counter 
            strcat(message, counter);
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


/*int main()
[{]
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
