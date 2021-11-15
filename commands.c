#include "globals.h"

#define ONOFF(F)      (flags & F) ? "ON" : "OFF"
#define OFFLINE_ERROR "ERROR: Offline, command cannot be sent."

/* Built in commands apart from SAVEART */
enum
{
	/* 0. Client commands */
	COM_EXIT,
	COM_VER,
	COM_SVCTM,
	COM_PROMPT,
	COM_RAW,

	/* 5 */
	COM_TAMP,
	COM_SHOW,
	COM_SHOWR,
	COM_CLEAR,
	COM_QMARK,

	/* 10 */
	COM_HELP,
	COM_XHELP,
	COM_SHELP,
	COM_CON,
	COM_DISCON,

	/* 15 */
	COM_CONSTAT,
	COM_HIST,
	COM_CHIST,
	COM_TITLES,
	COM_XTITLES,

	/* 20 */
	COM_WAIT,
	COM_CLS,
	COM_RXCOMS,
	COM_TXCOMS,
	COM_TIME,

	/* 25 */
	COM_LIST,
	COM_SELECTED,
	COM_ECHO,
	COM_MADEF,
	COM_MAAPP,

	/* 30 */
	COM_MADEL,
	COM_MACLEAR,
	COM_MARUN,
	COM_MAVRUN,
	COM_MALIST,

	/* 35 */
	COM_MALOAD,
	COM_MASAVA,
	COM_MASAVC,

	/* 38. Streamer commands */
	COM_MENU,
	COM_UP,

	/* 40 */
	COM_DN,
	COM_EN,
	COM_EX,
	COM_MSTAT,
	COM_FLIP,

	/* 45 */
	COM_DS,
	COM_DSD,
	COM_DSSTAT,
	COM_DF,
	COM_ARTBMP,

	/* 50 */
	COM_ARTURL,
	COM_SAVEART,

	/* Enums beyond saveart not required except for these */
	COM_SETNAME = 103,
	COM_LRA     = 118
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
	{ "svctm",  NULL },
	{ "prompt", NULL },
	{ "raw",    NULL },

	/* 5 */
	{ "tamp",   NULL },
	{ "show",   NULL },
	{ "showr",  NULL },
	{ "clear",  NULL },
	{ "?",      NULL },

	/* 10 */
	{ "help",   NULL },
	{ "xhelp",  NULL },
	{ "shelp",  NULL },
	{ "con",    NULL },
	{ "discon", NULL },

	/* 15 */
	{ "constat",NULL },
	{ "hist",   NULL },
	{ "chist",  NULL },
	{ "titles", NULL },
	{ "xtitles",NULL },

	/* 20 */
	{ "wait",   NULL },
	{ "cls",    NULL },
	{ "rxcoms", NULL },
	{ "txcoms", NULL },
	{ "time",   NULL },

	/* 25 */
	{ "list",   NULL },
	{ "selected",NULL },
	{ "echo",    NULL },
	{ "madef",   NULL },
	{ "maapp",   NULL },

	/* 30 */
	{ "madel",   NULL },
	{ "maclear",NULL },
	{ "marun",  NULL },
	{ "mavrun", NULL },
	{ "malist", NULL },

	/* 35 */
	{ "maload", NULL },
	{ "masava", NULL },
	{ "masavc", NULL },

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
	{ "svcstat", "NSVQSTN" },
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

int  parseCommand(u_char *buff, int bufflen);
int  sendCommand(int repeat_cnt, u_char *cmd, int cmd_len);
int  copyHistoryBuffer(int buffnum);
int  processBuiltInCommand(
	int comnum, int cmd_word, int word_cnt, u_char **words);
int  setPrompt(u_char *param);
int  setRaw(u_char *param);
void showHelp(int conmum, u_char *pat);
void showSortedHelp(u_char *pat);
int  doConnect(u_char *param);
int  doDisconnect();
void printHistory();
void clearHistory();
int  doWait(u_char *param);
void printTXCommands(u_char *pat);
void printConstat();
void printTimes();
void doEcho(int cmd_word, int word_cnt, u_char **words);
int  defineMacro(int cmd_word, int word_cnt, u_char **words);
int  doAppendMacro(int cmd_word, int word_cnt, u_char **words);
int  doDeleteMacro(u_char *name);
int  doRunMacro(int comnum, u_char *name);
int  doLoadMacros(u_char *filename);
int  doSaveMacro(int comnum, int cmd_word, int word_cnt, u_char **words);

int   compareComs(const void *addr1, const void *addr2);
u_int getUsecTime();


void sortCommands()
{
	int i;

	for(i=0;i < NUM_COMMANDS;++i)
		sorted_commands[i] = commands[i].com;
	qsort(sorted_commands,NUM_COMMANDS,sizeof(char *),compareComs);
}




/*** Parse a line from the user or a macro ***/
int parseInputLine(u_char *data, int len)
{
	u_char *separator;
	u_char *ptr;
	u_char *ptr2;
	u_char *end;
	u_char q_char;
	u_char c;
	int in_quotes;
	int ret;

	if (input_state != INPUT_CMD)
	{
		addMacroLine(data,len);
		return OK;
	}
	end = data + len;
	q_char = 0;
	
	for(ptr=data;ptr < end;ptr=separator+1)
	{
		in_quotes = 0;

		/* Find the seperator (but ignore if inside double quotes) */
		for(separator=ptr;*separator;++separator)
		{
			c = *separator;
			if (c == '"' || c == '\'')
			{
				if (in_quotes)
				{
					if (c == q_char)
					{
						in_quotes = 0;
						q_char = 0;
					}
				}
				else 
				{
					in_quotes = 1;
					q_char = c;
				}
			}
			else if (c == SEPARATOR && !in_quotes) break;
		}
		if (in_quotes)
		{
			puts("ERROR: Unterminated quotes.");
			if (!FLAGISSET(FLAG_MACRO_RUNNING))
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			break;
		}
	
		if (*separator)
			len = (int)(separator - ptr);
		else
			len = (int)(end - ptr);

		if (len)
		{
			if (FLAGISSET(FLAG_MACRO_RUNNING))
			{
				/* Skip initial whitespace */
				for(ptr2=ptr;*ptr2 < 33;++ptr2);
				if (FLAGISSET(FLAG_MACRO_VERBOSE))
				{
					printf("Exec cmd: %.*s\n",
						len - (int)(ptr2-ptr),ptr2);
				}
			}

			if ((ret = parseCommand(ptr,len)) != ERR_CMD_MISSING)
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			if (ret != OK) return ret;
			if (FLAGISSET(FLAG_INTERRUPTED)) return ERR_CMD_FAIL;
			if (!separator) break;
		}
	}
	return OK;
}




/*** Parse the command which can either be a built in command or a raw server
     command, eg NJAREQ  ***/
int parseCommand(u_char *buff, int bufflen)
{
	u_char *words[MAX_WORDS];
	u_char *comword;
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

	if (!bufflen) return ERR_CMD_MISSING;
	end = buff + bufflen;

	/* Make sure we have something other than whitespace */
	for(s=buff;s < end;++s) if (!isspace(*s)) break;
	if (s == buff+bufflen) return ERR_CMD_MISSING;

	/* See if we have !<num> which means put that history buffer into
	   current keyboard buffer */
	if (buff[0] == '!')
	{
		for(s=buff+1;s < end && isdigit(*s);++s);
		if (s < end)
		{
			puts("ERROR: '!' requires a number.");
			return ERR_CMD_MISSING;
		}
		if (!copyHistoryBuffer(atoi((char *)(buff + 1))))
		{
			puts("ERROR: Invalid or empty history buffer.");
			return ERR_CMD_MISSING;
		}
		putchar('\n');
		bufflen = buffer[keyb_buffnum].len;
		end = buff + bufflen;
	}

	/* Get the words in the buffer. If a raw command can use spaces, eg NFN
	   then quotes need to be used */
	bzero(words,sizeof(words));
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
		words[i] = (u_char *)realloc(words[i],word_len[i]+1);
		words[i][word_len[i]-1] = c;
		words[i][word_len[i]] = '\0';
		inc = 1;
	}
	word_cnt = i + (i < MAX_WORDS && words[i]);
	repeat_cnt = 1;
	cmd_word = 0;
	ret = OK;

	/* See if we have a number as the first word */
	for(s=words[0];*s && isdigit(*s);++s);
	if (!*s)
	{
		if (!(repeat_cnt = atoi((char *)words[0]))) 
		{
			puts("ERROR: Repeat count must be > 0.");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (!word_len[1])
		{
			puts("ERROR: Missing command.");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		cmd_word = 1;
	}

	comword = words[cmd_word];

	/* Treat uppercase as a raw streamer command */
	if (comword[0] >= 'A' && comword[0] <= 'Z')
	{
		if (!tcp_sock)
		{
			puts(OFFLINE_ERROR);
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (word_len[cmd_word] < 3)
		{
			puts("ERROR: Raw streamer commands need a minimum of 3 letters. eg: NTC");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		/* Clear value so its seen as a new value on RX so will get
		   printed out */
		clearValueOfKey((char *)comword);

		if (sendCommand(repeat_cnt,comword,word_len[cmd_word]))
			ret = OK;
		else
			ret = ERR_CMD_FAIL;
		goto FREE;
	}

	/* Look for a command. If we can't find one try a macro */
	if ((comnum = getCommand((char *)comword,word_len[cmd_word],1)) == -1)
	{
		if (findMacro(comword) != -1)
		{
			ret = doRunMacro(comnum,comword);
			goto FREE;
		}
		printf("ERROR: Unknown command or macro \"%s\". Type '?' or 'help' for a list of\n",comword);
		puts("       commands or 'malist' for a list of macros.");
		ret = ERR_CMD_FAIL;
		goto FREE;
	}

	/* If no data then its a built-in command */
	if (!commands[comnum].data)
	{
		for(i=0;i < repeat_cnt;++i)
		{
			if (processBuiltInCommand(
				comnum,cmd_word,word_cnt,words) != OK)
			{
				ret = ERR_CMD_FAIL;
				break;
			}
		}
		goto FREE;
	}
	if (!tcp_sock)
	{
		puts(OFFLINE_ERROR);
		ret = ERR_CMD_FAIL;
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
		          (char *)words[cmd_word+1] : NULL);
		break;

	case COM_SETNAME:
		/* Setname requires an argument */
		if (word_cnt < cmd_word + 2)
		{
			puts("Usage: setname <name>");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}

		clearValueOfKey("NFN");
		cmd_len = asprintf(&cmd,"NFN%s",words[cmd_word+1]);
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
			ret = ERR_CMD_FAIL;
		free(cmd);
		goto FREE;

	case COM_DS:
	case COM_DF:
	case COM_LRA:
		/* Commands that take a numeric argument */
		if (word_cnt < cmd_word + 2 || 
		    !isNumber((char *)words[cmd_word+1]) || 
		    (val = atoi((char *)words[cmd_word+1])) > 99)
		{
			printf("Usage: %s <number>\n",commands[comnum].com);
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		clearValueOfKey(commands[comnum].data);
		cmd_len = asprintf(&cmd,"%s%02d",commands[comnum].data,val);
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
			ret = ERR_CMD_FAIL;
		free(cmd);
		goto FREE;
	}

	/* Send command(s) to the streamer */
	for(cmd=commands[comnum].data;;cmd=separator+1)
	{
		/* Separator in .data is for consistency the same as for 
		   commands entered by the user which is a semi colon */
		if ((separator = strchr(cmd,SEPARATOR)))
			cmd_len = (int)(separator - cmd);
		else
			cmd_len = strlen(cmd);
		
		/* Reset list entry. Will only use the first 3 chars */
		clearValueOfKey(cmd);

		/* Send command the given count times */
		if (!sendCommand(repeat_cnt,(u_char *)cmd,cmd_len))
		{
			ret = ERR_CMD_FAIL;
			break;
		}
		if (!separator) break;
	}

	FREE:
	for(i=0;i <= cmd_word;++i) free(words[i]);
	if (comnum != COM_UP) UNSETFLAG(FLAG_COM_UP);
	if (comnum != COM_DN) UNSETFLAG(FLAG_COM_DN);
	return ret;
}




/*** Look for a client command. Try exact match first and if that fails look 
     for partial match ***/
int getCommand(char *word, int len, int expmsg)
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
		if (expmsg)
			printf("Expanded to: \"%s\"\n",commands[comnum].com);
		return comnum;
	}
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
int processBuiltInCommand(
	int comnum, int cmd_word, int word_cnt, u_char **words)
{
	u_char *param;
	char *end;
	int max = 0;
	
	if (word_cnt > cmd_word)
	{
		param = words[cmd_word+1];
		if (word_cnt > cmd_word + 2)
		{
			end = NULL;
			max = strtol((char *)words[cmd_word+2],&end,10);
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
	case COM_SVCTM:
		FLIPFLAG(FLAG_SHOW_SVC_TIME);
		printf("Show service time: %s\n",ONOFF(FLAG_SHOW_SVC_TIME));
		break;
	case COM_PROMPT:
		return setPrompt(param);
	case COM_RAW:
		return setRaw(param);
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
		clearHistory();
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
		return doConnect(param);
	case COM_DISCON:
		return doDisconnect();
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
		return doWait(param);
	case COM_CLS:
		write(STDOUT,"\033[2J\033[H",7);
		break;
	case COM_RXCOMS:
		printRXCommands(param);
		break;
	case COM_TXCOMS:
		printTXCommands(param);
		break;
	case COM_CONSTAT:
		printConstat();
		break;
	case COM_TIME:
		printTimes();
		break;
	case COM_LIST:
		printMenuList();
		break;
	case COM_SELECTED:
		printMenuSelection();
		break;
	case COM_ECHO:
		doEcho(cmd_word,word_cnt,words);
		break;
	case COM_MADEF:
		return defineMacro(cmd_word,word_cnt,words);
	case COM_MADEL:
		return doDeleteMacro(param);
	case COM_MAAPP:
		return doAppendMacro(cmd_word,word_cnt,words);
	case COM_MACLEAR:
		return clearMacros();
	case COM_MARUN:
	case COM_MAVRUN:
		return doRunMacro(comnum,param);
	case COM_MALIST:
		listMacros();
		break;
	case COM_MALOAD:
		return doLoadMacros(param);
	case COM_MASAVA:
	case COM_MASAVC:
		return doSaveMacro(comnum,cmd_word,word_cnt,words);
	default:
		assert(0);
	}
	return OK;
}


/****************************** BUILT IN COMMANDS *****************************/

int setPrompt(u_char *param)
{
	int val;

	if (param)
	{
		val = atoi((char *)param);
		if (isNumber((char *)param) && val >= 0 && val < NUM_PROMPTS)
		{
			prompt_type = val;
			return OK;
		}
	}
	printf("Usage: prompt [0 to %d]\n",NUM_PROMPTS-1);
	return ERR_CMD_FAIL;
}




int setRaw(u_char *param)
{
	int val;

	if (param)
	{
		val = atoi((char *)param);
		if (isNumber((char *)param) && val >= 0 && val < NUM_RAW_LEVELS)
		{
			raw_level = val;
			puts("OK");
			return OK;
		}
	}
	printf("Usage: raw [0 to %d]\n",NUM_RAW_LEVELS-1);
	return ERR_CMD_FAIL;
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




int doConnect(u_char *param)
{
	if (tcp_sock)
	{
		puts("ERROR: Already connected.");
		return ERR_CMD_FAIL;
	}
	/* Could have:       con [address]
	               <cnt> con [address]  - pointless to do but... */
	if (param) ipaddr = strdup((char *)param);
	if (!networkStart()) networkClear();
	return OK;
}




int doDisconnect()
{
	if (tcp_sock)
	{
		networkClear();
		return OK;
	}
	puts("ERROR: Not connected.");
	return ERR_CMD_FAIL;
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




int doWait(u_char *param)
{
	struct timeval tv;
	fd_set mask;
	float secs;
	u_int usecs;
	u_int end;

	if (param && (secs = atof((char *)param)) > 0)
	{
		usecs = (u_int)(secs * 1000000);
		if (tcp_sock)
		{
			/* Print out anything coming from the streamer while 
			   we wait */
			end = getUsecTime() + usecs;

			while(1)
			{
				FD_ZERO(&mask);
				FD_SET(tcp_sock,&mask);
				usecs = end - getUsecTime();
				tv.tv_sec = usecs / 1000000;
				tv.tv_usec = usecs % 1000000;
				switch(select(FD_SETSIZE,&mask,0,0,&tv))
				{
				case -1:
					return (errno == EINTR) ? ERR_CMD_FAIL : OK;
				case 0:
					return OK;
				}
				readSocket(0);
			}
		}
		else if (usleep(usecs) == -1 && errno == EINTR)
			return ERR_CMD_FAIL;

		return OK;
	}
	puts("Usage: wait <seconds>");
	return ERR_CMD_FAIL;
}




void printTXCommands(u_char *pat)
{
	int cnt1;
	int cnt2;
	int i;

	puts("\n*** TX streamer command mappings ***\n");
	for(i=cnt1=cnt2=0;i < NUM_COMMANDS;++i)
	{
		if (commands[i].data)
		{
			++cnt1;
			if (!pat || wildMatch(commands[i].com,(char *)pat))
			{
				if (cnt2 && !(cnt2 % 3)) putchar('\n');
				printf("%-8s = %-15s",commands[i].com,commands[i].data);
				++cnt2;
			}
		}
	}
	if (pat)
		printf("\n\n%d of %d commands.\n\n",cnt2,cnt1);
	else
		printf("\n\n%d commands.\n\n",cnt1);
}




void printConstat()
{
	t_iscp_data *pkt_data;

	puts("\n*** Connection status and traffic ***\n");
	printf("Streamer TCP: %s:%d\n",inet_ntoa(con_addr.sin_addr),tcp_port);
	printf("Connection  : %sCONNECTED\n",tcp_sock ? "" : "DIS");
	printf("Connect time: %s\n",getTimeString(connect_time));
	printf("Last RX ago : %s\n",getTimeString(last_rx_time));
	printf("Last TX ago : %s\n",getTimeString(last_tx_time));

	if (buffer[BUFF_TCP].data)
	{
		pkt_data = (t_iscp_data *)(buffer[BUFF_TCP].data + pkt_hdr->hdr_len);
		printf("Last RX com : %.3s\n",pkt_data->command);
	}
	else puts("Last RX com : ---");

	printf("RX reads    : %lu\n",rx_reads);
	printf("RX bytes    : %lu\n",rx_bytes);
	printf("TX writes   : %lu\n",tx_writes);
	printf("TX bytes    : %lu\n\n",tx_bytes);
}




void printTimes()
{
	printf("Local time  : %s\n",getTime());
	printf("Connect time: %s\n",getTimeString(connect_time));
	printf("Service time: %s\n",svc_time_str);
}




void doEcho(int cmd_word, int word_cnt, u_char **words)
{
	int i;
	for(i=cmd_word+1;i < word_cnt;++i) printf("%s ",words[i]);
	putchar('\n');
}




int defineMacro(int cmd_word, int word_cnt, u_char **words)
{
	switch(word_cnt - cmd_word)
	{
	case 2:
		return initMultiLineMacro(words[cmd_word+1]);
	case 3:
		return addMacro(words[cmd_word+1],words[cmd_word+2]);
	}
	puts("Usage: madef <macro> [\"<macro command list>\"]");
	return ERR_CMD_FAIL;
}




int doAppendMacro(int cmd_word, int word_cnt, u_char **words)
{
	switch(word_cnt - cmd_word)
	{
	case 2:
		return initMultiLineMacroAppend(words[cmd_word+1]);
	case 3:
		return appendMacro(words[cmd_word+1],words[cmd_word+2]);
	}
	puts("Usage: maapp <macro> [\"<macro command list>\"]");
	return ERR_CMD_FAIL;
}




int doDeleteMacro(u_char *name)
{
	if (name) return deleteMacro(name);
	puts("Usage: madel <macro>");
	return ERR_CMD_FAIL;
}




int doRunMacro(int comnum, u_char *name)
{
	static int recurse = 0;
	int ret;

	if (name)
	{
		/* Check recurse because want verbosity to continue even if a 
		   macro calls MARUN itself */
		if (comnum == COM_MAVRUN)
			SETFLAG(FLAG_MACRO_VERBOSE);
		else if (!recurse)
			UNSETFLAG(FLAG_MACRO_VERBOSE);
		++recurse;
		ret = runMacro(name);
		--recurse;
		return ret;
	}
	puts("Usage: marun <macro>");
	return ERR_CMD_FAIL;
}




int doLoadMacros(u_char *filename)
{
	if (filename) return loadMacros(filename);
	puts("Usage: maload <filename>");
	return ERR_CMD_FAIL;
}




/*** Save one or save all macros, either Append or Create ***/
int doSaveMacro(int comnum, int cmd_word, int word_cnt, u_char **words)
{
	u_char *filename;
	int append;

	if (word_cnt - cmd_word >= 2)
	{
		filename = words[cmd_word+1];
		if (comnum == COM_MASAVC && !access((char *)filename,F_OK))
		{
			printf("ERROR: File \"%s\" already exists, cannot create. Use MASAVA instead to append,\n",filename);
			puts("       or delete the file first.");
			return ERR_CMD_FAIL;
		}

		append = (comnum == COM_MASAVA);
		switch(word_cnt - cmd_word)
		{
		case 2:
			return saveAllMacros(filename,append);
		case 3:
			return saveMacro(filename,words[cmd_word+2],append);
		}
	}
	puts("Usage: masava/masavc <filename> [<macro>]");
	return ERR_CMD_FAIL;
}


/******************************** SUPPORT ************************************/

/*** addr1 and addr2 contain the array element memory location, not the
     pointer values stored at the location, so they have to be deref'd ***/
int compareComs(const void *addr1, const void *addr2)
{
	return strcmp(*(char **)addr1,*(char **)addr2);
}




/*** Get the current time down to the microsecond. Value wraps once every
     1000 seconds. Goes from 0 -> 999999999 (1 billion - 1) ***/
u_int getUsecTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (u_int)(tv.tv_sec % 1000) * 1000000 + (u_int)tv.tv_usec;
}
