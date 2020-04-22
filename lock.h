#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lockServerMessage.h"

typedef struct lock
{
  int in_use;                     //0 = not in use, 1 = in use
  int client_id;                  //client using this lock
  char file_path[1024];           //path this lock is locking
  int waiting_buffer[UINT32_MAX]; //other clients waiting on this lock

} lock_t;

void acquire_lock(message_t lock_msg, int client_id);
void release_lock(message_t lock_msg, int client_id);