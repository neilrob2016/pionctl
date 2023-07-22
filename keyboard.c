#include "globals.h"

#define CONTROL_D 4
#define ESC       27
#define DEL1      8   /* ASCII/terminal backspace */
#define DEL2      127 /* PC keyboard backspace */

void initKeyboard(void)
{
	struct termios tio;

	printf("Init keyboard... ");
	fflush(stdout);

	/* Get current state */
	if (tcgetattr(STDIN,&tio) == -1)
	{
		errPrintf("initKeyboard(): tcgetattr(): %s\n",strerror(errno));
		doExit(1);
	}
	saved_tio = tio;

	/* Echo off, canonical off */
	tio.c_lflag &= ~ECHO;
	tio.c_lflag &= ~ICANON;

	/* Min return 1 byte, no delay */
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	/* Set new state */
	if (tcsetattr(STDIN,TCSANOW,&tio) == -1)
	{
		errPrintf("initKeyboard(): tcsetattr()\n",strerror(errno));
		doExit(1);
	}

	keyb_buffnum = BUFF_KEYB_FIRST;
	from_buffnum = BUFF_KEYB_FIRST;

	ok();
}




void readKeyboard(void)
{
	enum 
	{
		ESC_DELETE,
		ESC_UP_ARROW,
		ESC_DOWN_ARROW,
		ESC_LEFT_ARROW,

		NUM_ESCAPE_CODES
	};
	/* Doesn't include the initial escape */
	static char *escape_code[NUM_ESCAPE_CODES] =
	{
		"[3~",
		"[A",
		"[B",
		"[D"
	};
	char s[100];
	int code;
	int len;
	int i;

	/* If user presses a key that produces an escape code, eg arrow keys,
	   or does a cut and paste then all the characters will be returned in 
	   1 read hence reading in an array not just a single character */
	if ((len = read(STDIN,s,sizeof(s)-1)) == -1)
	{
		errPrintf("readKeyboard(): read()\n",strerror(errno));
		return;
	}
	switch(s[0])
	{
	case '\n':
		putchar('\n');
		parseInputLine(
			buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
		from_buffnum = keyb_buffnum;
		clearBuffer(keyb_buffnum);
		break;

	case CONTROL_D:
		quitPrintf("by Control-D");
		doExit(0);
		/* Avoids gcc warning */
		break;

	case ESC:
		if (len == 1)
		{
			clearPrompt();
			clearBuffer(keyb_buffnum);
			break;
		}
		s[len] = 0;
		for(code=0;
		    code < NUM_ESCAPE_CODES && strcmp(s+1,escape_code[code]);
		    ++code);
		switch(code)
		{
		case ESC_LEFT_ARROW:
		case ESC_DELETE:
			delLastCharFromBuffer(keyb_buffnum);
			write(STDOUT,"\b ",2);
			break;

		case ESC_UP_ARROW:
		case ESC_DOWN_ARROW:
			/* If not all buffers filled need to find filled one */
			for(i=0;i < MAX_HIST_BUFFERS;++i)
			{
				if (code == ESC_UP_ARROW)
				{
					if (--from_buffnum < 0)
						from_buffnum = BUFF_KEYB_LAST;
				}
				else from_buffnum = (from_buffnum + 1) % MAX_HIST_BUFFERS;
				if (from_buffnum != keyb_buffnum &&
				    buffer[from_buffnum].len)
				{
					clearPrompt();
					copyBuffer(from_buffnum,keyb_buffnum);
					break;
				}
			}
			break;
			
		default:
			return;
		}
		break;
		
	case DEL1:  /* ASCII/terminal backspace */
	case DEL2:  /* PC keyboard backspace */
		if (delLastCharFromBuffer(keyb_buffnum))
			write(STDOUT,"\b \b",3);
		return;

	default:
		write(STDOUT,s,len);
		addToBuffer(keyb_buffnum,s,len);
		return;
	}
	printPrompt();
}




void resetKeyboard(void)
{
	tcsetattr(STDIN,TCSANOW,&saved_tio);
}
