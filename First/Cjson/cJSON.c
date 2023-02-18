#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
int generate_sendjson(char* send_message,const char* type,const char* msg);
int main()
{
    char buf_inputmessage[1024] = {0};
    cJSON *input_json = NULL;
    char *input_message = NULL;
    input_message = (char *)malloc(1024);
    char *output_message = NULL;
    output_message = (char *)malloc(1024);
    fgets(buf_inputmessage,1024,stdin);
    generate_sendjson(input_message,"login",buf_inputmessage);
    printf("%s\n",input_message);
    return 0;
}
int generate_sendjson(char* send_message,const char* type,const char* msg)
{
    cJSON *input_json = NULL;
    char *input_message = NULL;
    input_message = (char *)malloc(1024);
    char *output_message = NULL;
    output_message = (char *)malloc(1024);
    input_json = cJSON_CreateObject();
    cJSON_AddStringToObject(input_json,"type",type);
    cJSON_AddStringToObject(input_json,"msg",msg);
    input_message = cJSON_Print(input_json);
    strcpy(send_message,input_message);
    printf("%s\n",send_message);
}

