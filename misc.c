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




int isNumber(char *str)
{
	char *s;
	for(s=str;*s;++s) if (!isdigit(*s)) return 0;
	return 1;
}




int isNumberWithLen(char *str, int len)
{
	int i;
	for(i=0;i < len && isdigit(str[i]);++i);
	return (i == len);
}




char *getTime()
{
	static char text[9];
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
	static char text[9];
	time_t diff;
	int hours;
	int mins;
	int secs;

	diff = time(0) - tm;
	hours = (diff / 3600) % 100;
	mins = (diff % 3600) / 60;
	secs = diff % 60;
	snprintf(text,9,"%02d:%02d:%02d",hours,mins,secs);

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
		colprintf("~FY~OL*** BREAK ***\n");
		clearBuffer(keyb_buffnum);
		discardMultiLineMacro();
		if (!flags.macro_running) printPrompt();
		flags.interrupted = 1;
		break;
	case SIGQUIT:
	case SIGTERM:
		exitprintf("on signal %d",sig);
		doExit(sig);
	default:
		assert(0);
	}
}




void version(int print_pid)
{
	colprintf("\n~BM*** PIONCTL - Pioneer N-70AE control client ***\n\n");
	colprintf("~FTCopyright (C) Neil Robertson 2021-2022\n\n");
	colprintf("~FGVersion~RS   : %s\n",VERSION);
	colprintf("~FGBuild date~RS: %s\n",BUILD_DATE);
	if (print_pid) colprintf("~FGPID~RS       : %d\n",getpid());
	putchar('\n');
}




void ok()
{
	colprintf("~FGOK\n");
}
