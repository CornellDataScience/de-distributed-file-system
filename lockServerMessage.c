#include "lockServerMessage.h"
#include "cJSON.h"

void encodeMessage(message_t msg, char *buf)
{
  cJSON *monitor = cJSON_CreateObject();
  cJSON_AddStringToObject(monitor, "file_path", msg.file_path);
  cJSON_AddNumberToObject(monitor, "isSuccess", msg.isSuccess);
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
  msg.isSuccess = cJSON_GetObjectItemCaseSensitive(monitor_json, "isSuccess")->valueint;
  msg.messageType = cJSON_GetObjectItemCaseSensitive(monitor_json, "messageType")->valueint;
  cJSON_Delete(monitor_json);
  return msg;
}

// int main()
// {
//   struct message msg;
//   msg.messageType = 0;
//   strcpy(msg.file_path, "file.txt");
//   msg.isSuccess = 1;
//   char buffer[1024];
//   encodeMessage(msg, buffer);
//   printf("%s\n", buffer);
//   msg = decodeMessage(buffer);
//   printf("%s\n", msg.file_path);
//   printf("%i\n", msg.isSuccess);
//   printf("%i\n", msg.messageType);
//   return 0;
// }