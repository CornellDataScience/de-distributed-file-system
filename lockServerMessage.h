
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACQUIRE_LOCK 0
#define RELEASE_LOCK 1
#define REQUEST_READ 2
#define REQUEST_WRITE 3

#define SUCCESS 1
#define NO_SUCCESS 0
#define WAITING 2

typedef struct message
{
  int messageType;      // request,etc.
  char file_path[1024]; // Limit file path to 1024 chars
  int isSuccess;
} message_t;
