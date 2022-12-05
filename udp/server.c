// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
	
#define PORT	 8080
#define MAXLINE 1024
	
// Driver code
int main() {
	int sockfd;
	char buffer[MAXLINE];
    char message[MAXLINE];
	char *hello = "Hello from server";
	struct sockaddr_in servaddr, cliaddr;
		
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);
		
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	int len, n;
	
	len = sizeof(cliaddr); //len is value/result

	printf("Waiting for message...");
    while(strcmp(buffer, "finalizar_chat") != 0){
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
	    			MSG_WAITALL, (struct sockaddr *) &servaddr,
	    			&len);
	    buffer[n] = '\0';
	    printf("Server: %s\n", buffer);

        printf("Type message to send: ");
        scanf("%s", &message);
        sendto(sockfd, (const char *)message, strlen(message),
	    	MSG_CONFIRM, (const struct sockaddr *) &servaddr,
	    		sizeof(servaddr));
    }

	return 0;
}
