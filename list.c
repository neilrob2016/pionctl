#include "globals.h"


void initList()
{
	bzero(list,sizeof(list));
}




/*** Add data to the list. Returns 1 if new entry or updated value ***/
int updateList(char *key, u_char *value, int val_len)
{
	t_entry *entry;
	t_entry *end;
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
	assert((entry = (t_entry *)malloc(sizeof(t_entry))));
	bzero(entry,sizeof(t_entry));

	assert((entry->key = (char *)malloc(4)));
	strcpy(entry->key,key2);

	setEntryValue(entry,value,val_len);

	index = key[0];
	if (list[index])
	{
		/* Find end of particular list. We don't store the end pointer 
		   as we'd need 256 of them */
		for(end=list[index];end->next;end=end->next);
		end->next = entry;
	}
	else list[index] = entry;
	return 1;
}




void setEntryValue(t_entry *entry, u_char *value, int val_len)
{
	FREE(entry->value);
	if (val_len)
	{
		assert((entry->value = (u_char *)malloc(val_len+1)));
		memcpy(entry->value,value,val_len);
		entry->value[val_len] = 0;
	}
	else entry->value = NULL;

	entry->val_len = val_len;
}




void setUnknownKey(char *key)
{
	t_entry *entry;
	char key2[4];

	memcpy(key2,key,3);
	key2[3] = 0;
	if ((entry = findInList(key2))) entry->unknown = 1;
}




/*** Assumes key is the first 3 chars of the command ***/
void clearValueOfKey(char *key)
{
	t_entry *entry;
	char key2[4];

	memcpy(key2,key,3);
	key2[3] = 0;
	if ((entry = findInList(key2))) setEntryValue(entry,NULL,0);
}




void clearList()
{
	t_entry *entry;
	int i;

	for(i=0;i < 256;++i)
	{
		for(entry=list[i];entry;entry=entry->next)
		{
			free(entry->key);
			FREE(entry->value);
		}
		FREE(list[i]);
	}
	bzero(list,sizeof(list));
	puts("RX list cleared.");
}




int dumpList(u_char *pat, int max)
{
	t_entry *entry;
	int total;
	int entry_cnt;
	int unknown_cnt;
	int i;

	if (max < 0)
	{
		puts("Usage: showr [<pattern> [<count>]]");
		return CMD_ERROR;
	}

	printf("\n*** Raw streamer RX (%s%s) ***\n\n",
		inet_ntoa(con_addr.sin_addr),tcp_sock ? "" : " - offline");
	puts("Com   Bytes   Message");
	puts("---   -----   -------");
	for(i=unknown_cnt=entry_cnt=total=0;i < 256;++i)
	{
		for(entry=list[i];entry;entry=entry->next)
		{
			/* Lower case means its stored data, not a stored
			   streamer response */
			if (entry->key[0] < 'A' || entry->key[0] > 'Z')
				continue;
			++total;

			/* Continue looping anyway to get correct total */
			if (max && entry_cnt == max) continue;

			if (!pat || 
			     wildMatch(entry->key,(char *)pat) ||
			     (entry->value && wildMatch((char *)entry->value,(char *)pat)))
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
		printf("\n%d of %d entries (%d unknown)\n\n",
			entry_cnt,total,unknown_cnt);
	}
	else printf("\n%d entries (%d unknown)\n\n",entry_cnt,unknown_cnt);

	return CMD_OK;
}




t_entry *findInList(char *key)
{
	t_entry *entry;

	for(entry=list[(int)key[0]];entry;entry=entry->next)
		if (!strcmp(key,entry->key)) return entry;
	return NULL;
}
