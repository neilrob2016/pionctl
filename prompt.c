#include "globals.h"

void printPromptLocalTime(void);
void printPromptConnectTime(void);
void printPromptTrackTime(void);


void printPrompt(void)
{
	if (input_state != INPUT_CMD)
	{
		/* Macro input */
		colPrintf("~FM]~RS ");
		fflush(stdout);
		return;
	}
	putchar('\r');
	if (prompt_type > PROMPT_BASE) colPrintf("~FTPIONCTL~RS");

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
		write(STDOUT_FILENO,buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
}




void printPromptLocalTime(void)
{
	colPrintf(" ~FBL~RS%s",getTime());
}




void printPromptConnectTime(void)
{
	colPrintf(" ~FRC~RS%s",getTimeString(connect_time));
}




void printPromptTrackTime(void)
{
	colPrintf(" ~FGT~RS%s",track_time_str);
}




void clearPrompt(void)
{
	putchar('\r');
	write(STDOUT_FILENO,"\033[2K\r",5); /* Clears the whole line */
	fflush(stdout);
}
