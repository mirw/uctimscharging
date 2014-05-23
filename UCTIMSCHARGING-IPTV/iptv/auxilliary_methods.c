/***
Vitalis G .Ozianyi
**/

#include "includes.h"
#include "auxilliary_methods.h"

char* aiptv_get_time()
{
	char hour[50];
	char min[50];
	char sec[50];
	char time_str[200];
	char *return_val;
	
	struct tm *now = NULL;
	time_t time_value = 0;
	time_value = time(NULL);
	
	now = localtime(&time_value);
	if(now->tm_hour < 10)
	{
		sprintf(hour,"0%d",now->tm_hour);
	}
	else
	{
		sprintf(hour,"%d",now->tm_hour);
	}
	if(now->tm_min < 10)
	{
		sprintf(min,"0%d",now->tm_min);
	}
	else
	{
		sprintf(min,"%d",now->tm_min);
	}
	if(now->tm_sec < 10)
	{
		sprintf(sec,"0%d",now->tm_sec);
	}
	else
	{
		sprintf(sec,"%d",now->tm_sec);
	}

	
	sprintf(time_str, "%s:%s:%s",hour, min,sec);
	return_val = strdup(time_str);

	return return_val;
}

void aiptv_log(char *message)
{

	if (DEBUG)
	{
		char debugMessage[strlen(message)];
		sprintf(debugMessage, "\n%s - UCT Advanced IPTV:\n%s\n", aiptv_get_time(), message);

		fprintf(stderr, debugMessage);
	}

}

void aiptv_log_message(osip_message_t *sip_message)
{
char *dest = NULL;
size_t length = 0;
osip_message_to_str (sip_message, &dest, &length);
aiptv_log(dest);
osip_free(dest);
}


