#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 8192 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void print_menu(){
    printf("\n############################################################################\n");
    printf("Bem-vindo ao servidor de streaming de vídeos!\n");
    printf("O que deseja fazer?\n");
    printf("----------------------------------------------------------------------------\n");
    printf("1. Cadastrar um novo filme.\n");
    printf("2. Acrescentar um novo gênero a um filme.\n");
    printf("3. Listar todos os filmes.\n");
    printf("4. Listar informações de todos os filmes de um determinado gênero.\n");
    printf("5. Listar todas as informações de todos os filmes.\n");
    printf("6. Listar todas as informações de um filme a partir de seu identificador.\n");
    printf("7. Remover um filme a partir de seu identificador.\n");
    printf("0. Sair.\n");
    printf("----------------------------------------------------------------------------\n");
    printf("Digite o número correspondente à sua opção: ");

}

int write_d(int socket, char *buffer, int length)
{
  int i, r_val;

  // Fiil message to standard size of buffer
  for (i = length; i < MAXDATASIZE; ++i)
    buffer[i] = '\0';

  if ((r_val = send(socket, buffer, MAXDATASIZE, 0)) == -1)
  {
    perror("ERROR: send");
    exit(1);
  }
  else if (r_val == 0)
  {
    printf("ERROR: pairing socket is closed\n");
    exit(1);
  }

  return r_val;
}

// Debug wrapper for recv
int read_d(int socket, char *buffer)
{
  int r_val, total = 0;

  while (total != MAXDATASIZE)
  {
    if ((r_val = recv(socket, &buffer[total], (MAXDATASIZE - total), 0)) == -1)
    {
      perror("ERROR: send");
      exit(1);
    }
    else if (r_val == 0)
    { // if client not responding
      printf("ERROR: pairing socket is closed\n");
      exit(1);
    }
    else
    {
      total += r_val;
    }
  }

  return total;
}

int send_data(int sockfd, char* data){
    int return_value;
    return_value = write_d(sockfd, data, strlen(data));
    return return_value;
}

int create_client_request(int menu_option, int sockfd){
    switch(menu_option){
        case 1:
            printf("Digite as informações do filme a ser adicionado com o seguinte formato:\n");
            printf("    Título do Filme,Gênero1Gênero2[...],Diretor(a),Ano de Lançamento\n");
            printf("Exemplo: O Resgate do Soldado Ryan,DramaGuerra,Steven Spielburg,1999\n");
            printf("Informações do filme a ser adicionado: ");
            char movie_info[100];  // max lenght for movie info = 100 
            fgets(movie_info, sizeof(movie_info)+1, stdin);
            // sends the operation code for the server
            send(sockfd, "1", 1, 0);
            // send_data - sends the total bits that will be sent based on the size of the string
            send_data(sockfd, movie_info); // send movie data to the server
            break;
        case 2:
            printf("Digite as informações do filme a ser atualizado com novo gênero com o seguinte formato:\n");
            printf("    Identificador do Filme,Gênero a ser adicionado\n");
            printf("Exemplo: 1,Drama\n");
            printf("Informações do filme a ser atualizado com novo gênero: ");
            char movie_info_new_genre[30]; // max lenght for movie id + new genre = 30
            fgets(movie_info_new_genre, sizeof(movie_info_new_genre)+1, stdin);
            // sends the operation code for the server
            send(sockfd, "2", 1, 0);
            // send_data
            send_data(sockfd, movie_info_new_genre);
            break;
        case 3:
            break;
        case 4:
            printf("Gênero do qual se deseja obter as informações: ");
            char movie_genre[10];
            fgets(movie_genre, sizeof(movie_genre)+1, stdin);
            printf("%s", movie_genre);
        case 5:
            break;
        case 6:
            printf("Identificador do filme do qual se deseja obter as informações: ");
            char movie_id_info[5];
            fgets(movie_id_info, sizeof(movie_id_info)+1, stdin);
            break;
        case 7:
            printf("Identificador do filme do qual se deseja remover: ");
            char movie_id_delete[5];
            fgets(movie_id_delete, sizeof(movie_id_delete)+1, stdin);
            break;
    }
}

void connect_server(int sockfd){
    char menu_option[2]; // I only need 1 space to store the menu option
    print_menu();
    fgets(menu_option, sizeof(menu_option)+1, stdin); // It gets the user input
    strtok(menu_option, "\n");
    printf("----------------------------------------------------------------------------\n");
    printf("Você escolheu a opção %s!\n", menu_option);
    int menu_option_int = atoi(menu_option); // atoi(*str) converts char to int: '7' -> 7
    create_client_request(menu_option_int, sockfd);
}

int main(int argc, char *argv[]){

    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    connect_server(sockfd);
    
    close(sockfd);

    return 0;
}