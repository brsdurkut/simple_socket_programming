#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>


static int *threads;

const char MSG[] = "This is the whole data";

void sigchld_handler(int signo) {
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]){
  threads = mmap(NULL, sizeof *threads, PROT_READ | PROT_WRITE, 
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *threads = 0;
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
  struct sockaddr_in sAddr;
  int listensock;
  int newsock;
  char buffer[25];
  int result;
  int nread;
  int pid;
  int val;
  int sport;

  if (2 != argc) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  sport = atoi(argv[1]);

  listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  val = 1;
  result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  if (result < 0) {
    perror("server2");
    return 0;
  }

  sAddr.sin_family = AF_INET;
  sAddr.sin_port = htons(sport);
  sAddr.sin_addr.s_addr = INADDR_ANY;

  result = bind(listensock, (struct sockaddr*) &sAddr, sizeof(sAddr));
  if (result < 0) {
    perror("server2");
    return 0;
  }

  result = listen(listensock, 5);
  if (result < 0) {
    perror("server2");
    return 0;
  }

  signal(SIGCHLD, sigchld_handler);

  while (1) {
    newsock = accept(listensock, NULL, NULL);
    if ((pid = fork()) == 0) {
      printf("child process %i created\n", getpid());
      close(listensock);

      nread = recv(newsock, buffer, 25, 0);
      if ( *threads == 0) {
        *threads = atoi(buffer);
        int size_perpiece = ( strlen(MSG) / *threads );

        strcpy(buffer, "OK!");
      } else {
        int id_client = atoi(buffer);
        int size_perpiece = ( strlen(MSG) / *threads );
        memset(buffer, 0, sizeof buffer);
        strncpy(buffer, MSG+(size_perpiece * id_client), size_perpiece);
        printf("%d.piece( \"%s\" ) is sending to client #%d\n",
              id_client,
              buffer,
              id_client);
      }
      send(newsock, buffer, sizeof(buffer), 0);
      close(newsock);
      printf("child process %i finished.\n", getpid());
      munmap(threads, sizeof *threads);
      exit(0);
    }
    close(newsock);
  }
}
