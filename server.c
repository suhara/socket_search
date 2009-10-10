#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "index.h"

#define PORT 46490
#define MAX_CONNECT 5
#define BUFFLEN 1024

int
main (int argc, char *argv[])
{

  int sockfd;
  struct sockaddr_in sin;

  // same as (PF_INET, SOCK_STREAM, 0)
  if ((sockfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "Faild to socket.\n");
    exit(1);
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(PORT);

  if ( bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0 ) {
    fprintf(stderr, "Failed to bind.\n");
    exit(1);
  }

  if ( listen(sockfd, MAX_CONNECT) < 0 ) {
    fprintf(stderr, "Failed to listen.\n");
    exit(1);
  }

  printf("%s listening port %d...\n", argv[0], PORT);

  // memset(&sin_c, 0, sizeof(sin_c));
  // int addrlen = sizeof(sin);
  // int cliend_fd = accept(sockfd, &sin_c, &addrlen);


  int client_fd;

  if ( (client_fd = accept(sockfd, NULL, NULL)) < 0 ) {
    fprintf(stderr, "Failed to accept.\n");
    exit(1);
  }

  char buff[ BUFFLEN ];

  while (1) {

    int n = read(client_fd, buff, BUFFLEN);
    fprintf(stderr, "Received query: %s", buff);

    int resnum;
    buff[n - 1] = '\0'; // replace newline by NULL character
    int *reslist = search_index(buff, &resnum);
    
    // Create result string 
    char result_buff[ BUFFLEN ];
    char tmp[ 256 ];

    result_buff[ 0 ] = '\0';

    if (resnum < 0) {
      strcpy(result_buff, "Index NOT found.");
    } else if (resnum == 0) {
      strcpy(result_buff, "No result found.");
    } else {
      int i;
      for (i = 0; i < resnum; i++) {
	sprintf(tmp, "%d ", reslist[ i ]);
	strcat(result_buff, tmp);
      }
    }

    fprintf(stderr, "result_buff=%s\n", result_buff);
    write(client_fd, result_buff, strlen(result_buff) + 1);
  }
  close(client_fd);

  return 0;
}

