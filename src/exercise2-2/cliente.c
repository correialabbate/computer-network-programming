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
#include <sys/stat.h>

#define MAXLINE 4096

int main(int argc, char **argv)
{
    int sockfd, n, link[2], offset, remain_data, sent_bytes=0, file_size;
    pid_t pid;
    char recvline[MAXLINE + 1];
    char error[MAXLINE + 1];
    char message[MAXLINE + 1];
    char output[MAXLINE + 1];
    // char file_size[256];
    ssize_t len;
    struct sockaddr_in servaddr;
    struct stat file_stat;
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

    int times = 0;

    // while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
    //     recvline[n] = 0;
    //     // if (fputs(recvline, stdout) == EOF) {
    //     //     perror("fputs error");
    //     //     exit(1);
    //     // }

    //     times++;
    //     printf("n: %d\n", n);
    //     printf("recvline string (%d): %s. END\n", times, recvline);
    // }

    // read first command
    n = read(sockfd, recvline, MAXLINE);

    while (strncmp(recvline, "EXIT", 4) != 0){
        printf("Client received (%d): %s\n", n, recvline);
        snprintf(message, sizeof(message), "Client received (%d): %s\n", n, recvline);

        FILE *fp;
        char path[1035];
        char file_content[10000] = "";

        fp = popen(recvline, "r");
        if (fp == NULL) {
            printf("Failed to run command\n" );
            exit(1);
        }

        file_size = 0;
        while (fgets(path, sizeof(path), fp) != NULL) {
            printf("%s", path);
            printf("file_content: %s\n", file_content);

            strcpy(file_content, path);
        }

        printf("file_content: %s\n", file_content);
        
        // fstat(fp, &file_stat);
        // sprintf(file_size, "%d", file_stat.st_size);
        printf("file_size: %d", file_size);

        len = send(sockfd, file_size, sizeof(file_size), 0);

        offset = 0;
        remain_data = file_stat.st_size;
        /* Sending file data */
        while (((sent_bytes = sendfile(sockfd, fp, &offset, BUFSIZ)) > 0) && (remain_data > 0))
        {
                fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
                remain_data -= sent_bytes;
                fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
        }

        /* close */
        pclose(fp);

        write(sockfd, message, strlen(message));

        n = read(sockfd, recvline, MAXLINE);
    }

    if (n < 0) {
        close(sockfd);
    }

    exit(0);
}
