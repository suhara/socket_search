#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>


#define PORT 46490
#define BUFFLEN 1024

int
main (int argc, char *argv[])
{

  struct sockaddr_in server;

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  
  // struct hostent *h = gethostbyname("localhost");
  // or struct hostent *h = gethostbyname("rx8");
  // memcpy(&server.sin_addr.s_addr, h->h_addr, h->h_length);

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  // inet_aton("127.0.0.1", &server.sin_addr);

  server.sin_port = htons(PORT);
  
  int s;
  if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "Failed to socket().\n");
    exit(1);
  }


  if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
    fprintf(stderr, "Failed to connect.\n");
    exit(1);
  }

  char buff[ BUFFLEN ];

  char *inputmsg = "input query > ";
  int inputmsg_len = strlen(inputmsg) + 1;

  char *recvmsg = "search results: ";
  int recvmsg_len = strlen(recvmsg) + 1;
  
  do {
    write(1, inputmsg, inputmsg_len);
    int n = read(0, buff, BUFFLEN);
    if (buff[0] == 'q') break;

    write(s, buff, n);

    n = read(s, buff, BUFFLEN);
    write(1, recvmsg, recvmsg_len);
    write(1, buff, n);
    write(1, "\n", 1);

  } while (buff[0] != 'q');

  return 0;
}
