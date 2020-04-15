
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message
{
  int messageType;
  char *file_path;
  int isSuccess;
} message_t;

// convert msg to CJson, then convert to string
char *encodeMessage(struct message msg);

// convert json_msg string to CJson, then convert CJson to message struct
struct message decodeMessage(char *json_msg);
