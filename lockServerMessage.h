
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct message
{
  int messageType;
  char file_path[1024]; // Limit file path to 1024 chars
  int isSuccess;
  int mode;
} message_t;

void encodeMessage(message_t msg, char *buf);

message_t decodeMessage(char *json_msg);
