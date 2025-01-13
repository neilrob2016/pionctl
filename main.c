/***************************************************************************** 
 PIONCTL

 A command line control client for the Pioneer N-70AE streamer using the 
 Onkyo ISCP protocol.

 Initial non public version December 2019
*****************************************************************************/
#define MAINFILE
#include "globals.h"

#define DEVICE_CODE '1'

char *cmd_file;
char *cmd_list;

void comSanityCheck(void);
void parseCmdLine(int argc, char **argv);
void init(void);
void mainloop(void);
void runImmediate(void);
void runReset(void);

int main(int argc, char **argv)
{
	comSanityCheck();
	parseCmdLine(argc,argv);
	version(1);
	init();
	mainloop();
	return 0;
}




/*** Only required to check after updating commands array and enum ***/
void comSanityCheck(void)
{
	/*
	for(int i=0;i < NUM_COMMANDS;++i)
		printf("%-3d: %s\n",i,commands[i].com);
	*/
	assert(NUM_COMMANDS == 123);
	assert(LAST_CLIENT_COM == COM_ON_ERROR);
	assert(FIRST_STREAMER_COM == COM_MENU);
	assert(!strcmp(commands[COM_MENU].com,"menu"));
	assert(!strcmp(commands[COM_FILTER].com,"filter"));
	assert(!strcmp(commands[COM_LRA].com,"lra"));
	assert(!strcmp(commands[COM_APDON].com,"apdon"));
	assert(!strcmp(commands[COM_MSV].com,"msv"));
	assert(!strcmp(commands[COM_DTS].com,"dts"));
	assert(!strcmp(commands[COM_TIDALVER].com,"tidalver"));
	assert(!strcmp(commands[COM_STOP].com,"stop"));
	assert(!strcmp(commands[COM_SEEK].com,"seek"));
	assert(!strcmp(commands[COM_MRMSTAT].com,"mrmstat"));
	assert(!strcmp(commands[COM_SETNAME].com,"setname"));
	assert(!strcmp(commands[COM_SETUP].com,"setup"));
	assert(!strcmp(commands[COM_PRODID].com,"prodid"));
}




void parseCmdLine(int argc, char **argv)
{
	char c;
	int up;
	int i;

	ipaddr = NULL;
	udp_port = UDP_PORT;
	tcp_port = TCP_PORT;
	device_code = DEVICE_CODE;
	connect_timeout = CONNECT_TIMEOUT;
	cmd_file = NULL;
	cmd_list = NULL;
	prompt_type = PROMPT_NAME;
	raw_level = 0;
	bzero(&flags,sizeof(flags));
	flags.use_colour = 1;
	flags.run_rc_file = 1;

	for(i=1;i < argc;++i)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) goto USAGE;

		switch((c = argv[i][1]))
		{
		case 'b':
			flags.verbose = 1;
			continue;
		case 'c':
			flags.use_colour = 0;
			continue;
		case 'e':
			flags.exit_after_cmds = 1;
			continue;
		case 'o':
			flags.offline = 1;
			continue;
		case 'r':
			flags.run_rc_file = 0;
			continue;
		case 's':
			flags.trans_html_amps = 1;
			continue;
		case 't':
			flags.show_track_time = 1;
			continue;
		case 'v':
			version(0);
			exit(0);
			continue;
		}
 		if (i == argc - 1) goto USAGE;

		switch(argv[i][1])
		{
		case 'a':
			ipaddr = strdup(argv[++i]);
			break;
		case 'd':
			if (strlen(argv[++i]) != 1) goto USAGE;
			device_code = argv[i][0];
			break;
		case 'f':
			cmd_file = argv[++i];
			break;
		case 'i':
			cmd_list = argv[++i];
			break;
		case 'n':
			++i;
			if (!isNumber(argv[i]) ||
			    (connect_timeout = atoi(argv[i])) < 0) goto USAGE;
			break;
		case 'p':
			prompt_type = atoi(argv[++i]);
			if (!isNumber(argv[i]) ||
			    prompt_type < 0 || prompt_type >= NUM_PROMPTS)
			{
				goto USAGE;
			}
			continue;
		case 'r':
			raw_level = atoi(argv[++i]);
			if (!isNumber(argv[i]) ||
			    raw_level < 0 || raw_level >= NUM_RAW_LEVELS)
			{
				goto USAGE;
			}
			continue;
		case 'u':
			up = atoi(argv[++i]);
			/* Comparison wouldn't work if we used uint16_t */
			if (!isNumber(argv[i]) || up < 0 || up > 65535)
			{
				goto USAGE;
			}
			udp_port = (uint16_t)up;
			break;
		default:
			goto USAGE;
		}
	}
	if (ipaddr && flags.offline)
	{
		puts("ERROR: The -a and -o options are mutually exclusive.");
		exit(1);
	}
	if (flags.exit_after_cmds && !cmd_list && !cmd_file)
	{
		puts("ERROR: The -e option requires -f or -i.");
		exit(1);
	}
	return;
	
	USAGE:
	printf("Usage: %s\n"
	       "       -a <TCP address[:<port>]>\n"
	       "       -n <TCP/UDP timeout>     : For TCP connect and UDP EZPROXY listen.\n"
	       "                                  Default = %d seconds.\n"
	       "       -u <UDP listen port>     : 0 to 65535. Default = %d.\n"
	       "       -d <device code (0-9)>   : Default = %c.\n"
	       "       -f <command file>        : File of commands to run immediately after\n"
	       "                                  connect. Takes precendence over -i.\n"
	       "       -i <command list>        : Semi colon seperated list of commands to run\n"
	       "                                  after connection. Eg: tunein;3 dn;en\n"
	       "       -p [0 to %d]              : Prompt type. Default = %d.\n"
	       "       -r [0 to %d]              : Raw RX/TX print level. Default = %d.\n"
	       "       -b                       : Verbose mode (currently only shows macros run\n"
	       "                                  and expanded command names).\n"
	       "       -c                       : Do NOT use ansi colour.\n"
	       "       -e                       : Exit after commands run via -f or -i.\n"
	       "       -o                       : Offline mode, ie don't listen for streamer.\n"
	       "       -r                       : Do NOT run %s (if it exists) at startup.\n"
	       "       -s                       : Translate HTML ampersand codes in album,\n"
	       "                                  artist and title when pretty printing.\n"
	       "       -t                       : Print NTM track time info.\n"
	       "       -v                       : Print version then exit.\n"
	       "Notes:\n"
	       " - All arguments are optional.\n"
	       " - If the -a option or a %s or other command file with a connect command\n"
	       "   is not given with -f then the streamer address is obtained by listening for\n"
	       "   an EZProxy UDP packet.\n"
	       " - The -a and -o options are mutually exclusive. If -o is used with a command\n"
	       "   file at startup then any connect commands in the file will be ignored.\n"
	       " - The -f and -i commands are run *after* auto connection is complete. If you\n"
	       "   want any commands to run before connection put them in the %s file.\n",
			argv[0],
			CONNECT_TIMEOUT,
			UDP_PORT,
			DEVICE_CODE,
			NUM_PROMPTS-1,
			prompt_type,
			NUM_RAW_LEVELS-1,
			raw_level,
			RC_FILENAME,
			RC_FILENAME,
			RC_FILENAME);
	exit(1);
}




void init(void)
{
	int ret = OK;

	initBuffers();
	initKeyboard();
	initTitles();
	initMenu();
	initSave();
	initMacros();
	initReverse();
	sortCommands();
	menu_cursor_pos = -1;
	input_state = INPUT_CMD;
	macro_append = -1;
	start_time = time(0);
	nri_command = 0;
	runReset();

	signal(SIGINT,sigHandler);
	signal(SIGQUIT,sigHandler);
	signal(SIGTERM,sigHandler);

	if (flags.run_rc_file) runCommandFile(NULL);

	if (flags.offline) puts("Offline mode.\n");
	else if (flags.tried_connect)
	{
		if (!connect_time)
			puts("Connect already tried and failed, not attempting streamer listen.");
	}
	else if (!networkStart())
	{
		if (!flags.interrupted) doExit(1);
		flags.interrupted = 0;
	}
	strcpy(track_time_str,TIME_DEF_STR);
	strcpy(track_len_str,TIME_DEF_STR);

	if (cmd_file) ret = runCommandFile(cmd_file);

	/* Reset so connects in immediate and further command files won't be 
	   ignored */
	flags.offline = 0;

	if (cmd_list)
	{
		if (ret == OK)
			runImmediate();
		else
			warnPrintf("Not running immediate commands due to command file failing.\n");
	}
	if (flags.exit_after_cmds)
	{
		quitPrintf("after file/immediate commands");
		doExit(0);
	}

	printPrompt();
}




void mainloop(void)
{
	struct timeval tvs;
	struct timeval *tvp;
	fd_set mask;

	while(1)
	{
		runReset();

		FD_ZERO(&mask);
		FD_SET(STDIN_FILENO,&mask);
		if (tcp_sock) FD_SET(tcp_sock,&mask);
		if (save_state != SAVE_INACTIVE)
		{
			tvs.tv_sec = SAVE_TIMEOUT;
			tvs.tv_usec = 0;
			tvp = &tvs;
		}
		else tvp = NULL;

		switch(select(FD_SETSIZE,&mask,0,0,tvp))
		{
		case -1:
			if (flags.interrupted) continue;
			errPrintf("mainLoop(): select(): %s\n",strerror(errno));
			doExit(1);
			/* Avoids gcc warning */
			break; 
		case 0:
			/* Timeout */
			checkSaveTimeout();
			continue;
		}
		if (tcp_sock && FD_ISSET(tcp_sock,&mask)) readSocket(1);
		if (FD_ISSET(STDIN_FILENO,&mask)) readKeyboard();
		checkSaveTimeout();
	}
}




void runImmediate(void)
{
	int len = strlen(cmd_list);
	int ret;
	colPrintf("~FMRunning immediate commands:~RS \"%s\"\n",cmd_list);
	ret = parseInputLine(cmd_list,len);
	colPrintf("Immediate command run %s\n",
		ret == OK ? "~FGOK" : "~FRFAILED");
	printPrompt();
}




void runReset(void)
{
	flags.interrupted = 0;
	flags.do_halt = 0;
	flags.do_return = 0;
	flags.on_error_halt = 1;
	flags.on_error_print = 1;
	on_error_skip_set = 0;
	on_error_skip_cnt = 0;
}
