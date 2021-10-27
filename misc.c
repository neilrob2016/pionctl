#include "globals.h"
#include "build_date.h"

#define TIMER_STR_LEN 8

/*** Returns 1 if the string matches the pattern, else 0. Supports wildcard 
     patterns containing '*' and '?'. Case insensitive. ***/
int wildMatch(char *str, char *pat)
{
	char *s,*p,*s2;

	for(s=str,p=pat;*s && *p;++s,++p)
	{
		switch(*p)
		{
		case '?':
			continue;

		case '*':
			if (!*(p+1)) return 1;
			for(s2=s;*s2;++s2) if (wildMatch(s2,p+1)) return 1;
			return 0;
		}
		if (toupper(*s) != toupper(*p)) return 0;
	}

	/* Could have '*' leftover in the pattern which can match nothing.
	   eg: "abc*" should match "abc" and "*" should match "" */
	if (!*s)
	{
		/* Could have *'s on the end which should all match "" */
		for(;*p && *p == '*';++p);
		if (!*p) return 1;
	}

	return 0;
}



int isNumber(u_char *str, int len)
{
	int i;
	for(i=0;i < len && isdigit(str[i]);++i);
	return (i == len);
}




void printPrompt()
{
	if (input_state != INPUT_CMD)
	{
		write(STDOUT,"] ",2);
		return;
	}
	write(STDOUT,"\rPIONCTL",8);

	switch(prompt_type)
	{
	case PROMPT_BASE:
		break;

	case PROMPT_TIME:
		write(STDOUT," T",2);
		write(STDOUT,getTime(),8);
		break;

	case PROMPT_CONN_TIMER:
		write(STDOUT," C",2);
		write(STDOUT,getConnectTime(),8);
		break;

	case PROMPT_STRM_TIMER:
		write(STDOUT," S",2);
		write(STDOUT,timer_str,strlen(timer_str));
		break;

	default:
		assert(0);
	}
	write(STDOUT,">",1);

	if (buffer[keyb_buffnum].len)
		write(STDOUT,buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
}




void clearPrompt()
{
	int i;
	write(STDOUT,"\r                         ",26);

	for(i=buffer[keyb_buffnum].len+1;i >= 0;--i) putchar(' ');
	putchar('\r');
	fflush(stdout);
}




char *getConnectTime()
{
	static char text[9];
	time_t diff;
	int hours;
	int mins;
	int secs;

	if (connect_time)
	{
		diff = time(0) - connect_time;
		hours = (diff / 3600) % 100;
		mins = (diff % 3600) / 60;
		secs = diff % 60;
		snprintf(text,9,"%02d:%02d:%02d",hours,mins,secs);
	}
	else strcpy(text,"--.--.--");

	return text;
}




char *getTime()
{
	static char text[9];
	time_t now = time(0);

	strftime(text,sizeof(text),"%T",localtime(&now));
	return text;
}




void doExit(int code)
{
	resetKeyboard();
	exit(code);
}




void sigHandler(int sig)
{
	switch(sig)
	{
	case SIGINT:
		puts("*** BREAK ***");
		clearBuffer(keyb_buffnum);
		discardMultiLineMacro();
		if (!FLAGISSET(FLAG_MACRO_RUNNING)) printPrompt();
		SETFLAG(FLAG_INTERRUPTED);
		break;
	case SIGQUIT:
	case SIGTERM:
		printf("\n*** EXIT on signal %d ***\n",sig);
		doExit(sig);
	default:
		assert(0);
	}
}




void version(int print_pid)
{
	puts("\n*** PIONCTL - Pioneer N-70AE control client ***\n");
	printf("Version   : %s\n",VERSION);
	printf("Build date: %s\n",BUILD_DATE);
	if (print_pid) printf("PID       : %d\n",getpid());
	putchar('\n');
}
