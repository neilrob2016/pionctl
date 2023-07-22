#include "globals.h"
#include "build_date.h"


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




void version(int print_pid)
{
	colPrintf("\n~BM~FW*** PIONCTL - Pioneer N-70AE control client ***\n\n");
	colPrintf("~FTCopyright (C) Neil Robertson 2021-2023\n\n");
	colPrintf("~FYVersion~RS   : %s\n",VERSION);
	colPrintf("~FGBuild date~RS: %s\n",BUILD_DATE);
	if (print_pid) colPrintf("~FBPID~RS       : %d\n",getpid());
	putchar('\n');
}




void ok(void)
{
	colPrintf("~FGOK\n");
}
