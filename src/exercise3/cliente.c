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

#define MAXLINE 4096

char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int i = strlen(str) - 1, j = 0;

    char ch;
    while (i > j)
    {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

char* strupr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = toupper((unsigned char) *tmp);
    }

    return s;
}

void CheckInput(int *argc, char ***argv)
{
    char error[MAXLINE + 1];

    if (*argc != 3)
    {
        strcpy(error, "uso: ");
        strcat(error, *argv[0]);
        strcat(error, " <IPaddress> <Port>");
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
    int sockfd, n, file_size;
    char recvline[MAXLINE + 1];
    char aux[MAXLINE + 1];
    struct sockaddr_in servaddr;
    char message[MAXLINE + 1];

    CheckInput(&argc, &argv);
    sockfd = CreateSocket();

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    InetPton(AF_INET, argv[1], &servaddr.sin_addr);
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    GetSockName(sockfd, (struct sockaddr *)&servaddr);

    printf("Remote socket is %s:%s\n", argv[1], argv[2]);

    PrintLocalInfo(&servaddr.sin_addr, &servaddr.sin_port);

    // get hello
    // if (fputs(recvline, stdout) == EOF)
    // {
    //     perror("fputs error");
    //     exit(1);
    // }
    // bzero(recvline, sizeof(recvline));

    n = read(sockfd, recvline, MAXLINE);
    strcpy(aux, recvline);
    printf("%s\n", strupr(strrev(aux)));

    while (strncmp(recvline, "EXIT", 4) != 0)
    {
        FILE *fp;
        char path[1035];
        char file_content[10000] = "";

        fp = popen(recvline, "r");
        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        file_size = 0;
        while (fgets(path, sizeof(path), fp) != NULL)
        {
            strcat(file_content, path);
        }

        file_size = strlen(file_content) + 1;

        send(sockfd, file_content, file_size, 0);

        pclose(fp);

        write(sockfd, message, strlen(message));

        bzero(recvline, MAXLINE + 1);
        n = read(sockfd, recvline, MAXLINE);
        strcpy(aux, recvline);
        printf("%s\n", strupr(strrev(aux)));
    }

    if (n < 0)
    {
        close(sockfd);
    }

    exit(0);
}
