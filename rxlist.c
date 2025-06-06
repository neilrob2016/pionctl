#include "globals.h"

void setEntryValue(t_rx_entry *entry, char *value, int val_len);
t_rx_entry *findInList(char *key);


void initRXList(void)
{
	bzero(rx_list,sizeof(rx_list));
}




/*** Add data to the list. Returns 1 if new entry or updated value ***/
int updateRXList(char *key, char *value, int val_len)
{
	t_rx_entry *entry;
	t_rx_entry *end;
	char key2[4];
	int index;

	/* Key won't be null terminated but will always be 3 characters */
	memcpy(key2,key,3);
	key2[3] = 0;

	/* See if we already have the entry */
	if ((entry = findInList(key2)))
	{
		/* Check stored value */
		if (entry->val_len != val_len || 
		    memcmp(entry->value,value,val_len))
		{
			/* Update value */
			setEntryValue(entry,value,val_len);
			return 1;
		}
		return 0;
	}

	/* New entry */
	assert((entry = (t_rx_entry *)malloc(sizeof(t_rx_entry))));
	bzero(entry,sizeof(t_rx_entry));

	assert((entry->key = (char *)malloc(sizeof(key2))));
	strcpy(entry->key,key2);

	setEntryValue(entry,value,val_len);

	index = key[0];
	if (rx_list[index])
	{
		/* Find end of particular list. We don't store the end pointer 
		   as we'd need 256 of them */
		for(end=rx_list[index];end->next;end=end->next);
		end->next = entry;
	}
	else rx_list[index] = entry;
	return 1;
}




void setUnknownRXKey(char *key)
{
	t_rx_entry *entry;
	char key2[4];

	key2[0] = key[0];
	key2[1] = key[1];
	key2[2] = key[2];
	key2[3] = 0;
	if ((entry = findInList(key2))) entry->unknown = 1;
}




/*** Assumes key is the first 3 chars of the command ***/
void clearValueOfRXKey(char *key)
{
	t_rx_entry *entry;
	char key2[4];

	memcpy(key2,key,3);
	key2[3] = 0;
	if ((entry = findInList(key2))) setEntryValue(entry,NULL,0);
}




void clearRXList(void)
{
	t_rx_entry *entry;
	int i;

	for(i=0;i < 256;++i)
	{
		for(entry=rx_list[i];entry;entry=entry->next)
		{
			free(entry->key);
			FREEIF(entry->value);
		}
		FREEIF(rx_list[i]);
	}
	bzero(rx_list,sizeof(rx_list));
	colPrintf("RX list ~FGcleared.\n");
}




int dumpRXList(char *pat, int max)
{
	t_rx_entry *entry;
	int total;
	int entry_cnt;
	int unknown_cnt;
	int i;

	if (max < 0)
	{
		usagePrintf("showr [<pattern> [<count>]]\n");
		return ERR_CMD_FAIL;
	}

	colPrintf("\n~BB~FW*** Raw streamer RX (%s : %sCONNECTED) ***\n\n",
		inet_ntoa(con_addr.sin_addr),tcp_sock ? "" : "DIS");
	colPrintf("~FRCom   ~FGBytes   ~FBMessage\n");
	colPrintf("~FT---   -----   -------\n");

	for(i=unknown_cnt=entry_cnt=total=0;i < 256;++i)
	{
		for(entry=rx_list[i];entry;entry=entry->next)
		{
			/* Lower case means its stored data, not a stored
			   streamer response */
			if (entry->key[0] < 'A' || entry->key[0] > 'Z')
				continue;
			++total;

			/* Continue looping anyway to get correct total */
			if (max && entry_cnt == max) continue;

			if (!pat || 
			     wildMatch(entry->key,pat) ||
			     (entry->value && wildMatch(entry->value,pat)))
			{
				printf("%s %c %-6d  ",
					entry->key,
					entry->unknown ? '?' : ' ',
					entry->val_len);

				/* Converts non printing chars */
				printMesg(entry->value,entry->val_len);
				++entry_cnt;
				unknown_cnt += entry->unknown;
			}
		}
	}
	if (pat || max)
	{
		printf("\n%d of %d entries (%d unknown).\n\n",
			entry_cnt,total,unknown_cnt);
	}
	else printf("\n%d entries (%d unknown).\n\n",entry_cnt,unknown_cnt);

	return OK;
}


/********************************** INTERNAL *********************************/

void setEntryValue(t_rx_entry *entry, char *value, int val_len)
{
	FREEIF(entry->value);
	if (val_len > 0)
	{
		assert((entry->value = (char *)malloc(val_len+1)));
		memcpy(entry->value,value,val_len);
		entry->value[val_len] = 0;
		entry->val_len = val_len;
	}
	else
	{
		entry->value = NULL;
		entry->val_len = 0;
	}
}




t_rx_entry *findInList(char *key)
{
	t_rx_entry *entry;

	for(entry=rx_list[(int)key[0]];entry;entry=entry->next)
		if (!strcmp(key,entry->key)) return entry;
	return NULL;
}
