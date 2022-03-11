#include "globals.h"

#define MAX_WORDS       5
#define CMD_SEPERATOR   ';'
#define PRINT_ON_OFF(F) colprintf(F ? "~FGON\n" : "~FROFF\n");
#define OFFLINE_ERROR   "Offline, command cannot be sent.\n"

/* Built in commands apart from SAVEART */
enum
{
	/* 0. Client commands */
	COM_EXIT,
	COM_TOGGLE,
	COM_PROMPT,
	COM_RAW,
	COM_SHOW,

	/* 5 */
	COM_CLEAR,
	COM_HELP,
	COM_CONNECT,
	COM_DISCONNECT,
	COM_WAIT,

	/* 10 */
	COM_CLS,
	COM_ECHO,
	COM_MACRO,
	LAST_CLIENT_COM = COM_MACRO,

	/* 13. Streamer commands */
	COM_MENU,
	COM_MENUSTAT,

	/* 15 */
	COM_UP,
	COM_DN,
	COM_EN,
	COM_EX,
	COM_FLIP,

	/* 20 */
	COM_DS,
	COM_DSD,
	COM_DSSTAT,
	COM_FILTER,
	COM_FILSTAT,

	/* 25 */
	COM_ARTBMP,
	COM_ARTURL,
	COM_ARTSTAT,
	COM_ARTSAVE,

	/* Enums beyond saveart not required except for these */
	COM_SETNAME = 80,
	COM_LRA     = 95
};


/* Not a comprehensive list of commands but just the more useful ones */
static struct st_command
{
	char *com;
	char *data;
} commands[] =
{
	/* 0. Built in commands */
	{ "exit",  NULL },
	{ "toggle",NULL },
	{ "prompt",NULL },
	{ "raw",   NULL },
	{ "show",  NULL },

	/* 5 */
	{ "clear",     NULL },
	{ "help",      NULL },
	{ "connect",   NULL },
	{ "disconnect",NULL },
	{ "wait",  NULL },

	/* 10 */
	{ "cls",   NULL },
	{ "echo",  NULL },
	{ "macro", NULL },

	/* Menu navigation */
	{ "menu",    "NTCMENU"  },
	{ "menustat","NMSQSTN"  },
	{ "up",      "OSDUP"    }, 
	{ "dn",      "OSDDOWN"  },
	{ "en",      "OSDENTER" },
	{ "ex",      "OSDEXIT"  }, 
	{ "flip",    "NTCLIST"  },

	/* It seems the display command system is broken - you can dim the 
	   display but not make it brighter again. Can only reset it via the 
	   remote control */
	{ "ds",      "DIM"     },
	{ "dsd",     "DIMDIM"  },
	{ "dsstat",  "DIMQSTN" }, 

	/* Digital filter */
	{ "filter",  "DGF"     },
	{ "filstat", "DGFQSTN" },

	/* Content */
	{ "artbmp",  "NJABMP"         },
	{ "arturl",  "NJALINK;NJAREQ" },
	{ "artstat", "NJAQSTN"        },
	{ "artsave", "NJABMP;NJAREQ"  },
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
	{ "upson",   "UPS03"   },
	{ "upsoff",  "UPS00"   },
	{ "upsstat", "UPSQSTN" },

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
	{ "updstat", "UPDQSTN" },
	{ "codec",   "NFIQSTN" },
	{ "playstat","NSTQSTN" },
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

int  parseCommand(char *buff, int bufflen);
int  sendCommand(int repeat_cnt, char *cmd, int cmd_len);
int  copyHistoryBuffer(int buffnum);
int  processBuiltInCommand(
	int comnum, int cmd_word, int word_cnt, char **words);

int  comToggle(char *opt);
int  comPrompt(char *param);
int  comRaw(char *param);
int  comShow(char *opt, char *pat, int max);
void optShowFlags();
void optShowTXCommands(char *pat);
void optShowTimes();
void optShowConStat();
void optShowHistory();

int  comClear(char *opt);
int  comHelp(char *opt, char *pat);
void optHelpMain(int extra, char *pat);
void optHelpSorted(char *pat);
void optHelpNotes();

int  comConnect(char *param);
int  comDisconnect();
void comClearHistory();
int  comWait(char *param);
void comEcho(int cmd_word, int word_cnt, char **words);

int  comMacro(char *opt, char *name, int cmd_word, int word_cnt, char **words);
int  optMacroDefine(int cmd_word, int word_cnt, char **words);
int  optMacroAppend(int cmd_word, int word_cnt, char **words);
int  optMacroSave(int append, int cmd_word, int word_cnt, char **words);

void printFlagTrackTime();
void printFlagColour();
void printFlagHTML();
void printFlagVerb();

void  clearHistory();
char *bytesSizeStr(u_long num);
u_int getUsecTime();

/******************************** INTERFACE *********************************/

/*** Parse a line from the user or a macro ***/
int parseInputLine(char *data, int len)
{
	char *separator;
	char *ptr;
	char *ptr2;
	char *end;
	char q_char;
	char c;
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
			else if (c == CMD_SEPERATOR && !in_quotes) break;
		}
		if (in_quotes)
		{
			errprintf("Unterminated quotes.\n");
			if (!flags.macro_running)
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			break;
		}
	
		if (*separator)
			len = (int)(separator - ptr);
		else
			len = (int)(end - ptr);

		if (len)
		{
			if (flags.macro_running)
			{
				/* Skip initial whitespace */
				for(ptr2=ptr;*ptr2 < 33;++ptr2);
				if (flags.verbose)
				{
					colprintf("~FB~OLExec cmd:~RS %.*s\n",
						len - (int)(ptr2-ptr),ptr2);
				}
			}

			if ((ret = parseCommand(ptr,len)) != ERR_CMD_MISSING)
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			if (ret != OK) return ret;
			if (flags.interrupted) return ERR_CMD_FAIL;
			if (!separator) break;
		}
	}
	return OK;
}




/*** Parse the command which can either be a built in command or a raw server
     command, eg NJAREQ  ***/
int parseCommand(char *buff, int bufflen)
{
	char *words[MAX_WORDS];
	char *comword;
	char *end;
	char *s;
	char c;
	char q_char;
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
			errprintf("'!' requires a number.\n");
			return ERR_CMD_MISSING;
		}
		if (!copyHistoryBuffer(atoi((buff + 1))))
		{
			errprintf("Invalid or empty history buffer.\n");
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
	for(s=buff,i=inc=0;s < end;++s)
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
		if (i == MAX_WORDS)
		{
			errprintf("Too many words or strings. Maximum is %d.\n",MAX_WORDS);
			return ERR_CMD_FAIL;
		}

		/* Just add 1 char at a time. Could allocate memory in blocks
		   but overkill for this as efficiency is not important */
		++word_len[i];
		words[i] = (char *)realloc(words[i],word_len[i]+1);
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
		if (!(repeat_cnt = atoi(words[0]))) 
		{
			errprintf("Repeat count must be > 0.\n");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (!word_len[1])
		{
			errprintf("Missing command.\n");
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
			errprintf(OFFLINE_ERROR);
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (word_len[cmd_word] < 3)
		{
			errprintf("Raw streamer commands need a minimum of 3 letters. eg: NTC\n");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		/* Clear value so its seen as a new value on RX so will get
		   printed out */
		clearValueOfRXKey(comword);

		if (sendCommand(repeat_cnt,comword,word_len[cmd_word]))
			ret = OK;
		else
			ret = ERR_CMD_FAIL;
		goto FREE;
	}

	/* Look for a command. If we can't find one try a macro */
	if ((comnum = getCommand(comword,word_len[cmd_word])) == -1)
	{
		if (findMacro(comword) != -1)
		{
			ret = runMacro(comword);
			goto FREE;
		}
		errprintf("Unknown command or macro \"%s\". Type \"help std\" for a list of\n",comword);
		puts("       commands or \"macro list\" for a list of macros.");
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
		errprintf(OFFLINE_ERROR);
		ret = ERR_CMD_FAIL;
		goto FREE;
	}

	/* Some commands take arguments */
	switch(comnum)
	{
	case COM_UP:
		flags.com_up = 1;
		break;

	case COM_DN:
		flags.com_dn = 1;
		break;

	case COM_EX:
		clearMenu(0);
		menu_cursor_pos = -1;
		break;

	case COM_EN:
		setMenuSelection();
		menu_cursor_pos = -1;
		break;

	case COM_ARTSAVE:
		if (save_state != SAVE_INACTIVE)
		{
			/* Should never happen, just in case */
			warnprintf("Currently saving - resetting.\n");
		}
		/* Ignore repeat_cnt in preparing */
		prepareSave(word_cnt > cmd_word ? words[cmd_word+1] : NULL);
		break;

	case COM_SETNAME:
		/* Setname requires an argument */
		if (word_cnt < cmd_word + 2)
		{
			usageprintf("setname <name>\n");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}

		clearValueOfRXKey("NFN");
		cmd_len = asprintf(&cmd,"NFN%s",words[cmd_word+1]);
		assert(cmd_len != -1);
		if (!sendCommand(repeat_cnt,cmd,cmd_len)) ret = ERR_CMD_FAIL;
		free(cmd);
		goto FREE;

	case COM_DS:
	case COM_FILTER:
	case COM_LRA:
		/* Commands that take a numeric argument */
		if (word_cnt < cmd_word + 2 || 
		    !isNumber(words[cmd_word+1]) || 
		    (val = atoi(words[cmd_word+1])) > 99)
		{
			usageprintf("%s <number>\n",commands[comnum].com);
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		clearValueOfRXKey(commands[comnum].data);
		cmd_len = asprintf(&cmd,"%s%02d",commands[comnum].data,val);
		assert(cmd_len != -1);
		if (!sendCommand(repeat_cnt,cmd,cmd_len)) ret = ERR_CMD_FAIL;
		free(cmd);
		goto FREE;
	}

	/* Send translated command(s) to the streamer */
	for(cmd=commands[comnum].data;;cmd=separator+1)
	{
		/* Separator in .data is for consistency the same as for 
		   commands entered by the user which is a semi colon */
		if ((separator = strchr(cmd,CMD_SEPERATOR)))
			cmd_len = (int)(separator - cmd);
		else
			cmd_len = strlen(cmd);
		
		/* Reset list entry. Will only use the first 3 chars */
		clearValueOfRXKey(cmd);

		/* Send command the given count times */
		if (!sendCommand(repeat_cnt,cmd,cmd_len))
		{
			ret = ERR_CMD_FAIL;
			break;
		}
		if (!separator) break;
	}

	FREE:
	for(i=0;i < word_cnt;++i) free(words[i]);
	if (comnum != COM_UP) flags.com_up = 0;
	if (comnum != COM_DN) flags.com_dn = 0;
	return ret;
}




/*** Look for a client command. Try exact match first and if that fails look 
     for partial match ***/
int getCommand(char *word, int len)
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
		if (flags.verbose)
			colprintf("~FTExpanded to:~RS \"%s\"\n",commands[comnum].com);
		return comnum;
	}
	return -1;
}




int sendCommand(int repeat_cnt, char *cmd, int cmd_len)
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
	int comnum, int cmd_word, int word_cnt, char **words)
{
	char *param1 = NULL;
	char *param2 = NULL;
	char *end;
	int max = 0;
	
	if (word_cnt > cmd_word)
	{
		param1 = words[cmd_word+1];
		if (word_cnt > cmd_word + 1)
		{
			param2 = words[cmd_word+2];
			if (word_cnt > cmd_word + 3)
			{
				end = NULL;
				max = strtol(words[cmd_word+3],&end,10);
				if (end && *end) max = -1;
			}
		}
	}

	switch(comnum)
	{
	case COM_EXIT:
		exitprintf("by command");
		doExit(0);
	case COM_TOGGLE:
		return comToggle(param1);
	case COM_PROMPT:
		return comPrompt(param1);
	case COM_RAW:
		return comRaw(param1);
	case COM_SHOW:
		return comShow(param1,param2,max);
	case COM_CLEAR:
		return comClear(param1);
	case COM_HELP:
		return comHelp(param1,param2);
	case COM_CONNECT:
		return comConnect(param1);
	case COM_DISCONNECT:
		return comDisconnect();
	case COM_WAIT:
		return comWait(param1);
	case COM_CLS:
		/* [2J clears screen, [H makes the cursor go to the top left */
		write(STDOUT,"\033[2J\033[H",7);
		break;
	case COM_ECHO:
		comEcho(cmd_word,word_cnt,words);
		break;
	case COM_MACRO:
		return comMacro(param1,param2,cmd_word,word_cnt,words);
	default:
		assert(0);
	}
	return OK;
}


/********************************** COMMANDS **********************************/

int comToggle(char *opt)
{
	char *options[4] =
	{
		"tracktm",
		"colour",
		"htmlamp",
		"verbose"
	};
	int len;
	int i;

	if (!opt) goto USAGE;
	len = strlen(opt);
	for(i=0;i < 4;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			flags.show_track_time = !flags.show_track_time;
			printFlagTrackTime();
			return OK;
		case 1:
			flags.use_colour = !flags.use_colour;
			printFlagColour();
			return OK;
		case 2:
			flags.trans_html_amps = !flags.trans_html_amps;
			printFlagHTML();
			return OK;
		case 3:
			flags.verbose = !flags.verbose;
			printFlagVerb();
			return OK;
		}
	}
	USAGE:
	usageprintf("toggle tracktm\n");
	puts("              colour");
	puts("              htmlamp");
	puts("              verbose");
	return ERR_CMD_FAIL;
}




int comPrompt(char *param)
{
	int val;

	if (param)
	{
		val = atoi(param);
		if (isNumber(param) && val >= 0 && val < NUM_PROMPTS)
		{
			prompt_type = val;
			return OK;
		}
	}
	usageprintf("prompt [0 to %d]\n",NUM_PROMPTS-1);
	return ERR_CMD_FAIL;
}




int comRaw(char *param)
{
	int val;

	if (param)
	{
		val = atoi(param);
		if (isNumber(param) && val >= 0 && val < NUM_RAW_LEVELS)
		{
			raw_level = val;
			ok();
			return OK;
		}
	}
	usageprintf("raw [0 to %d]\n",NUM_RAW_LEVELS-1);
	return ERR_CMD_FAIL;
}




int comShow(char *opt, char *pat, int max)
{
	char *options[13] =
	{
		/* 0 */
		"titles",
		"xtitles",
		"rx",
		"rawrx",
		"rxcoms",

		/* 5 */
		"txcoms",
		"times",
		"flags",
		"menu",
		"selected",

		/* 10 */
		"connection",
		"history",
		"version"
	};
	int len;
	int i;

	if (!opt) goto USAGE;
	len = strlen(opt);
	for(i=0;i < 13;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			return printTitles(0,pat,max);
		case 1:
			return printTitles(1,pat,max);
		case 2:
			return prettyPrintRXList(pat,max);
		case 3:
			return dumpRXList(pat,max);
		case 4:
			printRXCommands(pat);
			break;
		case 5:
			optShowTXCommands(pat);
			break;
		case 6:
			optShowTimes();
			break;
		case 7:
			optShowFlags();
			break;
		case 8:
			printMenuList();
			break;
		case 9:
			printMenuSelection();
			break;
		case 10:
			optShowConStat();
			break;
		case 11:
			optShowHistory();
			break;
		case 12:
			version(1);
			break;
		}
		return OK;
	}

	USAGE:
	usageprintf("show titles  [<pattern>  [<count>]]\n");
	puts("            xtitles [<pattern>  [<count>]]");
	puts("            rx      [<pattern>  [<count>]]");
	puts("            rawrx   [<pattern>  [<count>]]");
	puts("            rxcoms  [<pattern>]");
	puts("            txcoms  [<pattern>]");
	puts("            times");
	puts("            flags");
	puts("            menu");
	puts("            selected");
	puts("            connection");
	puts("            history");
	puts("            version");
	return ERR_CMD_FAIL;
}




void optShowFlags()
{
	colprintf("\n~BB~FW*** Toggle flags ***\n\n");
	printFlagTrackTime();
	printFlagColour();
	printFlagHTML();
	printFlagVerb();
	putchar('\n');
}




void optShowTXCommands(char *pat)
{
	const char *data;
	char str[20];
	int cnt1;
	int cnt2;
	int i;

	colprintf("\n~BG~FW*** TX streamer command mappings ***\n\n");
	for(i=cnt1=cnt2=0;i < NUM_COMMANDS;++i)
	{
		if (commands[i].data)
		{
			++cnt1;
			if (!pat || wildMatch(commands[i].com,pat))
			{
				if (cnt2 && !(cnt2 % 3)) putchar('\n');
				data = commands[i].data;

				/* Some streamer commands are incomplete */
				if (!strcmp(data,"DIM") || 
				    !strcmp(data,"DGF") ||
				    !strcmp(data,"LRA"))
				{
					sprintf(str,"%snn",data);
				}
				else if (!strcmp(data,"NFN"))
					strcpy(str,"NFN*");
				else
					strcpy(str,data);

				colprintf("%-8s = ~FT%-15s~RS",
					commands[i].com,str);
				++cnt2;
			}
		}
	}
	if (pat)
		printf("\n\n%d of %d commands.\n\n",cnt2,cnt1);
	else
		printf("\n\n%d commands.\n\n",cnt1);
}




void optShowTimes()
{
	colprintf("\n~BB~FW*** Times ***\n\n");
	printf("Local  : %s\n",getTime());
	printf("Run    : %s\n",getRawTimeString(start_time));
	printf("Connect: %s\n",getTimeString(connect_time));
	printTrackTime(0);
	putchar('\n');
}




void optShowConStat()
{
	t_iscp_data *pkt_data;

	colprintf("\n~BB~FW*** Connection status and traffic ***\n\n");
	printf("Streamer TCP: %s:%d\n",inet_ntoa(con_addr.sin_addr),tcp_port);
	printf("Status      : ");
	colprintf(connect_time ? "~FGCONNECTED\n" : "~FRDISCONNECTED\n");
	printf("Connect time: %s\n",getTimeString(connect_time));
	printf("Last RX ago : %s\n",getTimeString(last_rx_time));
	printf("Last TX ago : %s\n",getTimeString(last_tx_time));

	if (buffer[BUFF_TCP].data)
	{
		pkt_data = (t_iscp_data *)(buffer[BUFF_TCP].data + pkt_hdr->hdr_len);
		colprintf("Last RX com : ~FT%.3s\n",pkt_data->command);
	}
	else puts("Last RX com : ---");

	printf("RX reads    : %lu\n",rx_reads);
	printf("RX data     : %s\n",bytesSizeStr(rx_bytes));
	printf("TX writes   : %lu\n",tx_writes);
	printf("TX data     : %s\n\n",bytesSizeStr(tx_bytes));
}




void optShowHistory()
{
	int cnt;
	int bn;
	int i;

	colprintf("\n~BB~FW*** Command history ***\n\n");
	bn = (keyb_buffnum + 2) % MAX_HIST_BUFFERS;
	for(i=cnt=0;i < MAX_HIST_BUFFERS-1;++i)
	{
		if (buffer[bn].len)
			colprintf("~FB~OL%3d:~RS %s\n",++cnt,buffer[bn].data);
		bn = (bn + 1) % MAX_HIST_BUFFERS;
	}
	puts("\nEnter !<number> or up/down arrow keys to select.\n");
}




int comClear(char *opt)
{
	char *options[5] =
	{
		"rx",
		"menu",
		"titles",
		"history",
		"*"
	};
	int len;
	int i;

	if (!opt) goto USAGE;
	len = strlen(opt);
	for(i=0;i < 5;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			clearRXList();
			break;
		case 1:
			clearMenu(1);
			break;
		case 2:
			clearTitles();
			break;
		case 3:
			clearHistory();
			break;
		case 4:
			clearRXList();
			clearMenu(1);
			clearTitles();
			clearHistory();
		}
	}
	return OK;

	USAGE:
	usageprintf("clear rx\n");
	puts("             menu");
	puts("             titles");
	puts("             history");
	puts("             *       (Clear all the above)");
	return ERR_CMD_FAIL;
}




int comHelp(char *opt, char *pat)
{
	char *options[4] =
	{
		"extra",
		"sorted",
		"notes",
		"usage"
	};
	int len;
	int ret;
	int i;

	/* If no option given do standard help */
	if (!opt)
	{
		optHelpMain(0,NULL);
		return OK;
	}
	len = strlen(opt);
	ret = ERR_CMD_FAIL;
	for(i=0;i < 4;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			optHelpMain(1,pat);
			return OK;
		case 1:
			optHelpSorted(pat);
			return OK;
		case 2:
			optHelpNotes();
			return OK;
		case 3:
			ret = OK;
			goto USAGE;
		}
	}

	USAGE:
	usageprintf("help [extra  [<pattern>]]\n");
	puts("            [sorted [<pattern>]]");
	puts("            [notes]");
	puts("            [usage]");
	return ret;
}




void optHelpMain(int extra, char *pat)
{
	int nlafter;
	int cnt;
	int i;

	nlafter = 5;

	colprintf("\n~BG~FW*** Client commands ***\n\n");
	for(i=cnt=0;i < NUM_COMMANDS;++i)
	{
		if (i && commands[i].data && !commands[i-1].data)
		{
			if (cnt % nlafter) putchar('\n');
			colprintf("\n~BB~FW*** Streamer commands ***\n\n");
			if (extra) nlafter = 4;
			cnt = 0;
		}
		if (!pat || wildMatch(commands[i].com,pat))
		{
			if (extra)
			{
				if (i > LAST_CLIENT_COM)
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

	if (extra)
	{
		colprintf("\n~FMUsage:~RS [<repeat count>] <client/streamer command>   eg: 3 up\n");
		puts("                                                        con 192.168.0.1");
		puts("       [<repeat count>] <raw streamer command>      eg: 3 OSDUP\n");
	}
	else putchar('\n');

	puts("Enter \"help usage\" for further help options.\n");
}



void optHelpNotes()
{
	colprintf("\n~BB~FW*** Help notes ***\n\n");
	puts("1) Any commands starting with a capital letter are passed to the streamer");
	puts("   unchanged as raw commands. eg: OSDUP\n");
	puts("2) Commands and their sub options can be shortened to any matching substring"); 
	puts("   depending on the internal order of the commands and options.");
	puts("   Eg: \"help notes\" can be shortened to \"h n\", \"show version\" to \"s v\"\n");
	puts("3) Built in, raw commands and macros can be chained with semicolon separators.");
	puts("   eg: \"tunein; 3 OSDDOWN; wait 2; en\"\n");
	puts("4) Some commands take an optional wildcard pattern to limit the number of");
	puts("   results. Eg: \"help std *stat\" (or \"h s *stat\" shorthand)\n");
	puts("5) The \"titles\" and \"show\" commands can take a 3rd parameter which is the");
	puts("   number of entries to print from the start of the list.");
	puts("   Eg: \"titles std * 5\" to show the first 5 titles (or \"ti s * 5\").\n");
	puts("6) Either double or single quotes can be used to pass whitespace as data.");
	puts("   eg: setname \"Neil's streamer\"");
	puts("       setname 'Pass double\"quote'\n");
	puts("7) Macros can be run either using \"macro run <macro name>\" or simply just by");
	puts("   entering the macro name.\n");
}




void optHelpSorted(char *pat)
{
	int cnt;
	int i;

	colprintf("\n~BG~FW*** Sorted commands ***\n\n");
	for(i=cnt=0;i < NUM_COMMANDS;++i)
	{
		if (!pat || wildMatch(sorted_commands[i],pat))
		{
			printf("   %-10s",sorted_commands[i]);
			if (!(++cnt % 5)) putchar('\n');
		}
	}
	if (cnt % 5) putchar('\n');
	putchar('\n');
}




int comConnect(char *param)
{
	if (tcp_sock)
	{
		if (connect_time)
		{
			errprintf("Already connected.\n");
			return ERR_CMD_FAIL;
		}
		networkClear();
	}
	/* Could have:       con [address]
	               <cnt> con [address]  - pointless to do but... */
	if (param) ipaddr = strdup(param);
	if (!networkStart()) networkClear();
	return OK;
}




int comDisconnect()
{
	if (tcp_sock)
	{
		networkClear();
		return OK;
	}
	errprintf("Not connected.\n");
	return ERR_CMD_FAIL;
}




int comWait(char *param)
{
	struct timeval tv;
	fd_set mask;
	float secs;
	u_int usecs;
	u_int end;

	if (param && (secs = atof(param)) > 0)
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
	usageprintf("wait <seconds>\n");
	return ERR_CMD_FAIL;
}




void comEcho(int cmd_word, int word_cnt, char **words)
{
	int i;

	for(i=cmd_word+1;i < word_cnt;++i) colprintf("%s ",words[i]);

	/* Sends an ansi reset code along with nl */
	colprintf("\n"); 
}




int comMacro(char *opt, char *name, int cmd_word, int word_cnt, char **words)
{
	char *options[8] =
	{
		/* 0 */
		"define",
		"append",
		"delete",
		"run",
		"load",

		/* 5 */
		"save",
		"sava",
		"list"
	};
	int len;
	int ret;
	int i;

	if (!opt) goto USAGE;
	len = strlen(opt);
	ret = ERR_CMD_FAIL;

	for(i=0;i < 8;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			ret = optMacroDefine(cmd_word,word_cnt,words);
			break;
		case 1:
			ret = optMacroAppend(cmd_word,word_cnt,words);
			break;
		case 2:
			if (name) return deleteMacro(name);
			goto USAGE;
		case 3:
			if (name) return runMacro(name);
			goto USAGE;
		case 4:
			if (name) return loadMacros(name);
			goto USAGE;
		case 5:
			ret = optMacroSave(0,cmd_word,word_cnt,words);
			break;
		case 6:
			ret = optMacroSave(1,cmd_word,word_cnt,words);
			break;
		case 7:
			listMacros();
			return OK;
		}
	}
	if (ret != ERR_CMD_FAIL) return ret;

	USAGE:
	usageprintf("macro define <macro name>   [\"<macro command list>\"]\n");
	puts("             append <macro name>   [\"<macro command list>\"]");
	puts("             delete <macro name>/*");
	puts("             run    <macro name>");
	puts("             load   <filename>");
	puts("             save   <filename>     [<macro name>]");
	puts("             sava   <filename>     [<macro name>]");
	puts("             list");
	return ERR_CMD_FAIL;
}




int optMacroDefine(int cmd_word, int word_cnt, char **words)
{
	switch(word_cnt - cmd_word)
	{
	case 3:
		return initMultiLineMacro(words[cmd_word+2]);
	case 4:
		return insertMacro(words[cmd_word+2],words[cmd_word+3]);
	}
	return ERR_CMD_FAIL;
}




int optMacroAppend(int cmd_word, int word_cnt, char **words)
{
	switch(word_cnt - cmd_word)
	{
	case 3:
		return initMultiLineMacroAppend(words[cmd_word+2]);
	case 4:
		return appendMacroComlist(words[cmd_word+2],words[cmd_word+3]);
	}
	return ERR_CMD_FAIL;
}




int optMacroSave(int append, int cmd_word, int word_cnt, char **words)
{
	char *filename;

	if (word_cnt - cmd_word >= 3)
	{
		filename = words[cmd_word+2];
		if (!append && !access(filename,F_OK))
		{
			errprintf("File \"%s\" already exists.\n",filename);
			return ERR_FILE;
		}

		switch(word_cnt - cmd_word)
		{
		case 3:
			return saveAllMacros(filename,append);
		case 4:
			return saveMacro(filename,words[cmd_word+3],append);
		}
	}
	return ERR_CMD_FAIL;
}


/********************************** FLAGS *************************************/

void printFlagTrackTime()
{
	printf("Show track time : ");
	PRINT_ON_OFF(flags.show_track_time);
}




void printFlagColour()
{
	printf("Ansi colour     : ");
	PRINT_ON_OFF(flags.use_colour);
}




void printFlagHTML()
{
	printf("Trans HTML codes: ");
	PRINT_ON_OFF(flags.trans_html_amps);
}




void printFlagVerb()
{
	printf("Verbose output  : ");
	PRINT_ON_OFF(flags.verbose);
}

/********************************** MISC *************************************/

/*** addr1 and addr2 contain the array element memory location, not the
     pointer values stored at the location, so they have to be deref'd ***/
int compareComs(const void *addr1, const void *addr2)
{
	return strcmp(*(char **)addr1,*(char **)addr2);
}




/*** Sort the commands into alphabetic order for 'shelp' ***/
void sortCommands()
{
	int i;

	for(i=0;i < NUM_COMMANDS;++i)
		sorted_commands[i] = commands[i].com;
	qsort(sorted_commands,NUM_COMMANDS,sizeof(char *),compareComs);
}




void clearHistory()
{
	int i;
	for(i=0;i < MAX_HIST_BUFFERS;++i) clearBuffer(i);
	colprintf("History ~FGcleared.\n");
}




char *bytesSizeStr(u_long bytes)
{
	static char str[20];

	/* Only start printing in kilobytes from 10000 as eg 2345 bytes is 
	   still easy to read */
	if (bytes < 1e4) 
		sprintf(str,"%lu bytes",bytes);
	else if (bytes < 1e6)
		sprintf(str,"%.1fK",(double)bytes / 1e3);
	else 
		sprintf(str,"%.1fM",(double)bytes / 1e6);
	return str;
}




/*** Get the current time down to the microsecond. Value wraps once every
     1000 seconds. Goes from 0 -> 999999999 (1 billion - 1) ***/
u_int getUsecTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (u_int)(tv.tv_sec % 1000) * 1000000 + (u_int)tv.tv_usec;
}
