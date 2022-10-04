#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 20000

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t pid;
    struct sockaddr_in servaddr;
    socklen_t servaddr_len;
    char buf[MAXDATASIZE];
    char error[MAXLINE + 1];
    char message[MAXDATASIZE];
    char commands[4][MAXDATASIZE] = {"ls -l", "ifconfig", "pwd", "EXIT"};
    time_t ticks;

    if (argc != 2)
    {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <Port>");
        perror(error);
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1)
    {
        perror("listen");
        exit(1);
    }

    servaddr_len = sizeof(servaddr);
    if (getsockname(listenfd, (struct sockaddr *)&servaddr, &servaddr_len) < 0)
    {
        perror("getsockname error");
        exit(1);
    }
    printf("Running in %s:%d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

    for (;;)
    {
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            perror("accept");
            exit(1);
        }

        if (getpeername(connfd, (struct sockaddr *)&servaddr, &servaddr_len) < 0)
        {
            perror("getpeername error");
            exit(1);
        }

        ticks = time(NULL);

        printf("Received connection from %s:%d at %.24s\r\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), ctime(&ticks));

        if ((pid = fork()) == 0)
        {
            close(listenfd);

            snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
            write(connfd, buf, strlen(buf));

            write(connfd, commands[0], sizeof(commands[0]));

            if (read(connfd, message, sizeof(message)) < 0)
            {
                perror("recv error");
                exit(1);
            }
            printf("Message from client: \n\n%s\n\n", message);

            ticks = time(NULL);
            printf("Closed connection from %s:%d at %.24s\r\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), ctime(&ticks));
            close(connfd);
            exit(0);
        }

        close(connfd);
    }

    return (0);
}
