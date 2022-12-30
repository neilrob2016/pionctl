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
	colprintf("\n~BM~FW*** PIONCTL - Pioneer N-70AE control client ***\n\n");
	colprintf("~FTCopyright (C) Neil Robertson 2021-2022\n\n");
	colprintf("~FYVersion~RS   : %s\n",VERSION);
	colprintf("~FGBuild date~RS: %s\n",BUILD_DATE);
	if (print_pid) colprintf("~FBPID~RS       : %d\n",getpid());
	putchar('\n');
}




void ok()
{
	colprintf("~FGOK\n");
}
