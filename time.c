#include "globals.h"

char *getTime(void)
{
	static char text[20];
	time_t now = time(0);

	strftime(text,sizeof(text),"%T",localtime(&now));
	return text;
}




char *getTimeString(time_t tm)
{
	return (tcp_sock && tm) ? getRawTimeString(tm) : TIME_DEF_STR;
}




char *getRawTimeString(time_t tm)
{
	static char text[20];
	time_t diff;
	int hours;
	int mins;
	int secs;

	diff = time(0) - tm;
	hours = (diff / 3600) % 100;
	mins = (diff % 3600) / 60;
	secs = diff % 60;
	snprintf(text,sizeof(text),"%02d:%02d:%02d",hours,mins,secs);

	return text;
}
