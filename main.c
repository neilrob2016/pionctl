/***************************************************************************** 
 PIONCTL

 A command line control client for the Pioneer N-70AE streamer using the 
 Onkyo ISCP protocol.

 Initial version December 2019
*****************************************************************************/
#define MAINFILE
#include "globals.h"

char *cmd_list;

void parseCmdLine(int argc, char **argv);
void init();
void mainloop();
void runImmediate();

int main(int argc, char **argv)
{
	parseCmdLine(argc,argv);
	version(1);
	init();
	mainloop();
	return 0;
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
	flags = 0;
	cmd_list = NULL;
	prompt_type = PROMPT_BASE;
	raw_level = 0;

	for(i=1;i < argc;++i)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) goto USAGE;

		switch((c = argv[i][1]))
		{
		case 'c':
			SETFLAG(FLAG_TRANS_HTML_AMPS);
			continue;
		case 'e':
			SETFLAG(FLAG_EXIT_AFTER_CMDS);
			continue;
		case 'o':
			SETFLAG(FLAG_OFFLINE);
			continue;
		case 't':
			SETFLAG(FLAG_SHOW_SVC_TIME);
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
	if (ipaddr && FLAGISSET(FLAG_OFFLINE))
	{
		puts("ERROR: The -a and -o options are mutually exclusive.");
		exit(1);
	}
	if (FLAGISSET(FLAG_EXIT_AFTER_CMDS) && !cmd_list)
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
	       "       -l <UDP listen timeout>] : Default = %d secs.\n"
	       "       -d <device code (0-9)>]  : Default = %c.\n"
	       "       -i <command list>        : Semi colon seperated list of commands to run\n"
	       "                                  immediately. Eg: tunein;3 dn;en\n"
	       "       -p [0 to %d]              : Prompt type. Default = %d.\n"
	       "       -r [0 to %d]              : Raw RX/TX print level. Default = %d.\n"
	       "       -e                       : Exit after immediate commands run.\n"
	       "       -o                       : Offline mode - don't listen for streamer.\n"
	       "       -c                       : Translate HTML ampersand codes in album,\n"
	       "                                  artist and title when pretty printing.\n"
	       "       -t                       : Print NTM service time messages.\n"
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




void init()
{
	initBuffers();
	initKeyboard();
	initTitles();
	initMenu();
	initSave();
	initMacros();
	sortCommands();
	menu_cursor_pos = -1;
	menu_selection = NULL;
	input_state = INPUT_CMD;
	macro_append = -1;

	signal(SIGINT,sigHandler);
	signal(SIGQUIT,sigHandler);
	signal(SIGTERM,sigHandler);
	connect_time = 0;

	if (FLAGISSET(FLAG_OFFLINE)) puts("Offline mode.\n");
	else if (!networkStart())
	{
		if (!FLAGISSET(FLAG_INTERRUPTED)) doExit(1);
		UNSETFLAG(FLAG_INTERRUPTED);
	}
	strcpy(svc_time_str,SVC_TIME_DEF);
	printPrompt();
}




void mainloop()
{
	fd_set mask;

	if (cmd_list) runImmediate();

	while(1)
	{
		UNSETFLAG(FLAG_INTERRUPTED);
		FD_ZERO(&mask);
		FD_SET(STDIN,&mask);
		if (tcp_sock) FD_SET(tcp_sock,&mask);

		switch(select(FD_SETSIZE,&mask,0,0,0))
		{
		case -1:
			if (FLAGISSET(FLAG_INTERRUPTED)) continue;
			perror("ERROR: mainLoop(): select()");
			doExit(1);
		case 0:
			/* Timeout, should never happen */
			puts("ERROR: mainLoop(): select() timeout");
			continue;
		}
		if (tcp_sock && FD_ISSET(tcp_sock,&mask)) readSocket(1);
		if (FD_ISSET(STDIN,&mask)) readKeyboard();
	}
}




void runImmediate()
{
	printPrompt();
	puts(cmd_list);
	addToBuffer(keyb_buffnum,cmd_list,strlen(cmd_list));
	parseInputLine(buffer[keyb_buffnum].data,buffer[keyb_buffnum].len);
	printPrompt();

	if (FLAGISSET(FLAG_EXIT_AFTER_CMDS))
	{
		puts("\n*** EXIT after immediate commands ***");
		doExit(0);
	}
}
