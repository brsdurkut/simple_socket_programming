#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


const char MESSAGE[] = "Connection is established!!\n";

int main(int argc, char *argv[]){
  int sv_socket = 0;
  int sv_port = 0;
  int result = 0;
  char buffer[256] = "";
  struct sockaddr_in sockaddr;

  if (3 != argc) {
    fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
    exit(1);
  }

  sv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sv_socket == -1) {
    fprintf(stderr, "Could not create a socket! \n");
    exit(1);
  } else {
    fprintf(stdout, "Socket created!\n");
  }

  sv_port = atoi(argv[2]);

  bzero(&sockaddr, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  inet_addr(argv[2], &sockaddr.sin_addr.s_addr);
  sockaddr.sin_port = htons(sv_port);

  /*  connect to the address and port with our socket  */
  result = connect(sv_socket,
               (struct sockaddr *)&sockaddr,
               sizeof(sockaddr));

  if (result == 0) {
    fprintf(stdout, "Connection is established!\n");
  } else {
    fprintf(stderr, "Could not connect to address!\n");
    close(sv_socket);
    exit(1);
  }

  /*  get the message from the server  */
  result = read(sv_socket, buffer, sizeof(buffer));

  if (result > 0) {
    fprintf(stdout, "%d: %s\n", result, buffer);
  } else {
    fprintf(stderr, "Return Status = %d \n", result);
  }

  close(sv_socket);

  return 0;
}
