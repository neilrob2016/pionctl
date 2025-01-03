#include "globals.h"

static int titles_alloc;


void initTitles(void)
{
	titles = NULL;
	titles_pos = 0;
	titles_alloc = 0;
}




void addTitle(char *mesg, uint32_t len)
{
	int prev_pos;
	uint32_t i;

	/* Remove non printing */
	for(i=0;i < len;++i) if (mesg[i] < 32) mesg[i] = '.';

	/* See if last title was the same, if so ignore */
	if (len && titles_pos)
	{
		prev_pos = titles_pos - 1;
		if (strlen(titles[prev_pos]) == len + TITLE_TEXT_POS &&
		    !memcmp(titles[prev_pos] + TITLE_TEXT_POS,mesg,len)) 
		{
			return;
		}
	}

	/* Allocate new memory in blocks. Could do 1 by 1 but thats not very
	   efficient */
	if (titles_pos >= titles_alloc)
	{
		titles_alloc += ALLOC_BLOCK;
		titles = (char **)realloc(
			titles,titles_alloc * sizeof(char **));
		assert(titles);
	}

	/* Allocate title. Change *_POS if asprintf changed */
	asprintf((char **)&titles[titles_pos],"%s  %s  %s  %-5d  %.*s",
		getTimeString(connect_time),
		track_time_str,getTime(),len,len,mesg);

	++titles_pos;
}




int printTitles(int xtitles, char *pat, int max)
{
	int pos;
	int cnt;

	if (!titles_pos)
	{
		puts("No titles received.");
		return OK;
	}

	colPrintf("\n~BB~FW*** Titles ***\n\n");
	if (xtitles) 
	{
		colPrintf("~FRCon time~RS  ~FGTrk time~RS  ~FBLoc time~RS  Bytes  Text\n");
		colPrintf("~FT--------  --------  --------  -----  ----\n");
	}
	else
	{
		colPrintf("~FBLoc time~RS  Bytes  Text\n");
		colPrintf("~FT--------  -----  ----\n");
	}
	for(pos=cnt=0;pos < titles_pos && (!max || cnt < max);++pos)
	{
		if (!pat || wildMatch(titles[pos] + TITLE_TEXT_POS,pat))
		{
			if (xtitles)
				puts(titles[pos]);
			else
				puts(titles[pos] + TITLE_TIME_POS);
			++cnt;
		}
	}
	if (pat || max)
		printf("\n%d of %d entries.\n\n",cnt,titles_pos);
	else
		printf("\n%d entries.\n\n",titles_pos);
	return OK;
}




void clearTitles(void)
{
	int i;
	for(i=0;i < titles_pos;++i) free(titles[i]);
	free(titles);
	initTitles();
	colPrintf("Titles ~FGcleared.\n");
}
