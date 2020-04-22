#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lockServerMessage.h"

typedef struct lock
{
  int in_use;                     //0 = not in use, 1 = in use
  int lock_id;                    // id of the lock
  int client_id;                  //client using this lock
  char file_path[1024];           //path this lock is locking
  int waiting_buffer[UINT32_MAX]; //other clients waiting on this lock

} lock_t;
