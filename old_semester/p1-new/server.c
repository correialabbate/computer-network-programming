#include "server.h"

void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){

  int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char ip[INET6_ADDRSTRLEN];
  int rv;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
  
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
  }
  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }
    
  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  printf("server: waiting for connections...\n");
  while (1)
  { // main accept() loop
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      exit(1);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), ip, sizeof ip);
    printf("server: got connection from %s\n", ip);

    if (!fork()) { // this is the child process
      close(sockfd);           // child doesn't need the listener
      request_options(new_fd); // Communication function
      close(new_fd);
      exit(0);
    }
    close(new_fd); // parent doesn't need this
  }

  return 0;
}

/*#############################################################################*/

void request_options(int socket){
  char buffer[BUFFLEN];

  // notify that connection is set
  strcpy(buffer, "connection is set...\n");
  write_d(socket, buffer, strlen(buffer));

  // // notify that connection is set
  // strcpy(buffer, "Type help for instructions\n");
  // write_d(socket, buffer, strlen(buffer));

  while (1)
  {
    // Await new message from client
    printf("server awaiting new message...\n");

    read_d(socket, buffer);

    char client_option = strtok(buffer, " ")[0];

    // Test which request the client asked for
    switch (client_option){
      case '1':
        printf("Você escolheu a opção 1!\n");
        printf("adding a new movie...\n");
        add_movie(socket, buffer, strtok(NULL, "\n"));
        printf("new movie added\n");
        break;
      case '2':
        printf("Você escolheu a opção 2!\n");
        printf("removing the movie...\n");
        remove_movie(socket, buffer, strtok(NULL, "\n"));
        printf("movie removed\n");
        break;
      case '3':
        printf("Você escolheu a opção 3!\n");
        printf("retrieving titles and movie rooms...\n");
        get_movie_titles_and_rooms(socket, buffer);
        printf("all movie titles and rooms retrieved!\n");
        break;
      case '4':
        printf("Você escolheu a opção 4!\n");
        printf("retrieving movies by genre...\n");
        movies_by_genre(socket, buffer, strtok(NULL, "\n"));
        printf("movies by genre retrieved\n");
        break;
      case '5':
        printf("Você escolheu a opção 5!\n");
        printf("retrieving movie title...\n");
        get_movie_title(socket, buffer, strtok(NULL, "\n"));
        printf("movie title retrieved\n");
        break;
      case '6': // Get full movie info
        printf("Você escolheu a opção 6!\n");
        printf("retrieving movie...\n");
        get_movie(socket, buffer, strtok(NULL, "\n"));
        printf("movie sent!\n");
        break;
      case '7':
        printf("Você escolheu a opção 7!\n");
        printf("retrieving all movies...\n");
        get_all_movies(socket, buffer);
        printf("all movies retrieved!\n");
        break;
      case '0':
        printf("Você escolheu a opção 0!\n");
        printf("sending help info...\n");
        send_help(socket, buffer);
        break;
      default:
        printf("Opção inválida!\n");
    }

    // End connection if requested by client
    if (!strcmp(buffer, "exit"))
      break;
  }

  return;
}

/*## Movie Functions ########################################################*/

void add_movie(int socket, char *buffer, char *movie_info){
  /* File pointer to hold reference to our file */
  printf("Movie info: %s\n", movie_info);
  FILE *movie, *index;
  int count = 0;
  char movie_info_copy[BUFFLEN], movie_name[BUFFLEN], movie_genre[BUFFLEN], movie_director[BUFFLEN], movie_release_year[BUFFLEN];
  strcpy(movie_info_copy, movie_info);

  int init_size = strlen(movie_info_copy);
  char delim[] = ",";

  char *ptr = strtok(movie_info_copy, delim);

  while (ptr != NULL){
    if (count == 0){
      strcpy(movie_name, ptr);
      printf("%s\n", movie_name);
    } else if (count == 1){
      strcpy(movie_genre, ptr);
      printf("%s\n", movie_genre);
    } else if (count == 2){
      strcpy(movie_director, ptr);
      printf("%s\n", movie_director);
    } else if (count == 3) {
      strcpy(movie_release_year, ptr);
      printf("%s\n", movie_release_year);
    }

    ptr = strtok(NULL, delim);

    count++;
  }

  // First we need to update the index.txt file

  //open file in append mode
  index = fopen("data/index.txt", "a");

  if (index == NULL)
  {
    /* File not created hence exit */
    printf("Unable to open the file.\n");
    exit(1);
  }
  else
  {
    fputs(movie_name, index);
    fputc('\n', index);
  }

  fclose(index);

  // Now we need to create the new file
  char fileName[1000] = "data/";
  char fileType[5] = ".txt";

  strcat(fileName, movie_name); // concatenate folder name with typed movie name
  strcat(fileName, fileType);   // concatenate folder + movie_name with file type to generate the complete filename

  movie = fopen(fileName, "w");

  /* fopen() return NULL if last operation was unsuccessful */
  if (movie == NULL)
  {
    /* File not created hence exit */
    printf("Unable to create file.\n");
    exit(1);
  }
  else
  {
    char c = '\n';

    /* Write data to file */
    fputs(movie_name, movie);
    fputc(c, movie);

    fputs(movie_genre, movie);
    fputc(c, movie);

    fputs(movie_director, movie);
    fputc(c, movie);

    fputs(movie_release_year, movie);
    fputc(c, movie);
  }

  /* Close file to save file data */
  fclose(movie);

  // Send positive response with the movie identifier to client
  char identifier[BUFFLEN] = "";
  strcat(identifier, movie_name);
  strcpy(buffer, identifier);
  write_d(socket, buffer, strlen(buffer));

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

void remove_movie(int socket, char *buffer, char *movie_name_copy)
{
  // First we need to delete the line that contains the removed movie from index.txt
  FILE *index, *replacer;
  char filename[20] = "data/index.txt";
  char ch, movie_name[BUFFLEN], movie_to_compare[BUFFLEN];
  int delete_line = 0, temp = 1;

  strcpy(movie_to_compare, movie_name_copy);

  //open file in read mode
  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';

    if (strcmp(movie_to_compare, movie_name) == 0)
    {
      delete_line = temp;
      break;
    }

    temp++;
  }

  ch = getc(index);
  while (ch != EOF)
  {
    ch = getc(index);
  }
  //rewind
  rewind(index);

  //open new file in write mode
  replacer = fopen("data/replica.txt", "w");
  temp = 1;
  int c = fgetc(index);

  do
  {
    if (temp != delete_line)
    {
      //copy all lines to file replica.txt
      fputc((char)c, replacer);
    }

    if ((char)c == '\n')
    {
      temp++;
    }

    // Checking for end of file
    if (feof(index))
      break;

  } while ((c = fgetc(index)) != EOF);

  fclose(index);
  fclose(replacer);
  remove(filename);

  //rename the file replica.txt to original name
  rename("data/replica.txt", filename);

  // Now we need to delete the txt file for the chosen movie
  char fileName[1000] = "data/";
  char fileType[5] = ".txt";

  strcat(fileName, movie_to_compare); // concatenate folder name with typed movie name
  strcat(fileName, fileType);         // concatenate folder + movie_name with file type to generate the complete filename

  if (remove(fileName) == 0)
    printf("Deleted successfully\n");
  else
  {
    printf("Unable to delete the file\n");
    exit(1);
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof
  return;
}

void get_movie_titles_and_rooms(int socket, char *buffer)
{
  FILE *index, *movie;
  char movie_name[BUFFLEN];

  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';
    movie = fopen(get_path(buffer, movie_name, 't'), "r");

    get_line(movie, buffer, 1);
    strcat(buffer, "\nSalas de exibição: ");
    get_line(movie, &buffer[strlen(buffer)], 4);
    strcat(buffer, "\n");
    write_d(socket, buffer, strlen(buffer));

    fclose(movie);
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  fclose(index);
  return;
}

void movies_by_genre(int socket, char *buffer, char *genre_copy)
{
  FILE *index, *movie;
  char genre[BUFFLEN], movie_name[BUFFLEN];
  int helper = 0; // flag to check if a movie from that genre exists

  strcpy(genre, genre_copy);
  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';
    movie = fopen(get_path(buffer, movie_name, 't'), "r");
    get_line(movie, buffer, 3);
    printf("%s is of the genre |%s|%s|\n", movie_name, buffer, genre);

    if (!strcmp(buffer, genre))
    {
      get_line(movie, buffer, 1);
      strcat(buffer, "\nGênero: ");
      get_line(movie, &buffer[strlen(buffer)], 3);
      strcat(buffer, "\n");
      write_d(socket, buffer, strlen(buffer));
      helper = 1;
    }

    fclose(movie);
  }

  if (helper == 0)
  {
    char no_movie[60] = "There is no movies from that genre inside the system :(\n";
    write_d(socket, no_movie, strlen(no_movie)); // send a message that no movie with that genre exists
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  fclose(index);
  return;
}

void get_movie_title(int socket, char *buffer, char *movie_name)
{
  FILE *movie;
  char path[BUFFLEN];
  char helper[14] = " <- Título\n";

  movie = fopen(get_path(path, movie_name, 't'), "r");

  get_line(movie, buffer, 1); // Get movie title
  write_d(socket, strcat(buffer, helper), strlen(buffer) + strlen(helper));

  write_d(socket, buffer, 0); // Send empty buffer to sinal eof

  fclose(movie);
  return;
}

void get_all_movies(int socket, char *buffer)
{
  FILE *index;

  get_path(buffer, "index", 't');
  index = fopen(buffer, "r");

  while (fgets(buffer, BUFFLEN, index))
  {
    buffer[strlen(buffer) - 1] = '\0';
    printf("sending movie: %s\n", buffer);
    write_d(socket, buffer, strlen(buffer)); // send movie name
    get_movie(socket, buffer, buffer);       // send movie
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

void get_movie(int socket, char *buffer, char *buff_movie_name)
{
  FILE *fptr;
  int line = 0;
  char movie_name[BUFFLEN], tag[BUFFLEN];
  char *tags[] = {"Nome: \0", "Sinopse: \0", "Gênero: \0", "Salas de exibição: \0", "                                 \0"};

  strcpy(movie_name, buff_movie_name); // Copy movie name key from buffer

  // Gets the values in the txt file
  get_path(buffer, movie_name, 't');

  if ((fptr = fopen(buffer, "r")) == NULL)
  {
    printf("Error! opening file: %s\n", buffer);
    exit(1); // Exits if failed to open file
  }

  // Send contents from file
  while (fgets(buffer, BUFFLEN, fptr))
  {
    strcpy(tag, tags[line]);
    strcat(tag, buffer);
    write_d(socket, tag, strlen(tag));
    if (line < 6)
      ++line;
  }

  write_d(socket, buffer, 0); // Send empty buffer to sinal eof

  return;
}

void send_help(int socket, char *buffer)
{
  FILE *help;

  get_path(buffer, "help", 't');
  help = fopen(buffer, "r");

  while (fgets(buffer, BUFFLEN, help))
  {
    buffer[strlen(buffer) - 1] = '\0';
    printf("sending: %s\n", buffer);
    write_d(socket, buffer, strlen(buffer));
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

/*## Transfer file functions ##################################################*/

// Function to split all the files that are inside "data" folder and send
// them to the client
void send_file(int socket, char *buffer, char *full_path)
{
  FILE *input;          // File that is going to be sent
  long int i = 0, size; // size of it

  input = fopen(full_path, "rb");
  printf("sending file \"%s\"\n", get_name(full_path));

  // Get file size
  fseek(input, 0, SEEK_END);
  size = ftell(input);
  fseek(input, 0, SEEK_SET);

  sprintf(buffer, "%ld", size);            // Convert size to string
  write_d(socket, buffer, strlen(buffer)); // Send to client

  while (i < size)
  { // reads char by char filling buffer until eof
    buffer[(i++) % BUFFLEN] = fgetc(input);
    if (i % BUFFLEN == 0 || i == size)
      write_d(socket, buffer, BUFFLEN); // sends entire buffer to avoid border issues
  }

  printf("file sent\n");
  fclose(input);
  return;
}

//## Functions to handle paths ##################################################

// Function to get the path of the file that will be sent
char *get_path(char *path, char *file_name_buff, char id)
{
  char szTmp[32], file_name[BUFFLEN];
  int bytes;

  strcpy(file_name, file_name_buff);
  sprintf(szTmp, "/proc/%d/exe", getpid()); // get this process origin file path
  bytes = readlink(szTmp, path, BUFFLEN);   // save path

  for (bytes; path[bytes] != '/'; --bytes)
    ;                     // removes the process name
  path[bytes + 1] = '\0'; // end of file

  if (id == 't')
    strcat(strcat(strcat(path, "data/"), file_name), ".txt");

  return path; // return the path and its size
}

char *get_name(char *path)
{
  int i;

  for (i = strlen(path); i >= 0; --i)
    if (path[i] == '/')
      return &(path[i + 1]);

  return path;
}

/*## Functions to search something specific inside a file #######################*/

// Retrieves a specific entry from the movie
char *get_line(FILE *movie, char *buffer, int line)
{
  int i, position = ftell(movie);

  fseek(movie, 0, SEEK_SET);
  for (i = 1; i < line; ++i)
    fgets(buffer, BUFFLEN, movie);
  buffer = fgets(buffer, BUFFLEN, movie);
  fseek(movie, position, SEEK_SET);

  if (buffer && buffer[strlen(buffer) - 1] == '\n')
    buffer[strlen(buffer) - 1] = '\0';

  return buffer;
}
