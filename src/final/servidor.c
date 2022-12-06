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

#define LISTENQ 5
#define MAXDATASIZE 100
#define MAXLINE 20000

int powInt(int x, int y)
{
    for (int i = 0; i < y; i++)
    {
        x *= 10;
    }
    return x;
}

int parseInt(char *chars)
{
    int sum = 0;
    int len = strlen(chars);
    for (int x = 0; x < len; x++)
    {
        int n = chars[len - (x + 1)] - '0';
        sum = sum + powInt(n, x);
    }
    return sum;
}

int main(int argc, char **argv)
{
    pid_t pid;
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    socklen_t servaddr_len;
    char buf[MAXDATASIZE];
    char message[MAXLINE];
    char client[MAXLINE];
    char error[MAXLINE + 1];
    int clients[MAXDATASIZE];
    int client_index = 0;

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

        clients[client_index] = ntohs(servaddr.sin_port);
        client_index++;

        if ((pid = fork()) == 0)
        {
            close(listenfd);

            printf("Received connection from %s:%d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

            strcat(buf, "Clients Available:\n");
            for (int i = 0; i < client_index - 1; i++)
            {
                snprintf(message, sizeof(message), "[%d] %s:%d\n", i, inet_ntoa(servaddr.sin_addr), clients[i]);
                strcat(buf, message);
            }
            if (client_index - 1 == 0)
            {
                strcat(buf, "No clients available\n");
            }
            write(connfd, buf, strlen(buf));
            read(connfd, client, MAXLINE);
            int selected = parseInt(client);
            if (selected >= 0)
            {
                snprintf(message, sizeof(message), "%d", clients[selected]);
                strcpy(client, message);

                write(connfd, client, strlen(client));
            }

            close(connfd);
            exit(0);
        }

        waitpid(pid, 0, 0);
        close(connfd);
    }

    return (0);
}
