#include <stdio.h>
#include <Winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <Ws2ipdef.h>  
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080


void error(const char* msg) {
	perror(msg);
	exit(1);
}

typedef int socklen_t;

int main()
{
	int iResult;
	int sockfd, newsockfd, portno, valread;
	const char opt = 1;
	struct sockaddr_in serv_addr, cli_addr;
	int addrlen = sizeof(serv_addr);
	char buffer[1024] = { 0 };
	const char* hello = "Hello From server";

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
		perror("Failed to initialize Winsock.");
		return 1;
	}


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket Failed");
		exit(EXIT_FAILURE);
	}

	socklen_t client;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
	{
		error("Binding Failed.");
	}

	listen(sockfd, 5);

	client = sizeof(cli_addr);

	newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,&client);

	if (newsockfd < 0) 
	{
		perror("couldnt accept socket\n");
		exit(EXIT_FAILURE);
	}

	iResult = send(newsockfd, buffer, 1024, 0);

	if (iResult < 0) 
	{
		perror("senrerror\n");
		closesocket(newsockfd);
		WSACleanup();
		return 1;
	}

	iResult = shutdown(newsockfd, SD_SEND);

	if (iResult == SOCKET_ERROR) 
	{
		perror("shutdown error\n");
		closesocket(newsockfd);
		WSACleanup();
		return 1;
	}

	do
	{
		iResult = recv(newsockfd, buffer, 1024, 0);
		if (iResult > 0) 
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

	} while (iResult > 0);
	
	closesocket(newsockfd);
	WSACleanup();


	return 0;

}
