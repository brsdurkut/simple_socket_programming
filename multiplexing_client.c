#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int parent_func(int children, int port, char *server);
void child_func(int childnum, int port, char *server);

char **recv_pieces;

int main(int argc, char *argv[]){
  int nchildren = 1;
  int pid;
  int x;
  int port = 0;


  if (argc < 3) {
    fprintf(stderr, "Usage: %s <server> <port> [#ofchild]\n", argv[0]);
    exit(1);
  } else if (argc > 3) {
    nchildren = atoi(argv[3]);
  }

  port = atoi(argv[2]); 

  if ( parent_func(nchildren, port, argv[1]) != 0 ) {
    perror("parent_func");
    close(1);
  }

  recv_pieces = (char**)malloc(nchildren*sizeof(char*));

  int i = 0;
  for (i; i < nchildren; i++) {
    recv_pieces[i] = (char*)malloc( 25 * sizeof(char));
  }

  for (x = 0; x < nchildren; x++) {
      if ((pid = fork()) == 0) {
        child_func(x, port, argv[1]);
        exit(0);
      }
  }

  wait(NULL);

  printf("\nData is received: \"");
  for (i = 0; i < nchildren; i++) {
    FILE *fp;
    char filename[32];
    char buff[25];
    snprintf(filename, sizeof(char) * 32, "child%i", i);
    fp = fopen(filename, "r");
    fgets(buff, 255, (FILE*)fp);
    printf("%s", buff);
    fclose(fp);
  }
  printf("\"\n");
  return 0;
}

int parent_func(int children, int port, char *server) {
  int sock;
  struct sockaddr_in sAddr;
  char buffer[25];

  memset((void*) &sAddr, 0, sizeof(struct sockaddr_in));
  sAddr.sin_family = AF_INET;
  sAddr.sin_addr.s_addr = INADDR_ANY;
  sAddr.sin_port = 0;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

  sAddr.sin_addr.s_addr = inet_addr(server);
  sAddr.sin_port = htons(port);

  if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0) {
      perror("parent_client");
      return 1;
  }

  snprintf(buffer, 128, "%i", children);
  printf("Sending \"%s\" command to server..\n", buffer);
  send(sock, buffer, strlen(buffer), 0);

  sleep(1);
  recv(sock, buffer, 25, 0);
  printf("\"%s\" from server\n", buffer);
  sleep(1);
  close(sock);
  return 0;
}

void child_func(int childnum, int port, char *server) {
  int sock;
  struct sockaddr_in sAddr;
  char buffer[25];

  memset((void*) &sAddr, 0, sizeof(struct sockaddr_in));
  sAddr.sin_family = AF_INET;
  sAddr.sin_addr.s_addr = INADDR_ANY;
  sAddr.sin_port = 0;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

  sAddr.sin_addr.s_addr = inet_addr(server);
  sAddr.sin_port = htons(port);

  if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0) {
      perror("client");
      return;
  }

  snprintf(buffer, 128, "%i.", childnum);
  printf("child #%i sent\n",
        childnum);

  send(sock, buffer, strlen(buffer), 0);

  sleep(1);
  recv(sock, buffer, 25, 0);
  printf("child #%i received \"%s\"\n",
        childnum,
        buffer);

  FILE *fp;
  char filename[32];
  snprintf(filename, sizeof(char) * 32, "child%i", childnum);
  fp = fopen(filename, "w+");
  fprintf(fp, buffer);
  fclose(fp);

  sleep(1);
  close(sock);
}
