#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "lockServerMessage.h"

#define PORT 8889 /* the port client will be connecting to */

#define MAXDATASIZE 100 /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct hostent *he;
  struct sockaddr_in their_addr; /* connector's address information */

  if (argc != 2)
  {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
  }

  if ((he = gethostbyname(argv[1])) == NULL)
  { /* get the host info */
    herror("gethostbyname");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }

  their_addr.sin_family = AF_INET;   /* host byte order */
  their_addr.sin_port = htons(PORT); /* short, network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8); /* zero the rest of the struct */

  if (connect(sockfd, (struct sockaddr *)&their_addr,
              sizeof(struct sockaddr)) == -1)
  {
    perror("connect");
    exit(1);
  }
  int connection_accepted = 0;
  while (1)
  {
    // if (connection_accepted == 0)
    // {
    //   char *message = "hello";
    //   if (send(sockfd, message, strlen(message), 0) == -1)
    //   {
    //     perror("send");
    //     exit(1);
    //   }

    //   if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1)
    //   {
    //     perror("recv");
    //     exit(1);
    //   }
    //   connection_accepted = 1;
    // }
    message_t msg;
    msg.messageType = ACQUIRE_LOCK;
    strcpy(msg.file_path, "file.txt");
    msg.isSuccess = 1;
    char buffer[1024];
    encodeMessage(msg, buffer);

    if (send(sockfd, buffer, strlen(buffer), 0) == -1)
    {
      perror("send");
      exit(1);
    }

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1)
    {
      perror("recv");
      exit(1);
    }
    printf("Received %u bytes\n", numbytes);

    buf[numbytes] = '\0';

    printf("text=: %s \n", buf);
    sleep(1);
  }

  close(sockfd);

  return 0;
}
