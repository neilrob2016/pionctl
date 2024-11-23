/***************************************************************************** 
 PIONCTL

 A command line control client for the Pioneer N-70AE streamer using the 
 Onkyo ISCP protocol.

 Initial non public version December 2019
*****************************************************************************/
#define MAINFILE
#include "globals.h"

#define DEVICE_CODE    '1'
#define LISTEN_TIMEOUT 60

char *cmd_list;

void comCheck(void);
void parseCmdLine(int argc, char **argv);
void init(void);
void mainloop(void);
void runImmediate(void);

int main(int argc, char **argv)
{
	comCheck();
	parseCmdLine(argc,argv);
	version(1);
	init();
	mainloop();
	return 0;
}



/*** Only required to check after updating commands array and enum ***/
void comCheck(void)
{
	assert(LAST_CLIENT_COM == 14);
	assert(FIRST_STREAMER_COM == 15);
	assert(!strcmp(commands[COM_FILTER].com,"filter"));
	assert(!strcmp(commands[COM_SETNAME].com,"setname"));
	assert(!strcmp(commands[COM_SETUP].com,"setup"));
	assert(!strcmp(commands[COM_LRA].com,"lra"));
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
	listen_timeout = LISTEN_TIMEOUT;
	connect_timeout = 0;
	cmd_list = NULL;
	prompt_type = PROMPT_NAME;
	raw_level = 0;
	bzero(&flags,sizeof(flags));
	flags.use_colour = 1;

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
		case 'i':
			cmd_list = argv[++i];
			break;
		case 'l':
			++i;
			if (!isNumber(argv[i]) ||
			    (listen_timeout = atoi(argv[i])) < 0) goto USAGE;
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
	if (flags.exit_after_cmds && !cmd_list)
	{
		puts("ERROR: The -e option requires -i.");
		exit(1);
	}
	return;
	
	USAGE:
	printf("Usage: %s\n"
	       "       -a <TCP address[:<port>]>\n"
	       "       -n <TCP connect timeout> : Default = TCP default\n"
	       "       -u <UDP listen port>     : 0 to 65535. Default = %d.\n"
	       "       -l <UDP listen timeout>  : Default = %d secs.\n"
	       "       -d <device code (0-9)>   : Default = %c.\n"
	       "       -i <command list>        : Semi colon seperated list of commands to run\n"
	       "                                  immediately. Eg: tunein;3 dn;en\n"
	       "       -p [0 to %d]              : Prompt type. Default = %d.\n"
	       "       -r [0 to %d]              : Raw RX/TX print level. Default = %d.\n"
	       "       -b                       : Verbose mode (currently only shows macros run\n"
	       "                                  and expanded command names).\n"
	       "       -c                       : Do NOT use ansi colour.\n"
	       "       -e                       : Exit after immediate commands run using -i.\n"
	       "       -o                       : Offline mode, ie don't listen for streamer.\n"
	       "       -s                       : Translate HTML ampersand codes in album,\n"
	       "                                  artist and title when pretty printing.\n"
	       "       -t                       : Print NTM track time info.\n"
	       "       -v                       : Print version then exit.\n"
	       "All arguments are optional. If the -a option is not used then the streamer\n"
	       "address is obtained by listening for an EZProxy UDP packet unless -o is.\n"
	       "specified.\n",
		argv[0],
		UDP_PORT,LISTEN_TIMEOUT,DEVICE_CODE,
		NUM_PROMPTS-1,prompt_type,
		NUM_RAW_LEVELS-1,raw_level);
	exit(1);
}




void init(void)
{
	initBuffers();
	initKeyboard();
	initTitles();
	initMenu();
	initSave();
	initMacros();
	initReverse();
	sortCommands();
	menu_cursor_pos = -1;
	menu_selection = NULL;
	input_state = INPUT_CMD;
	macro_append = -1;
	connect_time = 0;
	start_time = time(0);
	nri_command = 0;

	signal(SIGINT,sigHandler);
	signal(SIGQUIT,sigHandler);
	signal(SIGTERM,sigHandler);

	if (flags.offline) puts("Offline mode.\n");
	else if (!networkStart())
	{
		if (!flags.interrupted) doExit(1);
		flags.interrupted = 0;
	}
	strcpy(track_time_str,TIME_DEF_STR);
	strcpy(track_len_str,TIME_DEF_STR);
	printPrompt();
}




void mainloop(void)
{
	struct timeval tvs;
	struct timeval *tvp;
	fd_set mask;

	if (cmd_list) runImmediate();

	while(1)
	{
		flags.interrupted = 0;
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
	printPrompt();
	puts(cmd_list);
	addToBuffer(keyb_buffnum,cmd_list,strlen(cmd_list));
	parseInputLine(buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
	printPrompt();

	if (flags.exit_after_cmds)
	{
		quitPrintf("after immediate commands");
		doExit(0);
	}
}
