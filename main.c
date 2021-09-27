/***************************************************************************** 
 PIONCTL

 Connects to the control port of the Pioneer N70 streamer. Unfortunately
 the server on the Pioneer is not well written - sometimes it doesn't reply,
 sometimes sends the same thing 2 or 3 times.

 TODO:
 - Use NLA to navigate menus better? See bottom of NRI section in doc.
 - Find command for Now Playing remote control button functionality.

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
		case 'r':
			SETFLAG(FLAG_SHOW_RAW);
			continue;
		case 's':
			SETFLAG(FLAG_SHOW_DETAIL);
			continue;
		case 't':
			SETFLAG(FLAG_SHOW_TIMER);
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
			if ((listen_timeout = atoi(argv[++i])) < 0) goto USAGE;
			break;
		case 'n':
			if ((connect_timeout = atoi(argv[++i])) < 0) goto USAGE;
			break;
		case 'p':
			prompt_type = atoi(argv[++i]);
			if (prompt_type < 0 || prompt_type >= NUM_PROMPTS)
				goto USAGE;
			continue;
		case 'u':
			up = atoi(argv[++i]);
			/* Comparison wouldn't work if we used uint16_t */
			if (up < 0 || up > 65535)
			{
				printf("ERROR: Invalid UDP port %d. Must be from 0 to 65535.\n",up);
				exit(1);
			}
			udp_port = (uint16_t)up;
			break;
		default:
			goto USAGE;
		}
	}
	if (ipaddr && (flags & FLAG_OFFLINE))
	{
		puts("ERROR: The -a and -o options are mutually exclusive.");
		exit(1);
	}
	if ((flags & FLAG_TRANS_HTML_AMPS) && (flags & FLAG_SHOW_RAW))
	{
		puts("ERROR: The -c and -r options are mutually exclusive.");
		exit(1);
	}
	if ((flags & FLAG_EXIT_AFTER_CMDS) && !cmd_list)
	{
		puts("ERROR: The -e option requires -i.");
		exit(1);
	}
	return;
	
	USAGE:
	printf("Usage: %s\n"
	       "       -a <TCP address[:<port>]>\n"
	       "       -n <TCP connect timeout> : Default = TCP default\n"
	       "       -u <UDP listen port>     : Default = %d.\n"
	       "       -l <UDP listen timeout>] : Default = %d secs.\n"
	       "       -d <device code (0-9)>]  : Default = %c.\n"
	       "       -i <command list>        : CSV list of commands to run immediately.\n"
	       "       -p [0/1/2/3]             : 0 = bare prompt, 1 = show connect timer\n"
	       "                                  2 = show streamer timer, 3 = show both.\n"
	       "                                  Default = %d.\n"
	       "       -e                       : Exit after immediate commands run.\n"
	       "       -o                       : Offline mode - don't listen for streamer.\n"
	       "       -c                       : Translate HTML ampersand codes in album,\n"
	       "                                  artist and title when pretty printing.\n"
	       "       -r                       : Show raw RX from streamer, don't prettify.\n"
	       "       -s                       : Show field info. If -p enabled then show\n"
	       "                                  raw messages.\n"
	       "       -t                       : Print NTM service timer messages.\n"
	       "       -v                       : Print version then exit.\n"
	       "All arguments are optional. If the -a option is not used then the streamer\n"
	       "address is obtained by listening for an EZProxy UDP packet unless -o is.\n"
	       "specified.\n",
		argv[0],UDP_PORT,LISTEN_TIMEOUT,DEVICE_CODE,prompt_type);
	exit(1);
}




void init()
{
	initBuffers();
	initKeyboard();
	initTitles();
	initMenu();
	initSave();
	sortCommands();
	menu_cursor_pos = -1;
	menu_selection = NULL;

	signal(SIGINT,sigHandler);
	signal(SIGQUIT,sigHandler);
	connect_time = 0;

	if (flags & FLAG_OFFLINE)
		puts("Offline mode.\n");
	else
	{
		if (!networkStart()) doExit(1);
	}
	strcpy(timer_str,TIMER_STR_DEF);
	printPrompt();
}




void mainloop()
{
	fd_set mask;

	if (cmd_list) runImmediate();

	while(1)
	{
		FD_ZERO(&mask);
		FD_SET(STDIN,&mask);
		if (tcp_sock) FD_SET(tcp_sock,&mask);

		switch(select(FD_SETSIZE,&mask,0,0,0))
		{
		case -1:
			perror("ERROR: mainLoop(): select()");
			doExit(1);
		case 0:
			/* Timeout, should never happen */
			puts("ERROR: mainLoop(): select() timeout");
			continue;
		}
		if (tcp_sock && FD_ISSET(tcp_sock,&mask)) readSocket();
		if (FD_ISSET(STDIN,&mask)) readKeyboard();
	}
}




void runImmediate()
{
	printPrompt();
	puts(cmd_list);
	addToBuffer(keyb_buffnum,cmd_list,strlen(cmd_list));
	parseUserInput();
	printPrompt();

	if (flags & FLAG_EXIT_AFTER_CMDS)
	{
		puts("\n*** EXIT after immediate commands ***");
		doExit(0);
	}
}
