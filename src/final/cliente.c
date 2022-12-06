#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAXLINE 40960

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

void CheckInput(int *argc, char ***argv)
{
    char error[MAXLINE + 1];

    if (*argc != 3)
    {
        strcpy(error, "uso: ");
        strcat(error, *argv[0]);
        strcat(error, " <IPaddress> <Port1>");
        perror(error);
        exit(1);
    }
}

int CreateSocket()
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }
    return sockfd;
}

void InetPton(int af, char *src, void *dst)
{
    if (inet_pton(af, src, dst) <= 0)
    {
        perror("inet_pton error");
        exit(1);
    }
}

void GetSockName(int sockfd, struct sockaddr *servaddr)
{
    socklen_t servaddr_len = sizeof(servaddr);

    if (getsockname(sockfd, servaddr, &servaddr_len) < 0)
    {
        perror("getsockname error");
        exit(1);
    }
}

void Connect(int sockfd, struct sockaddr *servaddr, int servaddr_len)
{
    if (connect(sockfd, servaddr, servaddr_len) < 0)
    {
        perror("connect error");
        exit(1);
    }
}

void PrintLocalInfo(struct in_addr *ip, in_port_t *localport)
{
    printf("Local socket is %s:%d\n", inet_ntoa(*ip), ntohs(*localport));
}

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];
    char message[MAXLINE + 1] = {};
    struct sockaddr_in servaddr;

    CheckInput(&argc, &argv);
    sockfd = CreateSocket();

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    InetPton(AF_INET, argv[1], &servaddr.sin_addr);
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    GetSockName(sockfd, (struct sockaddr *)&servaddr);

    while ((n = read(sockfd, recvline, MAXLINE)) > 0)
    {
        if (fputs(recvline, stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        }
        bzero(recvline, sizeof(recvline));

        scanf("%s", message);
        send(sockfd, message, sizeof(message), 0);
        bzero(recvline, sizeof(recvline));
        read(sockfd, recvline, MAXLINE);
        printf("Connect with %s:%d\n", inet_ntoa(servaddr.sin_addr), parseInt(recvline));
    }

    if (n < 0)
    {
        shutdown(sockfd, SHUT_WR);
        close(sockfd);
    }

    exit(0);
}
