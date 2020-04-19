#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct message
{
  int messageType;
  char file_path[1024]; // Limit file path to 1024 chars
  int mode; // 0 - read, 1 - write, 2 - new
} client_message_t;

void encodeMessage(client_message_t msg, char *buf);

client_message_t decodeMessage(char *json_msg);
