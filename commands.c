#include "globals.h"

#define ONOFF(F)  (flags & F) ? "ON" : "OFF"

/* Built in commands apart from SAVEART */
enum
{
	/* 0 */
	COM_EXIT,
	COM_VER,
	COM_DET,
	COM_STMR,
	COM_PROMPT,

	/* 5 */
	COM_RAW,
	COM_TAMP,
	COM_SHOW,
	COM_SHOWR,
	COM_CLEAR,

	/* 10 */
	COM_QMARK,
	COM_HELP,
	COM_XHELP,
	COM_SHELP,
	COM_CON,

	/* 15 */
	COM_DISCON,
	COM_HIST,
	COM_CHIST,
	COM_TITLES,
	COM_XTITLES,

	/* 20 */
	COM_WAIT,
	COM_CLS,
	COM_RXCOMS,
	COM_TIME,
	COM_LIST,
	COM_WHERE,

	/* 25 */
	COM_MENU,
	COM_UP,
	COM_DN,
	COM_EN,

	/* 30 */
	COM_EX,
	COM_MSTAT,
	COM_FLIP,
	COM_DS,
	COM_DSD,

	/* 35 */
	COM_DSSTAT,
	COM_DF,
	COM_ARTBMP,
	COM_ARTURL,
	COM_SAVEART,

	COM_SETNAME = 91,
	COM_LRA     = 106
};


/* Not a comprehensive list of commands but just the more useful ones */
static struct st_command
{
	char *com;
	char *data;
} commands[] =
{
	/* 0. Built in commands */
	{ "exit",   NULL },
	{ "ver",    NULL },
	{ "det",    NULL },
	{ "stmr",   NULL },
	{ "prompt", NULL },

	/* 5 */
	{ "raw",    NULL },
	{ "tamp",   NULL },
	{ "show",   NULL },
	{ "showr",  NULL },
	{ "clear",  NULL },

	/* 10 */
	{ "?",      NULL },
	{ "help",   NULL },
	{ "xhelp",  NULL },
	{ "shelp",  NULL },
	{ "con",    NULL },

	/* 15 */
	{ "discon", NULL },
	{ "hist",   NULL },
	{ "chist",  NULL },
	{ "titles", NULL },
	{ "xtitles",NULL },

	/* 20 */
	{ "wait",   NULL },
	{ "cls",    NULL },
	{ "rxcoms", NULL },
	{ "time",   NULL },
	{ "list",   NULL },

	/* 25 */
	{ "where",  NULL },

	/* Menu navigation */
	{ "menu",    "NTCMENU"  },
	{ "up",      "OSDUP"    }, 
	{ "dn",      "OSDDOWN"  },
	{ "en",      "OSDENTER" },
	{ "ex",      "OSDEXIT"  }, 
	{ "mstat",   "NMSQSTN"  },
	{ "flip",    "NTCLIST"  },

	/* It seems the display command system is broken - you can dim the 
	   display but not make it brighter again. Can only reset it via the 
	   remote control */
	{ "ds",      "DIM"     },
	{ "dsd",     "DIMDIM"  },
	{ "dsstat",  "DIMQSTN" }, 

	/* Digital filter */
	{ "df",      "DGF" },

	/* Content */
	{ "artbmp",  "NJABMP"         },
	{ "arturl",  "NJALINK;NJAREQ" },
	{ "saveart", "NJABMP;NJAREQ"  },
	{ "album",   "NALQSTN"        },
	{ "artist",  "NATQSTN"        },
	{ "title",   "NTIQSTN"        },
	{ "tracks",  "NTRQSTN"        },
		
	/* Audio muting */
	{ "mute",    "AMT01"   },
	{ "unmute",  "AMT00"   },
	{ "mutestat","AMTQSTN" },

	/* Network standby - if off then streamer switches completely off 
	   when standby pressed on remote */
	{ "sbon",    "NSBON"   },  
	{ "sboff",   "NSBOFF"  },
	{ "sbstat" , "NSBQSTN" },

	/* Upsampling (called music optimisation in the docs) */
	{ "upon",    "UPS03"   },
	{ "upoff",   "UPS00"   },
	{ "upstat",  "UPSQSTN" },

	/* Hi bit */
	{ "hbon",    "HBT01" },
	{ "hboff",   "HBT00" },

	/* Direct on/off */
	{ "diron",   "DIR01"   },
	{ "diroff",  "DIR00"   },
	{ "dirstat", "DIRQSTN" },

	/* Power on/off */
	{ "pwron",   "PWR01"   },
	{ "pwroff",  "PWR00"   },
	{ "pwrstat", "PWRQSTN" },

	/* Auto power down */
	{ "apdon",   "APD01"   },
	{ "apdoff",  "APD00"   },
	{ "apdstat", "APDQSTN" },

	/* Music optimisation - ASR on remote */
	{ "asron",   "MOT01"   },
	{ "asroff",  "MOT00"   },
	{ "asrstat", "MOTQSTN" },

	/* Network services */
	{ "msv",     "SLI27"   },
	{ "net",     "SLI2B"   },
	{ "dts",     "NSV420"  },
	{ "tidal",   "NSV1B0"  },
	{ "playq",   "NSV1D0"  },
	{ "flare",   "NSV430"  },
	{ "tunein",  "NSV0E0"  },
	{ "deezer",  "NSV120"  },
	{ "chrome",  "NSV400"  },
	{ "spotify", "NSV0A0"  },
	{ "airplay", "NSV180"  },
	{ "netstat", "NSVQSTN" },
	{ "mrmstat", "MRMQSTN" },

	/* Misc */
	{ "cnstat",  "NDSQSTN" },
	{ "top",     "NTCTOP"  },
	{ "dev",     "NDNQSTN" },
	{ "mem",     "EDFQSTN" },
	{ "scr",     "NLTQSTN" },
	{ "pps",     "PPSQSTN" },
	{ "fwver",   "FWVQSTN" },
	{ "sysinfo", "MDIQSTN" },
	{ "auinfo",  "IFAQSTN" },
	{ "stop",    "NTCSTOP" },
	{ "name",    "NFNQSTN" },
	{ "setname", "NFN"     },  /* NFN only here for help print out */
	{ "setup",   "NRIQSTN" },
	{ "ustat",   "UPDQSTN" },
	{ "codec",   "NFIQSTN" },
	{ "pstat",   "NSTQSTN" },
	{ "reset",   "RSTALL"  },
	{ "mgver",   "MGVQSTN" },

	/* Hardware input sources */
	{ "usbf",    "SLI29"   },
	{ "usbr",    "SLI2A"   },
	{ "usbdac",  "SLI2F"   },
	{ "dig1",    "SLI45"   },
	{ "dig2",    "SLI44"   },
	{ "inpup",   "SLIUP"   },
	{ "inpdn",   "SLIDOWN" },
	{ "inpstat", "SLIQSTN" },

	/* LRA - Lock range adjust which is to do with the accuracy of the
	   decoding clock */
	{ "lra",     "LRA"     },
	{ "lraup",   "LRAUP"   },
	{ "lradn",   "LRADOWN" },
	{ "lrastat", "LRAQSTN" }
};

#define NUM_COMMANDS (int)(sizeof(commands) / sizeof(struct st_command))

char *sorted_commands[NUM_COMMANDS];

int  getComNum(char *word, int len);
int  sendCommand(int repeat_cnt, u_char *cmd, int cmd_len);
int  copyHistoryBuffer(int buffnum);
int  processClientCommand(
	int comnum, int cmd_word, int word_cnt, u_char *word[MAX_WORDS]);
int  setPrompt(u_char *param);
void showHelp(int conmum, u_char *pat);
void showSortedHelp(u_char *pat);
void doConnect(u_char *param);
void doDisconnect();
void printHistory();
void clearHistory();


/*** addr1 and addr2 contain the array element memory location, not the
     pointer values stored at the location, so they have to be deref'd ***/
int compareComs(const void *addr1, const void *addr2)
{
	return strcmp(*(char **)addr1,*(char **)addr2);
}




void sortCommands()
{
	int i;

	for(i=0;i < NUM_COMMANDS;++i)
		sorted_commands[i] = commands[i].com;
	qsort(sorted_commands,NUM_COMMANDS,sizeof(char *),compareComs);
}




/*** Parse the command which can either be a built in command or a raw server
     command, eg NJAREQ  ***/
int parseCommand(u_char *buff, int bufflen)
{
	u_char *word[MAX_WORDS];
	u_char *end;
	u_char *s;
	u_char c;
	u_char q_char;
	char *separator;
	char *cmd;
	int word_len[MAX_WORDS];
	int word_cnt;
	int cmd_word;
	int cmd_len;
	int comnum;
	int repeat_cnt;
	int in_quotes;
	int inc;
	int ret;
	int val;
	int i;

	if (!bufflen) return CMD_MISSING;
	end = buff + bufflen;

	/* Make sure we have something other than whitespace */
	for(s=buff;s < end;++s) if (!isspace(*s)) break;
	if (s == buff+bufflen) return CMD_MISSING;

	/* See if we have !<num> which means put that history buffer into
	   current keyboard buffer */
	if (buff[0] == '!')
	{
		for(s=buff+1;s < end && isdigit(*s);++s);
		if (s < end)
		{
			puts("ERROR: '!' requires a number.");
			return CMD_MISSING;
		}
		if (!copyHistoryBuffer(atoi((char *)(buff + 1))))
		{
			puts("ERROR: Invalid or empty history buffer.");
			return CMD_MISSING;
		}
		putchar('\n');
		bufflen = buffer[keyb_buffnum].len;
		end = buff + bufflen;
	}

	/* Get the words in the buffer. If a raw command can use spaces, eg NFN
	   then quotes need to be used */
	bzero(word,sizeof(word));
	bzero(word_len,sizeof(word_len));
	word_cnt = 0;
	in_quotes = 0;
	q_char = 0;

	/* Parse buffer */
	for(s=buff,i=inc=0;s < end && i < MAX_WORDS;++s)
	{
		c = *s;
		if (c == '"' || c == '\'')
		{
			if (in_quotes)
			{
				if (c == q_char)
				{
					in_quotes = 0;
					q_char = 0;
					continue;
				}
			}
			else
			{
				in_quotes = 1;
				q_char = c;
				continue;
			}
		}
		if (!in_quotes && isspace(c))
		{
			i += inc;
			inc = 0;
			continue;
		}

		/* Just add 1 char at a time. Could allocate memory in blocks
		   but overkill for this as efficiency is not important */
		++word_len[i];
		word[i] = (u_char *)realloc(word[i],word_len[i]+1);
		word[i][word_len[i]-1] = c;
		word[i][word_len[i]] = '\0';
		inc = 1;
	}
	word_cnt = i + (i < MAX_WORDS && word[i]);
	repeat_cnt = 1;
	cmd_word = 0;
	ret = CMD_OK;

	/* See if we have a number as the first word */
	for(s=word[0];*s && isdigit(*s);++s);
	if (!*s)
	{
		if (!(repeat_cnt = atoi((char *)word[0]))) 
		{
			puts("ERROR: Repeat count must be > 0.");
			ret = CMD_ERROR;
			goto FREE;
		}
		if (!word_len[1])
		{
			puts("ERROR: Missing command.");
			ret = CMD_ERROR;
			goto FREE;
		}
		cmd_word = 1;
	}

	/* Treat uppercase as a raw streamer command */
	if (word[cmd_word][0] >= 'A' && word[cmd_word][0] <= 'Z')
	{
		if (!tcp_sock)
		{
			puts("ERROR: Offline - cannot send command.");
			ret = CMD_ERROR;
			goto FREE;
		}
		if (word_len[cmd_word] < 3)
		{
			puts("ERROR: Raw streamer commands need a minimum of 3 letters. eg: NTC");
			ret = CMD_ERROR;
			goto FREE;
		}
		/* Clear value so its seen as a new value on RX so will get
		   printed out */
		clearValueOfKey((char *)word[cmd_word]);

		if (sendCommand(repeat_cnt,word[cmd_word],word_len[cmd_word]))
			ret = CMD_OK;
		else
			ret = CMD_ERROR;
		goto FREE;
	}

	if ((comnum = getComNum(
		(char *)word[cmd_word],word_len[cmd_word])) == -1)
	{
		ret = CMD_ERROR;
		goto FREE;
	}

	/* If no data then its a built-in command */
	if (!commands[comnum].data)
	{
		for(i=0;i < repeat_cnt;++i)
		{
			if (processClientCommand(
				comnum,cmd_word,word_cnt,word) != CMD_OK)
			{
				ret = CMD_ERROR;
				break;
			}
		}
		goto FREE;
	}
	if (!tcp_sock)
	{
		puts("ERROR: Offline - cannot send command.");
		ret = CMD_ERROR;
		goto FREE;
	}

	/* Some commands take arguments */
	switch(comnum)
	{
	case COM_UP:
		SETFLAG(FLAG_COM_UP);
		break;

	case COM_DN:
		SETFLAG(FLAG_COM_DN);
		break;

	case COM_EX:
		clearMenu();
		menu_cursor_pos = -1;
		break;

	case COM_EN:
		setMenuSelection();
		menu_cursor_pos = -1;
		break;

	case COM_SAVEART:
		if (save_stage != SAVE_INACTIVE)
		{
			/* Should never happen, just in case */
			puts("WARNING: Currently saving - resetting.");
		}
		/* Ignore repeat_cnt */
		startSave(word_cnt > cmd_word ? 
		          (char *)word[cmd_word+1] : NULL);
		break;

	case COM_SETNAME:
		/* Setname requires an argument */
		if (word_cnt < cmd_word + 2)
		{
			puts("Usage: setname <name>");
			ret = CMD_ERROR;
			goto FREE;
		}

		clearValueOfKey("NFN");
		cmd_len = asprintf(&cmd,"NFN%s",word[cmd_word+1]);
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
			ret = CMD_ERROR;
		free(cmd);
		goto FREE;

	case COM_DS:
	case COM_DF:
	case COM_LRA:
		/* Commands that take a numeric argument */
		if (word_cnt < cmd_word + 2 || 
		    !isNumber(word[cmd_word+1],word_len[cmd_word+1]) || 
		    (val = atoi((char *)word[cmd_word+1])) > 99)
		{
			printf("Usage: %s <number>\n",commands[comnum].com);
			ret = CMD_ERROR;
			goto FREE;
		}
		clearValueOfKey(commands[comnum].data);
		cmd_len = asprintf(&cmd,"%s%02d",commands[comnum].data,val);
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
			ret = CMD_ERROR;
		free(cmd);
		goto FREE;
	}

	/* Send command(s) to the streamer */
	for(cmd=commands[comnum].data;;cmd=separator+1)
	{
		/* Separator in .data is for consistency the same as for 
		   commands entered by the user which is a semi colon */
		if ((separator = strchr(cmd,CMD_SEPARATOR)))
			cmd_len = (int)(separator - cmd);
		else
			cmd_len = strlen(cmd);
		
		/* Reset list entry. Will only use the first 3 chars */
		clearValueOfKey(cmd);

		/* Send command the given count times */
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
		{
			ret = CMD_ERROR;
			break;
		}
		if (!separator) break;
	}

	FREE:
	for(i=0;i <= cmd_word;++i) free(word[i]);
	if (comnum != COM_UP) UNSETFLAG(FLAG_COM_UP);
	if (comnum != COM_DN) UNSETFLAG(FLAG_COM_DN);
	return ret;
}




/*** Look for a client command. Try exact match first and if that fails look 
     for partial match ***/
int getComNum(char *word, int len)
{
	int comnum;

	for(comnum=0;
	    comnum < NUM_COMMANDS && strcmp(word,commands[comnum].com);
	    ++comnum);

	if (comnum < NUM_COMMANDS) return comnum;

	for(comnum=0;
	    comnum < NUM_COMMANDS && strncmp(word,commands[comnum].com,len);
	    ++comnum);

	if (comnum < NUM_COMMANDS)
	{
		printf("Expanded to: \"%s\"\n",commands[comnum].com);
		return comnum;
	}

	printf("ERROR: Unknown command \"%s\". Type '?' or 'help' for a list.\n",word);
	return -1;
}




int sendCommand(int repeat_cnt, u_char *cmd, int cmd_len)
{
	int i;
	for(i=0;i < repeat_cnt;++i) if (!writeSocket(cmd,cmd_len)) return 0;
	return 1;
}




/*** Copy the given buffer to the keyboard buffer ***/
int copyHistoryBuffer(int buffnum)
{
	int bn = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
	int num = 0;
	int cnt;

	for(cnt=0;cnt < MAX_HIST_BUFFERS-1;++cnt)
	{
		if (buffer[bn].len)
		{
			if (++num == buffnum)
			{
				clearPrompt();
				copyBuffer(bn,keyb_buffnum);
				printPrompt();
				return 1;
			}
		}
		bn = (bn + 1) % MAX_HIST_BUFFERS;
	}
	return 0;
}




/*** A command not sent to the server ***/
int processClientCommand(
	int comnum, int cmd_word, int word_cnt, u_char *word[MAX_WORDS])
{
	u_char *param;
	char *end;
	int max = 0;
	
	if (word_cnt > cmd_word)
	{
		param = word[cmd_word+1];
		if (word_cnt > cmd_word + 2)
		{
			end = NULL;
			max = strtol((char *)word[cmd_word+2],&end,10);
			if (end && *end) max = -1;
		}
	}
	else param = NULL;

	switch(comnum)
	{
	case COM_EXIT:
		puts("*** EXIT by command ***");
		doExit(0);
	case COM_VER:
		version(1);
		break;
	case COM_DET:
		FLIPFLAG(FLAG_SHOW_DETAIL);
		printf("Show detail: %s\n",ONOFF(FLAG_SHOW_DETAIL));
		break;
	case COM_STMR:
		FLIPFLAG(FLAG_SHOW_TIMER);
		printf("Show service timer: %s\n",ONOFF(FLAG_SHOW_TIMER));
		break;
	case COM_PROMPT:
		return setPrompt(param);
	case COM_RAW:
		FLIPFLAG(FLAG_SHOW_RAW);
		printf("Show raw RX: %s\n",ONOFF(FLAG_SHOW_RAW));
		break;
	case COM_TAMP:
		FLIPFLAG(FLAG_TRANS_HTML_AMPS);
		printf("Translate HTML amp codes: %s\n",
			ONOFF(FLAG_TRANS_HTML_AMPS));
		break;
	case COM_SHOW:
		nja_prev = 0;
		return prettyPrintList(param,max);
	case COM_SHOWR:
		return dumpList(param,max);
	case COM_CLEAR:
		clearList();
		clearTitles();
		break;
	case COM_QMARK:
	case COM_HELP:
	case COM_XHELP:
		showHelp(comnum,param);
		break;
	case COM_SHELP:
		showSortedHelp(param);
		break;
	case COM_CON:
		doConnect(param);
		break;
	case COM_DISCON:
		doDisconnect();
		break;
	case COM_HIST:
		printHistory();
		break;
	case COM_CHIST:
		clearHistory();
		break;
	case COM_TITLES:
		return printTitles(0,param,max);
	case COM_XTITLES:
		return printTitles(1,param,max);
	case COM_WAIT:
		/* Meant for use in command line -i option */
		sleep(1);
		break;
	case COM_CLS:
		write(STDOUT,"\033[2J\033[H",7);
		break;
	case COM_RXCOMS:
		printRXCommands(param);
		break;
	case COM_TIME:
		printTimes();
		break;
	case COM_LIST:
		printMenu();
		break;
	case COM_WHERE:
		printMenuInfo();
		break;
	default:
		assert(0);
	}
	return CMD_OK;
}




int setPrompt(u_char *param)
{
	int val;

	if (param)
	{
		val = atoi((char *)param);
		if (val >= 0 && val < NUM_PROMPTS)
		{
			prompt_type = val;
			return CMD_OK;
		}
	}
	puts("Usage: prompt [0/1/2/3]");
	return CMD_ERROR;
}




void showHelp(int comnum, u_char *pat)
{
	int nlafter;
	int cnt;
	int i;

	nlafter = 5;

	puts("\n*** Client commands ***\n");
	for(i=cnt=0;i < NUM_COMMANDS;++i)
	{
		if (i && commands[i].data && !commands[i-1].data)
		{
			if (cnt % nlafter) putchar('\n');
			puts("\n*** Streamer commands ***\n");
			if (comnum == COM_XHELP) nlafter = 4;
			cnt = 0;
		}
		if (!pat || wildMatch(commands[i].com,(char *)pat))
		{
			if (comnum == COM_XHELP)
			{
				if (i > COM_RXCOMS)
				{
					printf("   %-10s (%.3s)",
						commands[i].com,
						commands[i].data);
				}
				else printf("   %-10s",commands[i].com);
			}
			else printf("   %-10s",commands[i].com);
			if (!(++cnt % nlafter)) putchar('\n');
		}
	}
	if (cnt % nlafter) putchar('\n');
	if (comnum != COM_XHELP)
	{
		puts("\nType 'xhelp' for more help information or 'shelp' for a sorted list.\n");
		return;
	}
	puts("\nUsage: [<repeat count>] <client/streamer command>   eg: 3 up");
	puts("                                                        con 192.168.0.1");
	puts("       [<repeat count>] <raw streamer command>      eg: 3 OSDUP\n");

	puts("Notes:");
	puts("1) Any commands starting with a capital letter are passed to the streamer");
	puts("   unchanged as raw commands. eg: OSDUP\n");
	puts("2) Built in and raw commands can be chained with semicolon separators.");
	puts("   eg: tunein; 3 OSDDOWN; wait; en\n");
	puts("   commands take an optional wildcard pattern to limit the number of returned");
	puts("   results. Eg: help *stat\n");
	puts("4) The 'titles', 'xtitles', 'show' and 'showr' commands can take a 2nd");
	puts("   parameter which is the number of entries to print from the start of the");
	puts("   list. Eg: 'titles * 5' to show the first 5 titles.\n");
	puts("5) Either double or single quotes can be used to pass whitespace as data.");
	puts("   eg: setname \"Neil's streamer\"");
	puts("       setname 'Pass double\"quote'\n");
	puts("6) Commands can be shortened to any matching substring depending on the");
	puts("   internal order of the commands. Eg: 'xtitles' can be shortened to 'xt'");
	puts("   and 'prompt' to just 'p'.\n");
}




void showSortedHelp(u_char *pat)
{
	int cnt;
	int i;

	puts("\n*** Sorted commands ***\n");
	for(i=cnt=0;i < NUM_COMMANDS;++i)
	{
		if (!pat || wildMatch(sorted_commands[i],(char *)pat))
		{
			printf("   %-10s",sorted_commands[i]);
			if (!(++cnt % 5)) putchar('\n');
		}
	}
	if (cnt % 5) putchar('\n');
	putchar('\n');
}




void doConnect(u_char *param)
{
	if (tcp_sock)
	{
		puts("ERROR: Already connected.");
		return;
	}
	/* Could have:       con [address]
	               <cnt> con [address]  - pointless to do but... */
	if (param) ipaddr = strdup((char *)param);
	if (!networkStart()) networkClear();
}




void doDisconnect()
{
	if (tcp_sock)
		networkClear();
	else
		puts("ERROR: Not connected.");
}




/*** Prints the command history ***/
void printHistory()
{
	int bn = (keyb_buffnum + 2) % MAX_HIST_BUFFERS;
	int num = 0;
	int cnt;

	for(cnt=0;cnt < MAX_HIST_BUFFERS-1;++cnt)
	{
		if (buffer[bn].len) printf("%3d  %s\n",++num,buffer[bn].data);
		bn = (bn + 1) % MAX_HIST_BUFFERS;
	}
	puts("Enter !<number> or up/down arrow keys to select.");
}




void clearHistory()
{
	int i;
	for(i=0;i < MAX_HIST_BUFFERS;++i) clearBuffer(i);
	puts("History cleared.");
}
