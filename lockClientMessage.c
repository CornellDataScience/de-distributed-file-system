#include "lockClientMessage.h"
#include "cJSON.h"

void encodeMessage(client_message_t msg, char *buf)
{
  cJSON *monitor = cJSON_CreateObject();
  cJSON_AddStringToObject(monitor, "file_path", msg.file_path);
  cJSON_AddNumberToObject(monitor, "mode", msg.mode);
  cJSON_AddNumberToObject(monitor, "messageType", msg.messageType);
  cJSON_PrintPreallocated(monitor, buf, 2048, 1);
  cJSON_Delete(monitor);
}

struct message decodeMessage(char *json_msg)
{
  message_t msg;
  cJSON *monitor_json = cJSON_Parse(json_msg);
  char *s = cJSON_GetObjectItemCaseSensitive(monitor_json, "file_path")->valuestring;
  strcpy(msg.file_path, s);
  msg.mode = cJSON_GetObjectItemCaseSensitive(monitor_json, "mode")->valueint;
  msg.messageType = cJSON_GetObjectItemCaseSensitive(monitor_json, "messageType")->valueint;
  cJSON_Delete(monitor_json);
  return msg;
}

int main()
{
  struct message msg;
  msg.messageType = 0;
  strcpy(msg.file_path, "file.txt");
  msg.mode = 0;
  char buffer[1024];
  encodeMessage(msg, buffer);
  printf("%s\n", buffer);
  msg = decodeMessage(buffer);
  printf("%s\n", msg.file_path);
  printf("%i\n", msg.mode);
  printf("%i\n", msg.messageType);
  return 0;
}