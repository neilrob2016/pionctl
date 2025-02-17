#include "globals.h"

#define MAX_WORDS         5
#define NUM_CLIENT_COMS   (LAST_CLIENT_COM + 1)
#define NUM_STREAMER_COMS (NUM_COMMANDS - NUM_CLIENT_COMS)
#define CMD_SEPERATOR     ';'
#define PRINT_ON_OFF(F)   colPrintf(F ? "~FGON\n" : "~FROFF\n");

char *sorted_client_coms[NUM_CLIENT_COMS];
char *sorted_streamer_coms[NUM_STREAMER_COMS];

int parseCommand(char *buff, int bufflen);
int sendCommand(int repeat_cnt, char *cmd, int cmd_len);
int copyHistoryBuffer(int buffnum);
int comSeek(int repeat_cnt, char *timestr);
int comSetName(int repeat_cnt, char *name);
int comNumeric(int comnum, int repeat_cnt, char *valstr);
int processBuiltInCommand(
	int comnum, int cmd_word, int word_cnt, char **words);

int comToggle(char *opt);
int comPrompt(char *param);
int comRaw(char *param);
int comShow(char *opt, char *pat, int max);

void optShowSettings(void);
void optShowTXCommands(char *pat);
void optShowTimes(void);
void optShowConStat(void);
void optShowHistory(void);

int  comClear(char *opt);
int  comHelp(char *opt, char *pat);
int  optHelpMain(int extra, char *pat);
int  optHelpSorted(char *pat);
void optHelpNotes(void);

int  comConnect(char *param1);
int  comDisconnect(void);
int  comTimeout(char *param1);
void comClearHistory(void);
int  comWait(int comnum, char *param);
void comEcho(int cmd_word, int word_cnt, char **words);
int  comRun(char *param);
int  comOnError(char *param1, char *param2);

int  comMacro(char *opt, char *name, int cmd_word, int word_cnt, char **words);
int  optMacroDefine(int cmd_word, int word_cnt, char **words);
int  optMacroAppend(int cmd_word, int word_cnt, char **words);
int  optMacroSave(int append, int cmd_word, int word_cnt, char **words);

void printFlagTrackTime(void);
void printFlagPromptTime(void);
void printFlagHTML(void);
void printFlagColour(void);
void printFlagVerb(void);

void  clearHistory(void);
char *bytesSizeStr(u_long num);

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
		for(separator=ptr;*separator && separator < end;++separator)
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
			errPrintf("Unterminated quotes.\n");
			if (!flags.macro_running && !flags.cmdfile_running)
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			break;
		}
	
		if (*separator)
			len = (int)(separator - ptr);
		else
			len = (int)(end - ptr);

		if (len)
		{
			if (flags.macro_running || flags.cmdfile_running)
			{
				/* Skip initial whitespace */
				for(ptr2=ptr;*ptr2 < 33;++ptr2);
				if (flags.verbose)
				{
					colPrintf("~FB~OLExec cmd:~RS %.*s\n",
						len - (int)(ptr2-ptr),ptr2);
				}
			}

			if ((ret = parseCommand(ptr,len)) != ERR_CMD_MISSING &&
			    !flags.macro_running && !flags.cmdfile_running)
			{
				keyb_buffnum = (keyb_buffnum + 1) % MAX_HIST_BUFFERS;
			}

			/* Stop running */
			if (flags.do_halt || flags.do_return) return OK;

			if (ret != OK)
			{
				if (flags.on_error_halt) return ret;
				if (on_error_skip_set)
				{
					on_error_skip_cnt = on_error_skip_set;
					continue;
				}
			}
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
	int i;

	if (!bufflen) return ERR_CMD_MISSING;
	end = buff + bufflen;
	comnum = 0;

	/* Make sure we have something other than whitespace */
	for(s=buff;s < end;++s) if (!isspace(*s)) break;
	if (s == buff+bufflen) return ERR_CMD_MISSING;

	if (on_error_skip_cnt)
	{
		--on_error_skip_cnt;
		return OK;
	}

	/* See if we have !<num> which means put that history buffer into
	   current keyboard buffer */
	if (buff[0] == '!')
	{
		for(s=buff+1;s < end && isdigit(*s);++s);
		if (s < end)
		{
			errPrintf("'!' requires a number.\n");
			return ERR_CMD_MISSING;
		}
		if (!copyHistoryBuffer(atoi((buff + 1))))
		{
			errPrintf("Invalid or empty history buffer.\n");
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
			errPrintf("Maximum (%d) words or strings exceeded.\n",MAX_WORDS);
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
			errPrintf("Repeat count must be > 0.\n");
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (!word_len[1])
		{
			errPrintf("Missing command.\n");
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
			errNotConnected();
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		if (word_len[cmd_word] < 3)
		{
			errPrintf("Raw streamer commands need a minimum of 3 letters. eg: NTC\n");
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
		errPrintf("Unknown command or macro \"%s\". Type \"help\" for a list of commands\n",comword);
		puts("       or \"macro list\" for a list of macros.");
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
		errNotConnected();
		ret = ERR_CMD_FAIL;
		goto FREE;
	}

	/* Some commands take arguments or we store command num for NRI */
	switch(comnum)
	{
	case COM_UP:
		flags.com_up = 1;
		addReverseCom(rev_arr,COM_UP,repeat_cnt);
		break;

	case COM_DN:
		flags.com_dn = 1;
		addReverseCom(rev_arr,COM_DN,repeat_cnt);
		break;

	case COM_EX:
		clearMenu(0);
		addReverseCom(rev_arr,COM_EX,repeat_cnt);
		menu_cursor_pos = -1;
		break;

	case COM_EN:
		setMenuSelection();
		addReverseCom(rev_arr,COM_EN,repeat_cnt);
		menu_cursor_pos = -1;
		break;

	case COM_ARTSAVE:
		if (save_state != SAVE_INACTIVE)
		{
			/* Should never happen, just in case */
			warnPrintf("Currently saving - resetting.\n");
		}
		/* Ignore repeat_cnt in preparing */
		if (!prepareSave(word_cnt > cmd_word ? words[cmd_word+1] : NULL))
		{
			ret = ERR_CMD_FAIL;
			goto FREE;
		}
		break;

	case COM_SEEK:
		/* Requires [[HH:]MM:]SS argument for absolute seek */
		if (word_cnt < cmd_word + 2 ||
		    !comSeek(repeat_cnt,words[cmd_word+1]))
		{
			usagePrintf("seek [[HH:]MM:]SS\n");
			ret = ERR_CMD_FAIL;
		}
		goto FREE;

	case COM_SETNAME:
		/* Setname requires an argument */
		if (word_cnt < cmd_word + 2 ||
		    !comSetName(repeat_cnt,words[cmd_word+1]))
		{
			usagePrintf("setname <name>\n");
			ret = ERR_CMD_FAIL;
		}
		goto FREE;

	case COM_DIM:
	case COM_FILTER:
	case COM_LRA:
		if (word_cnt < cmd_word + 2 || 
		    !comNumeric(comnum,repeat_cnt,words[cmd_word+1]))
		{
			usagePrintf("%s <number>\n",commands[comnum].com);
			ret = ERR_CMD_FAIL;
		}
		goto FREE;

	case COM_SETUP:
	case COM_SERIAL:
	case COM_ETHMAC:
	case COM_ICONURL:
	case COM_MODINFO:
	case COM_TIDALVER:
	case COM_ECOVER:
	case COM_PRODID:
		nri_command = comnum;
		break;
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

	/* Set the flag that causes the reverse nav commands to be reset
	   so we don't end up with some really long sequence */
	switch(comnum)
	{
	case COM_MACRO:
		flags.reset_reverse = 1;
		break;
	case COM_UP:
	case COM_DN:
	case COM_EN:
	case COM_EX:
		flags.reset_reverse = 0;
		break;
	default:
		if (comnum > LAST_CLIENT_COM) flags.reset_reverse = 1;
	}
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
			colPrintf("~FMExpanded to:~RS \"%s\"\n",commands[comnum].com);
		return comnum;
	}
	return -1;
}




int sendCommand(int repeat_cnt, char *cmd, int cmd_len)
{
	int i;
	for(i=0;i < repeat_cnt;++i)
	{
		if (i && repeat_wait_secs) doWait(COM_WAIT,repeat_wait_secs);
		if (!writeSocket(cmd,cmd_len)) return 0;
	}
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


/************************* STREAMER COMMAND FUNCS ****************************/

/*** Absolute seek to a given time position in a track. Can be any combination 
     of S, SS, M:S, M:SS, MM:SS etc up to HH:MM:SS ***/
int comSeek(int repeat_cnt, char *timestr)
{
	char cmd[12];
	char *colon[2];
	char *ptr;
	int colcnt;
	int hours = 0;
	int mins = 0;
	int secs = 0;
	int cmd_len;
	int len = strlen(timestr);

	/* SS to HH:MM:SS */
	if (len > 8) return 0;

	for(ptr=timestr,colcnt=0;*ptr;++ptr)
	{
		if (*ptr == ':')
		{
			if (colcnt == 2) return 0;
			colon[colcnt++] = ptr;
		}
		else if (!isdigit(*ptr)) return 0;
	}
	switch(colcnt)
	{
	case 0:
		secs = atoi(timestr);
		break;
	case 1:
		*colon[0] = 0;
		mins = atoi(timestr);
		secs = atoi(colon[0]+1);
		break;
	case 2:
		secs = atoi(colon[1]+1);
		*colon[1] = 0;
		mins = atoi(colon[0]+1);
		*colon[1] = 0;
		hours = atoi(timestr);
		break;
	}
	if (hours > 99 || mins > 59 || secs > 59) return 0;

	cmd_len = snprintf(cmd,sizeof(cmd),"NTS%02d:%02d:%02d",hours,mins,secs);
	assert(cmd_len != -1);
	printf("Seeking to track time %02d:%02d:%02d...\n",hours,mins,secs);
	return sendCommand(repeat_cnt,cmd,cmd_len);
}




int comSetName(int repeat_cnt, char *name)
{
	int ret;
	int cmd_len;
	char *cmd;

	clearValueOfRXKey("NFN");
	cmd_len = asprintf(&cmd,"NFN%s",name);
	assert(cmd_len != -1);
	ret = sendCommand(repeat_cnt,cmd,cmd_len);
	free(cmd);
	return ret;
}




/*** Commands that take a numeric argument ***/
int comNumeric(int comnum, int repeat_cnt, char *valstr)
{
	int val;
	int cmd_len;
	int ret;
	char *cmd;

	if (!isNumber(valstr) || (val = atoi(valstr)) > 99) return 0;

	clearValueOfRXKey(commands[comnum].data);
	cmd_len = asprintf(&cmd,"%s%02d",commands[comnum].data,val);
	assert(cmd_len != -1);
	ret = sendCommand(repeat_cnt,cmd,cmd_len);
	free(cmd);
	return ret;
}



/************************* CLIENT COMMAND FUNCS ****************************/

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
	case COM_QUIT:
		quitPrintf("by command");
		doExit(0);
		/* Avoids gcc warning */
		break;
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
	case COM_TIMEOUT:
		return comTimeout(param1);
	case COM_WAIT:
	case COM_WAIT_MENU:
	case COM_WAIT_REPEAT:
		return comWait(comnum,param1);
	case COM_CLS:
		/* [2J clears screen, [H makes the cursor go to the top left */
		write(STDOUT_FILENO,"\033[2J\033[H",7);
		break;
	case COM_ECHO:
		comEcho(cmd_word,word_cnt,words);
		break;
	case COM_MACRO:
		return comMacro(param1,param2,cmd_word,word_cnt,words);
	case COM_BACK:
		runShowReverse(1,param1);
		break;
	case COM_RUN:
		return comRun(param1);
	case COM_HALT:
		flags.do_halt = 1;
		return OK;
	case COM_RETURN:
		flags.do_return = 1;
		return OK;
	case COM_ON_ERROR:
		return comOnError(param1,param2);
	default:
		assert(0);
	}
	return OK;
}




int comToggle(char *opt)
{
	char *options[] =
	{
		"tracktm",
		"prompttm",
		"htmlamp",
		"colour",
		"verbose"
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
			flags.show_track_time = !flags.show_track_time;
			printFlagTrackTime();
			return OK;
		case 1: 
			flags.update_prompt_time = !flags.update_prompt_time;
			printFlagPromptTime();
			return OK;
		case 2:
			flags.trans_html_amps = !flags.trans_html_amps;
			printFlagHTML();
			return OK;
		case 3:
			flags.use_colour = !flags.use_colour;
			printFlagColour();
			return OK;
		case 4:
			flags.verbose = !flags.verbose;
			printFlagVerb();
			return OK;
		}
	}
	USAGE:
	usagePrintf("toggle tracktm  : Show track time received from streamer.\n");
	puts("              prompttm : Update prompt times if appropriate prompt type set.");
	puts("              htmlamp  : Translate HTML ampersand values text data.");
	puts("              colour");
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
	usagePrintf("prompt [0 to %d]\n",NUM_PROMPTS-1);
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
	usagePrintf("raw [0 to %d]\n",NUM_RAW_LEVELS-1);
	return ERR_CMD_FAIL;
}




int comShow(char *opt, char *pat, int max)
{
	char *options[14] =
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
		"settings",
		"menu",
		"selected",

		/* 10 */
		"connection",
		"history",
		"version",
		"back"
	};
	int len;
	int i;

	if (!opt) goto USAGE;
	len = strlen(opt);
	for(i=0;i < 14;++i)
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
			optShowSettings();
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
		case 13:
			runShowReverse(0,NULL);
			break;
		}
		return OK;
	}

	USAGE:
	usagePrintf("show titles  [<pattern>  [<count>]]\n");
	puts("            xtitles [<pattern>  [<count>]]");
	puts("            rx      [<pattern>  [<count>]]");
	puts("            rawrx   [<pattern>  [<count>]]");
	puts("            rxcoms  [<pattern>]");
	puts("            txcoms  [<pattern>]");
	puts("            times");
	puts("            settings");
	puts("            menu");
	puts("            back");
	puts("            selected");
	puts("            connection");
	puts("            history");
	puts("            version");
	return ERR_CMD_FAIL;
}




void optShowSettings(void)
{
	colPrintf("\n~BM~FW*** Wait seconds ***\n\n");
	printf("Cmd repeat wait: %.2f\n",repeat_wait_secs);
	colPrintf("Connect timeout: %.2f %s\n\n",
		timeout_secs,
		timeout_secs ? "" : "~FY(Wait for TCP timeout)");

	colPrintf("~BB~FW*** Toggle flags ***\n\n");
	printFlagTrackTime();
	printFlagPromptTime();
	printFlagHTML();
	printFlagColour();
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

	colPrintf("\n~BB~FW*** TX streamer command mappings ***\n\n");
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
					snprintf(str,sizeof(str),"%snn",data);
				}
				else if (!strcmp(data,"NFN"))
					strcpy(str,"NFN*");
				else
					strcpy(str,data);

				colPrintf("%-8s = ~FT%-15s~RS",
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




void optShowTimes(void)
{
	colPrintf("\n~BB~FW*** Times ***\n\n");
	printf("Local  : %s\n",getTime());
	printf("Run    : %s\n",getRawTimeString(start_time));
	printf("Connect: %s\n",getTimeString(connect_time));
	printTrackTime(0);
	putchar('\n');
}




void optShowConStat(void)
{
	t_iscp_data *pkt_data;

	colPrintf("\n~BB~FW*** Connection status and traffic ***\n\n");
	printf("Streamer TCP: %s:%d\n",inet_ntoa(con_addr.sin_addr),tcp_port);
	printf("Status      : ");
	colPrintf(connect_time ? "~FGCONNECTED\n" : "~FRDISCONNECTED\n");
	printf("Connect time: %s\n",getTimeString(connect_time));
	printf("Last RX ago : %s\n",getTimeString(last_rx_time));
	printf("Last TX ago : %s\n",getTimeString(last_tx_time));

	if (buffer[BUFF_TCP].data)
	{
		pkt_data = (t_iscp_data *)(buffer[BUFF_TCP].data + pkt_hdr->hdr_len);
		colPrintf("Last RX com : ~FT%.3s\n",pkt_data->command);
	}
	else puts("Last RX com : ---");

	printf("RX reads    : %lu\n",rx_reads);
	printf("RX data     : %s\n",bytesSizeStr(rx_bytes));
	printf("TX writes   : %lu\n",tx_writes);
	printf("TX data     : %s\n\n",bytesSizeStr(tx_bytes));
}




void optShowHistory(void)
{
	int cnt;
	int bn;
	int i;

	colPrintf("\n~BB~FW*** Command history ***\n\n");
	bn = (keyb_buffnum + 2) % MAX_HIST_BUFFERS;
	for(i=cnt=0;i < MAX_HIST_BUFFERS-1;++i)
	{
		if (buffer[bn].len)
			colPrintf("~FM%3d:~RS %s\n",++cnt,buffer[bn].data);
		bn = (bn + 1) % MAX_HIST_BUFFERS;
	}
	puts("\nEnter !<number> or up/down arrow keys to select.\n");
}




int comClear(char *opt)
{
	char *options[6] =
	{
		"rx",
		"back",
		"menu",
		"titles",
		"history",
		"all"
	};
	int len;
	int i;

	if (!opt) goto USAGE;

	len = strlen(opt);
	for(i=0;i < 6;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			clearRXList();
			return OK;
		case 1:
			clearReverse();
			return OK;
		case 2:
			clearMenu(1);
			return OK;
		case 3:
			clearTitles();
			return OK;
		case 4:
			clearHistory();
			return OK;
		case 5:
			clearRXList();
			clearMenu(1);
			clearTitles();
			clearHistory();
			clearReverse();
			return OK;
		}
	}

	USAGE:
	usagePrintf("clear rx\n");
	puts("             menu");
	puts("             back");
	puts("             titles");
	puts("             history");
	puts("             all      (Clear all the above)");
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
	int i;

	/* If no option given do standard help */
	if (!opt)
	{
		optHelpMain(0,NULL);
		goto DONE;
	}
	len = strlen(opt);
	for(i=0;i < 4;++i)
	{
		if (strncmp(opt,options[i],len)) continue;

		switch(i)
		{
		case 0:
			if (optHelpMain(1,pat) != OK) goto USAGE;
			goto DONE;
		case 1:
			if (optHelpSorted(pat) != OK) goto USAGE;
			goto DONE;
		case 2:
			optHelpNotes();
			goto DONE;
		case 3:
			goto USAGE;
		}
	}
	/* Not found, just assume its a pattern */
	if (optHelpMain(0,opt) != OK) goto USAGE;

	DONE:
	colPrintf("Enter \"~FYhelp usage~RS\" for further help options.\n\n");
	return OK;

	USAGE:
	usagePrintf("help [<pattern>]\n");
	puts("            [extra      [<pattern>]]");
	puts("            [sorted     [<pattern>]]");
	puts("            [notes]");
	puts("            [usage]");
	return ERR_CMD_FAIL;
}




int optHelpMain(int extra, char *pat)
{
	int nlafter = 5;
	int nl = 0;
	int cnt;
	int i;

	if (pat && !isPattern(pat)) return ERR_CMD_FAIL;

	colPrintf("\n~BM~FW*** Local commands ***\n\n");
	for(i=cnt=0;i < NUM_COMMANDS;++i)
	{
		if (i && commands[i].data && !commands[i-1].data)
		{
			if (cnt % nlafter) putchar('\n');
			colPrintf("\n~BB~FW*** Streamer commands ***\n\n");
			if (extra) nlafter = 4;
			nl = 1;
			cnt = 0;
		}
		if (pat)
		{
			if (!wildMatch(commands[i].com,pat)) continue;
		}
		else
		{
			/* Print command category headers */
			switch(i)
			{
			case COM_MENU:
			case COM_DIM:
			case COM_FILTER:
			case COM_ALBUM:
			case COM_ARTDIS:
			case COM_SBON:
			case COM_AUINFO:
			case COM_APDON:
			case COM_MSV:
			case COM_DTS:
			case COM_STOP:
			case COM_MRMSTAT:
				if (!nl) putchar('\n');
				cnt = 0;
			}

			switch(i)
			{
			case COM_MENU:
				colPrintf("~FTMenu navigation:\n");
				break;
			case COM_DIM:
				colPrintf("~FTLCD dimming:\n");
				break;
			case COM_FILTER:
				colPrintf("~FTFilter:\n");
				break;
			case COM_ALBUM:
				colPrintf("~FTContent:\n");
				break;
			case COM_ARTDIS:
				colPrintf("~FTArtwork:\n");
				break;
			case COM_SBON:
				colPrintf("~FTStandby:\n");
				break;
			case COM_AUINFO:
				colPrintf("~FTAudio:\n");
				break;
			case COM_APDON:
				colPrintf("~FTAuto power down:\n");
				break;
			case COM_MSV:
				colPrintf("~FTInput select:\n");
				break;
			case COM_DTS:
				colPrintf("~FTNetwork services:\n");
				break;
			case COM_STOP:
				colPrintf("~FTPlayback:\n");
				break;
			case COM_MRMSTAT:
				colPrintf("~FTMiscellanious:\n");
				break;
			}
		}

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
		if (!(++cnt % nlafter))
		{
			putchar('\n');
			nl = 1;
		}
		else nl = 0;
	}
	if (cnt % nlafter) putchar('\n');

	if (extra)
	{
		colPrintf("\n~FMUsage:~RS [<repeat count>] <client/streamer command>   eg: 3 up\n");
		puts("                                                        con 192.168.0.1");
		puts("       [<repeat count>] <raw streamer command>      eg: 3 OSDUP\n");
	}
	else putchar('\n');
	return OK;
}



void optHelpNotes(void)
{
	colPrintf("\n~BB~FW*** Help notes ***\n\n");
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




int optHelpSorted(char *pat)
{
	int cnt;
	int i;

	if (pat && !isPattern(pat)) return ERR_CMD_FAIL;

	colPrintf("\n~BM~FW*** Sorted client commands ***\n\n");
	for(i=cnt=0;i < NUM_CLIENT_COMS;++i)
	{
		if (!pat || wildMatch(sorted_client_coms[i],pat))
		{
			printf("   %-10s",sorted_client_coms[i]);
			if (!(++cnt % 5)) putchar('\n');
		}
	}
	if (cnt % 5) putchar('\n');

	colPrintf("\n~BB~FW*** Sorted streamer commands ***\n\n");
	for(i=cnt=0;i < NUM_STREAMER_COMS;++i)
	{
		if (!pat || wildMatch(sorted_streamer_coms[i],pat))
		{
			printf("   %-10s",sorted_streamer_coms[i]);
			if (!(++cnt % 5)) putchar('\n');
		}
	}
	if (cnt % 5) putchar('\n');
	putchar('\n');
	return OK;
}




int comConnect(char *param1)
{
	if (flags.cmdfile_running && flags.offline)
	{
		errPrintf("Offline mode set, cannot run connect command.\n");
		return ERR_CMD_FAIL;
	}
	if (tcp_sock)
	{
		if (connect_time)
		{
			errPrintf("Already connected.\n");
			return ERR_CMD_FAIL;
		}
		networkClear();
	}
	/* Formats:
	     connect
	     connect <ipaddr>
	*/
	if (param1) ipaddr = strdup(param1);
	if (networkStart()) return OK;
	networkClear();
	return ERR_CMD_FAIL;
}




int comDisconnect(void)
{
	if (tcp_sock)
	{
		networkClear();
		return OK;
	}
	errNotConnected();
	return ERR_CMD_FAIL;
}




/*** Set connect timeout ***/
int comTimeout(char *param1)
{
	float val;
	if (param1 && isNumber(param1) && (val = atof(param1)) >= 0)
	{
		timeout_secs = val;
		if (!val) puts("Wait for TCP timeout.");
		return 1;
	}
	usagePrintf("timeout <connect timeout secs>\n");
	return 0;
}




/*** WAIT, WAIT_MENU and WAIT_REPEAT ***/
int comWait(int comnum, char *param)
{
	float secs = 0;
	u_int usecs;

	/* COM_WAIT_MENU can also take a timeout just in case we never receive
	   a menu. If the timeout expires it errors. */
	if (param)
	{
		if ((secs = atof(param)) < 0) goto USAGE;
		if (comnum == COM_WAIT_REPEAT)
		{
			repeat_wait_secs = secs;
			return OK;
		}
	}
	else if (comnum != COM_WAIT_MENU) goto USAGE;

	if (tcp_sock) return doWait(comnum,secs);

	/* We're not connected to anything */
	if (comnum == COM_WAIT_MENU)
	{
		errNotConnected();
		return ERR_CMD_FAIL;
	}
	usecs = (u_int)(secs * 1000000);
	return (usleep(usecs) == -1 && errno == EINTR) ? ERR_CMD_FAIL : OK;

	USAGE:
	switch(comnum)
	{
	case COM_WAIT:
		usagePrintf("wait <seconds>\n");
		break;
	case COM_WAIT_REPEAT:
		usagePrintf("wait_rep <seconds>\n");
		break;
	case COM_WAIT_MENU:
		usagePrintf("wait_menu [<seconds>]\n");
		break;
	default:
		assert(0);	
	}
	return ERR_CMD_FAIL;
}




void comEcho(int cmd_word, int word_cnt, char **words)
{
	int i;

	for(i=cmd_word+1;i < word_cnt;++i) colPrintf("%s ",words[i]);

	/* Sends an ansi reset code along with nl */
	colPrintf("\n"); 
}




int comRun(char *param)
{
	if (param) return runCommandFile(param);
	usagePrintf("run <command file>\n");
	return ERR_CMD_FAIL;
}




int comOnError(char *param1, char *param2)
{
	if (!param1) goto USAGE;
	if (param2)
	{
		if (strcmp(param1,"skip") || !isNumber(param2)) goto USAGE;
		on_error_skip_set = atoi(param2);
		on_error_skip_cnt = 0;
		flags.on_error_halt = 0;
		return OK;		
	}
 	if (!strcmp(param1,"halt"))
	{
		flags.on_error_halt = 1;
		on_error_skip_cnt = 0;
		on_error_skip_set = 0;
		return OK;
	}
	if (!strcmp(param1,"cont"))
	{
		flags.on_error_halt = 0;
		on_error_skip_cnt = 0;
		on_error_skip_set = 0;
		return OK;
	}
	if (!strcmp(param1,"print"))
	{
		flags.on_error_print = 1;
		return OK;
	}
	if (!strcmp(param1,"noprint"))
	{
		flags.on_error_print = 0;
		return OK;
	}
	
	USAGE:
	usagePrintf("on_error halt\n"
	            "                cont\n"
	            "                print\n"
	            "                noprint\n"
	            "                skip <line count>\n");
	colPrintf("~FYNote:~RS halt, cont and skip will reset each others settings.\n");
	return ERR_CMD_FAIL;
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
			goto DONE;
		case 1:
			ret = optMacroAppend(cmd_word,word_cnt,words);
			goto DONE;
		case 2:
			if (name) return deleteMacro(name);
			goto USAGE;
		case 3:
			if (name)
			{
				ret = runMacro(name);
				return ret;
			}
			goto USAGE;
		case 4:
			if (name) return loadMacros(name);
			goto USAGE;
		case 5:
			ret = optMacroSave(0,cmd_word,word_cnt,words);
			goto DONE;
		case 6:
			ret = optMacroSave(1,cmd_word,word_cnt,words);
			goto DONE;
		case 7:
			listMacros();
			return OK;
		}
	}
	DONE:
	if (ret != ERR_CMD_FAIL) return ret;

	USAGE:
	usagePrintf("macro define <macro name>   [\"<macro command list>\"]\n");
	puts("             append <macro name>   [\"<macro command list>\"]");
	puts("             delete <macro name>/*");
	puts("             run    <macro name>");
	puts("             load   <filename>");
	puts("             save   <filename>     [<macro name>]  : Overwrite");
	puts("             sava   <filename>     [<macro name>]  : Append");
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
			errPrintf("File \"%s\" already exists.\n",filename);
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

void printFlagTrackTime(void)
{
	printf("Show track time      : ");
	PRINT_ON_OFF(flags.show_track_time);
}




void printFlagPromptTime(void)
{
	printf("Update prompt time   : ");
	PRINT_ON_OFF(flags.update_prompt_time);
}




void printFlagHTML(void)
{
	printf("Translate HTML codes : ");
	PRINT_ON_OFF(flags.trans_html_amps);
}




void printFlagColour(void)
{
	printf("Ansi colour          : ");
	PRINT_ON_OFF(flags.use_colour);
}




void printFlagVerb(void)
{
	printf("Verbose output       : ");
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
void sortCommands(void)
{
	int i;

	for(i=0;i < NUM_CLIENT_COMS;++i)
		sorted_client_coms[i] = commands[i].com;
	qsort(sorted_client_coms,NUM_CLIENT_COMS,sizeof(char *),compareComs);

	for(i=0;i < NUM_STREAMER_COMS;++i)
		sorted_streamer_coms[i] = commands[FIRST_STREAMER_COM+i].com;
	qsort(sorted_streamer_coms,NUM_STREAMER_COMS,sizeof(char *),compareComs);
}




void clearHistory(void)
{
	int i;
	for(i=0;i < MAX_HIST_BUFFERS;++i) clearBuffer(i);
	colPrintf("History ~FGcleared.\n");
}




char *bytesSizeStr(u_long bytes)
{
	static char str[20];

	/* Only start printing in kilobytes from 10000 as eg 2345 bytes is 
	   still easy to read */
	if (bytes < 1e4) 
		snprintf(str,sizeof(str),"%lu bytes",bytes);
	else if (bytes < 1e6)
		snprintf(str,sizeof(str),"%.1fK",(double)bytes / 1e3);
	else 
		snprintf(str,sizeof(str),"%.1fM",(double)bytes / 1e6);
	return str;
}
