#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>


const char MSG[] = "This is the whole data";
int threads = 0; 

int main(int argc, char *argv[]){
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
  fd_set readset, testset;
  int listensock;
  int newsock;
  char buffer[25];
  int result;
  int nread;
  int x;
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
    perror("server1");
    return 0;
  }

  sAddr.sin_family = AF_INET;
  sAddr.sin_port = htons(sport);
  sAddr.sin_addr.s_addr = INADDR_ANY;

  result = bind(listensock, (struct sockaddr*) &sAddr, sizeof(sAddr));
  if (result < 0) {
    perror("server1");
    return 0;
  }

  result = listen(listensock, 5);
  if (result < 0) {
    perror("server1");
    return 0;
  }

  FD_ZERO(&readset);
  FD_SET(listensock, &readset);

  while (1) {
    testset = readset;
    result = select(FD_SETSIZE, &testset, NULL, NULL, NULL);
    if (result < 0) {
      perror("server1");
      return 0;
    }

    for (x = 0; x < FD_SETSIZE; x++) {
      if (FD_ISSET(x, &testset)) {
        if (x == listensock) {
          newsock = accept(listensock, NULL, NULL);
          FD_SET(newsock, &readset);
        } else {
          nread = recv(x, buffer, 25, 0);
          if (nread <= 0) {
            close(x);
            FD_CLR(x, &readset);
            printf("client on descriptor #%i disconnected\n", x);
          } else {
            char **pieces;
            if ( threads == 0) {
              threads = atoi(buffer);
              int size_perpiece = ( strlen(MSG) / threads );
              printf("threads: %d; size of pieces: %d\n",threads,size_perpiece);
              pieces = (char**)malloc(threads*sizeof(char*));
              int i = 0;
              for (i; i < threads; i++) {
                pieces[i] = (char*)malloc( (threads+1) * sizeof(char));
                strncpy(pieces[i], MSG+(size_perpiece * i), size_perpiece);
                printf("%d.piece: %s\n", i, pieces[i]);
              }
            } else {
              int id_client = atoi(buffer);
              printf("%d.piece( \"%s\" ) is sending to client #%d\n",
                    id_client,
                    pieces[id_client],
                    id_client);
              send(x, pieces[id_client], sizeof(pieces[id_client]), 0);
            }
            buffer[nread] = '\0';
            printf("%s\n", buffer);
            send(x, buffer, nread, 0);
          }
        }
      }
    }
  }
}
