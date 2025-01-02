#include "globals.h"


int doWait(int comnum, float secs)
{
	struct timeval tv;
	struct timeval *tvp;
	fd_set mask;
	u_int usecs;
	u_int end = 0;
	int final = 0;

	usecs = (u_int)(secs * 1000000);

	if (!tcp_sock)
	{
		/* We're not connected to anything */
		if (comnum == COM_WAIT_MENU)
		{
			errNotConnected();
			return ERR_CMD_FAIL;
		}
		return (usleep(usecs) == -1 && errno == EINTR) ? ERR_CMD_FAIL : OK;
	}

	if (usecs) end = getUsecTime() + usecs;

	/* Loop until we timeout or read a menu depending on the command */
	while(1)
	{
		FD_ZERO(&mask);
		FD_SET(tcp_sock,&mask);

		if (usecs || final)
		{
			usecs = end - getUsecTime();
			tv.tv_sec = usecs / 1000000;
			tv.tv_usec = usecs % 1000000;
			tvp = &tv;
		}
		else tvp = NULL;

		switch(select(FD_SETSIZE,&mask,0,0,tvp))
		{
		case -1:
			return (errno == EINTR) ? ERR_CMD_FAIL : OK;
		case 0:
			/* Wait time expired */
			if (comnum == COM_WAIT_MENU)
			{
				if (final) return OK; 
				errPrintf("TIMEOUT\n");
				return ERR_CMD_FAIL;
			}
			return OK;
		}

		/* Process anything coming from the streamer while we wait */
		readSocket(0);
		if (comnum == COM_WAIT_MENU)
		{
			/* Do final waits for everything to be sent from the
			   streamer. Can't be >= 1 sec or the every 1 second 
			   NTM message will screw us up and we'll get stuck in 
			   an endless loop as we'll never timeout */
			final = 1;
			end = getUsecTime() + 500000;
		}
	}
	return ERR_CMD_FAIL;
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
		colPrintf("~FY~OL*** BREAK ***\n");
		clearBuffer(keyb_buffnum);
		discardMultiLineMacro();
		if (!flags.macro_running) printPrompt();
		flags.interrupted = 1;
		break;
	case SIGQUIT:
	case SIGTERM:
		quitPrintf("on signal %d",sig);
		doExit(sig);
		/* Avoids gcc warning */
		break;
	default:
		assert(0);
	}
}




/*** Get the current time down to the microsecond. Value wraps once every
     1000 seconds. Goes from 0 -> 999999999 (1 billion - 1) ***/
u_int getUsecTime(void)
{               
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (u_int)(tv.tv_sec % 1000) * 1000000 + (u_int)tv.tv_usec;
}




void version(int print_pid)
{
	colPrintf("\n~BM~FW*** PIONCTL - Pioneer N-70AE control client ***\n\n");
	colPrintf("~FTCopyright (C) Neil Robertson 2021-2025\n\n");
	colPrintf("~FYVersion~RS   : %s\n",VERSION);
	colPrintf("~FGBuild date~RS: %s, %s\n",__DATE__,__TIME__);
	if (print_pid) colPrintf("~FBPID~RS       : %d\n",getpid());
	putchar('\n');
}




void errNotConnected(void)
{
	errPrintf("Not connected.\n");
}




void ok(void)
{
	colPrintf("~FGOK\n");
}
