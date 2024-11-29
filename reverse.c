#include "globals.h"

static int rev_bot[2];
static int rev_top[2];
static int rev_start[2];


void initArray(int arr)
{
	rev_bot[arr] = 0;
	rev_top[arr] = 0;
	rev_start[arr] = 1; 
}




void initReverse(void)
{
	initArray(0);
	initArray(1);
	rev_arr = 0;
	flags.reset_reverse = 0;
}




void clearReverse(void)
{
	initReverse();
	colPrintf("Menu reverse navigation commands ~FGcleared.\n");
}




void addReverseCom(int ra, int com, int repeat_cnt)
{
	int rt;

	/* Reset if we've done other streamer commands since we last
	   navigated so we don't end up with a huge reverse list */
	if (flags.reset_reverse) initReverse();

	rt = rev_top[ra];
	revcom[ra][rt].com = getReverseCom(com);
	revcom[ra][rt].repeat_cnt = repeat_cnt;
	revcom[ra][rt].seq_start = rev_start[ra];
	rev_start[ra] = 0;
	rev_top[ra] = (rt + 1) % MAX_HIST_BUFFERS;
	if (rev_top[ra] == rev_bot[ra]) 
		rev_bot[ra] = (rev_bot[ra] + 1) % MAX_HIST_BUFFERS;
}




int getReverseCom(int com)
{
	switch(com)
	{
	case COM_UP: return COM_DN;
	case COM_DN: return COM_UP;
	case COM_EN: return COM_EX;
	case COM_EX: return COM_EN;
	}
	assert(0);
	return 0;
}




void runShowReverse(int run, char *param)
{
	struct st_rev_com *rev;
	char *cmd;
	float secs = 0;
	int rev_rev_arr = 0;
	int start_pos = 0;
	int cnt = 0;
	int pos;

	if (rev_top[rev_arr] == rev_bot[rev_arr])
	{
		puts("No menu reverse commands stored.");
		return;
	}
	if (run)
	{
		if (!tcp_sock)
		{
			errNotConnected();
			return;
		}
		if (param)
		{
			if ((secs = atof(param)) <= 0)
			{
				usagePrintf("rev [<wait seconds>]\n");
				return;
			}
		}
		rev_rev_arr = !rev_arr;
		initArray(rev_rev_arr);
	}
	else
	{
		colPrintf("\n~BB~FW*** Reverse menu navigation commands in order of execution ***\n");
		colPrintf("\n~FGCmd  ~FMRep\n");
		puts("===  ===");
	}

	/* Have to execute the commands in reverse so do it from the top to 
	   the bottom. Currently we only do UP, DN, EN, EX so can skip a 
	   load of checks that parseCommand() would do. Only execute back to
	   the sequence start */
	start_pos = rev_top[rev_arr] - 1;
	for(pos=start_pos;;--pos)
	{
		if (pos < 0) pos = MAX_HIST_BUFFERS - 1;
		rev = &revcom[rev_arr][pos];
		cmd = commands[rev->com].com;
		if (run)
		{
			colPrintf("~FMReverse command:~RS %d %s\n",
				rev->repeat_cnt,cmd);
			sendCommand(
				rev->repeat_cnt,
				commands[rev->com].data,
				strlen(commands[rev->com].data));

			/* Wait until a menu appears or timeout happens */
			if (doWait(COM_WAIT_MENU,secs) != OK)
			{
				/* Can't leave half modified list so clear it */
				if (pos != start_pos) clearReverse();
				return;
			}

			/* Store the reverse of reverse commands in the other 
			   array so the "back" command can switch back and
			   forth between menu positions */
			addReverseCom(!rev_arr,rev->com,rev->repeat_cnt);
		}
		else
		{
			printf("%s   %3d\n",cmd,rev->repeat_cnt);
			++cnt;
		}
		if (pos == rev_top[rev_arr] || rev->seq_start) break;
	}
	if (run)
		rev_arr = rev_rev_arr;
	else
		printf("\n%d commands.\n\n",cnt);
}
