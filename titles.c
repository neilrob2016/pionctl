#include "globals.h"

#define TIME_POS 20
#define TEXT_POS 37

static u_char **titles;
static int titles_pos;
static int titles_alloc;


void initTitles()
{
	titles = NULL;
	titles_pos = 0;
	titles_alloc = 0;
}




void addTitle(u_char *mesg, uint32_t len)
{
	int prev_pos;

	/* See if last title was the same, if so ignore */
	if (len && titles_pos)
	{
		prev_pos = titles_pos - 1;
		if (strlen((char *)titles[prev_pos]) == len + TEXT_POS &&
		    !memcmp(titles[prev_pos]+TEXT_POS,mesg,len)) 
		{
			return;
		}
	}

	/* Allocate new memory in blocks. Could do 1 by 1 but thats not very
	   efficient */
	if (titles_pos >= titles_alloc)
	{
		titles_alloc += ALLOC_BLOCK;
		titles = (u_char **)realloc(
			titles,titles_alloc * sizeof(u_char **));
		assert(titles);
	}

	/* Allocate title. Change *_POS if asprintf changed */
	asprintf((char **)&titles[titles_pos],"%s  %s  %s  %-5d  %.*s",
		getConnectTime(),timer_str,getTime(),len,len,mesg);

	++titles_pos;
}




int printTitles(int xtitles, u_char *pat, int max)
{
	int pos;
	int cnt;

	if (max < 0)
	{
		printf("Usage: %stitles [<pattern> [<count>]]\n",
			xtitles ? "x" : "");
		return ERR_CMD_FAIL;
	}

	if (!titles_pos)
	{
		puts("No titles received.");
		return OK;
	}

	puts("\n*** Titles ***\n");
	if (xtitles) 
	{
		puts("C timer   S timer   Time      Bytes  Text");
		puts("--------  --------  --------  -----  ----");
	}
	else
	{
		puts("Time      Bytes  Text");
		puts("--------  -----  ----");
	}
	for(pos=cnt=0;pos < titles_pos && (!max || cnt < max);++pos)
	{
		if (!pat || 
		    wildMatch((char *)titles[pos]+TEXT_POS,(char *)pat))
		{
			if (xtitles)
				puts((char *)titles[pos]);
			else
				puts((char *)titles[pos] + TIME_POS);
			++cnt;
		}
	}
	if (pat || max)
		printf("\n%d of %d entries\n\n",cnt,titles_pos);
	else
		printf("\n%d entries\n\n",titles_pos);
	return OK;
}




void clearTitles()
{
	int i;
	for(i=0;i < titles_pos;++i) free(titles[i]);
	free(titles);
	initTitles();
	puts("Titles cleared.");
}
