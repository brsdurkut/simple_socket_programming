#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


char MESSAGE[42] = "Connection is established with number: ";
char rand_str[2];
int rand_number = 0;


int main(int argc, char *argv[]){
  srand(time(NULL));

  int sv_socket = 0;
  int sv_port = 0;
  int result = 0;

  /*
   *  Its purpose is to provide a standard way of handling endpoint addresses
   *  for network communications.
   *  short   sin_family; // type of address
   *  u_short sin_port;   // protocol port number
   *                      // network byte ordered
   *  u_long  sin_addr;   // net address for the remote host
   *                      // network byte ordered
   *  char    sin_zero[8];// unused, set to zero
   */
  struct sockaddr_in sockaddr;

  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /*
   *  int socket(int domain, int type, int protocol);
   *    int domain;   //  protocol or address family
   *    int type:     //  type of service
   *    int protocol: //  the protocol number; generally zero(0) is used
   */
  sv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sv_socket == -1) {
    fprintf(stderr, "Could not create a socket! \n");
    exit(1);
  } else {
    fprintf(stdout, "Socket created!\n");
  }

  /*  convert port number to integer from ascii */
  sv_port = atoi(argv[1]);

  /*
   *  setup the address structure
   *  use INADDR_ANY to bind to all local addresses
   *  do network byte ordered conversions
  */
  bzero(&sockaddr, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons(sv_port);

  /*
   *  the bind() function is used to bind the socket to a given address once the
   *  socket is created.
   *
   *  int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
   *    int sockfd;               //  our socket descriptor
   *    struct sockaddr *my_addr; //  our sockaddr structure
   *    socklen_t addrlen;        //  length of sockaddr
  */
  result = bind(sv_socket,
               (struct sockaddr *)&sockaddr,
               sizeof(sockaddr));

  if (result == 0) {
    fprintf(stdout, "Bind completed!\n");
  } else {
    fprintf(stderr, "Could not bind to address!\n");
    close(sv_socket);
    exit(1);
  }

  /*
   *  our socket is ready to accept connections
   *
   *  int listen(int s, int backlog);
   *    int s;        //  our socket
   *    int backlog;  //  connection queue
  */
  result = listen(sv_socket, 5);
  if (result == -1) {
    fprintf(stderr, "Cannot listen on socket!\n");
    close(sv_socket);
    exit(1);
  }

  while(1) {
    struct sockaddr_in cl_sockaddr = { 0 };
    int cl_socket = 0;
    int cl_sockaddr_len = sizeof(cl_sockaddr);

    /*
     *  accept() function is a blocking function that waits until a connection
     *  request is received from a client.
     *  the second and third parameters passed to accept() are locations for
     *  storing information about the client, not about the server.
     *
     *  int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
     *    int s;  //  our socket descriptor
     *    struct sockaddr *addr;  //  pointer to client sockaddr structure
     *    socklen_t *addrlen;     //  length of client sockaddr structure
    */
    cl_socket = accept(sv_socket,
                      (struct sockaddr *)&cl_sockaddr,
                      &cl_sockaddr_len);
    if (cl_socket == -1) {
      fprintf(stderr, "Cannot accept connections!\n");
      close(sv_socket);
      exit(1);
    }


    rand_number = rand() % 10;
    snprintf (rand_str, sizeof(rand_str), "%d", rand_number);
    strcat(MESSAGE, rand_str);
    strcat(MESSAGE, "\n"); 
    fprintf(stdout, "New connection is established! \n");
    fprintf(stdout,
            "Client from: %s - with number: %s\n",
            inet_ntoa(cl_sockaddr.sin_addr),
            rand_str);

    /*  send data to client using client socket descriptor */
    write(cl_socket, MESSAGE, strlen(MESSAGE));

    /*  close client socket when the job is finished with the client  */
    close(cl_socket);

  }

  close(sv_socket);

  return 0;
}
