#include "globals.h"

void printPromptLocalTime();
void printPromptConnectTime();
void printPromptTrackTime();


void printPrompt()
{
	if (input_state != INPUT_CMD)
	{
		/* Macro input */
		colprintf("~FM]~RS ");
		fflush(stdout);
		return;
	}
	putchar('\r');
	if (prompt_type > PROMPT_BASE) colprintf("~FTPIONCTL~RS");

	switch(prompt_type)
	{
	case PROMPT_BASE:
	case PROMPT_NAME:
		break;
	case PROMPT_C_TIME:
		printPromptConnectTime();
		break;
	case PROMPT_T_TIME:
		printPromptTrackTime();
		break;
	case PROMPT_L_TIME:
		printPromptLocalTime();
		break;
	case PROMPT_C_T_TIME:
		printPromptConnectTime();
		printPromptTrackTime();
		break;
	case PROMPT_C_L_TIME:
		printPromptConnectTime();
		printPromptLocalTime();
		break;
	case PROMPT_T_L_TIME:
		printPromptTrackTime();
		printPromptLocalTime();
		break;
	case PROMPT_C_T_L_TIME:
		printPromptConnectTime();
		printPromptTrackTime();
		printPromptLocalTime();
		break;
	default:
		assert(0);
	}
	putchar('>');
	fflush(stdout);

	if (buffer[keyb_buffnum].len)
		write(STDOUT,buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
}




void printPromptLocalTime()
{
	colprintf(" ~FBL~RS%s",getTime());
}




void printPromptConnectTime()
{
	colprintf(" ~FRC~RS%s",getTimeString(connect_time));
}




void printPromptTrackTime()
{
	colprintf(" ~FGT~RS%s",track_time_str);
}




void clearPrompt()
{
	int i;

	/* Clear the prompt itself */
	printf("\r                             ");

	/* Clear anything in the keyboard buffer */
	for(i=buffer[keyb_buffnum].len+1;i >= 0;--i) putchar(' ');

	putchar('\r');
	fflush(stdout);
}
