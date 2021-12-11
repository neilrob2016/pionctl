#include "globals.h"

#define PRINT_ON_OFF() \
	if (!memcmp(mesg,"00",2)) puts("OFF"); \
	else if (!memcmp(mesg,"01",2)) puts("ON"); \
	else printMesg(mesg,len);

/* Forward declarations */
void printAMT(u_char *mesg, uint32_t len);
void printDGF(u_char *mesg, uint32_t len);
void printDIR(u_char *mesg, uint32_t len);
void printEDF(u_char *mesg, uint32_t len);
void printEDV(u_char *mesg, uint32_t len);
void printIFA(u_char *mesg, uint32_t len);
void printFWV(u_char *mesg, uint32_t len);
void printHBT(u_char *mesg, uint32_t len);
void printMDI(u_char *mesg, uint32_t len);
void printMOT(u_char *mesg, uint32_t len);
void printPWR(u_char *mesg, uint32_t len);
void printAPD(u_char *mesg, uint32_t len);
void printNAL(u_char *mesg, uint32_t len);
void printNAT(u_char *mesg, uint32_t len);
void printNCP(u_char *mesg, uint32_t len);
void printNDN(u_char *mesg, uint32_t len);
void printNDS(u_char *mesg, uint32_t len);
void printNFI(u_char *mesg, uint32_t len);
void printNFN(u_char *mesg, uint32_t len);
void printNJA(u_char *mesg, uint32_t len);
void printNLS(u_char *mesg, uint32_t len);
void printNMS(u_char *mesg, uint32_t len);
void printNLT(u_char *mesg, uint32_t len);
void printNRI(u_char *mesg, uint32_t len);
void printNSB(u_char *mesg, uint32_t len);
void printNST(u_char *mesg, uint32_t len);
void printNTI(u_char *mesg, uint32_t len);
void printNTM(u_char *mesg, uint32_t len);
void printNTR(u_char *mesg, uint32_t len);
void printNLU(u_char *mesg, uint32_t len);
void printSLI(u_char *mesg, uint32_t len);
void printUPD(u_char *mesg, uint32_t len);
void printUPS(u_char *mesg, uint32_t len);
void printPPS(u_char *mesg, uint32_t len);
void printDIM(u_char *mesg, uint32_t len);
void printLRA(u_char *mesg, uint32_t len);
void printMGV(u_char *mesg, uint32_t len);
void printMRM(u_char *mesg, uint32_t len);
void printTranslatedMesg(u_char *mesg, uint32_t len, int add_title);
u_char *replaceAmpCodes(u_char *str, uint32_t *len);
u_char translateAmpCode(char *code);
long hexToInt(u_char *str, int len);

/* Map of 3 character server commands/replies to functions ***/
static struct st_comfunc
{
	char *com;
	void (*func)(u_char *mesg, uint32_t);
} comfunc[] =
{
	/* 0 */
	{ "AMT", printAMT },
	{ "DGF", printDGF },
	{ "DIR", printDIR },
	{ "EDF", printEDF },
	{ "EDV", printEDV },

	/* 5 */
	{ "FWV", printFWV },
	{ "HBT", printHBT },
	{ "IFA", printIFA },
	{ "MDI", printMDI },
	{ "MOT", printMOT },

	/* 10 */
	{ "NAL", printNAL },
	{ "NAT", printNAT },
	{ "NCP", printNCP },
	{ "NDN", printNDN },
	{ "NDS", printNDS },

	/* 15 */
	{ "NFI", printNFI },
	{ "NFN", printNFN },
	{ "NJA", printNJA },
	{ "NLS", printNLS },
	{ "NLT", printNLT },

	/* 20 */
	{ "NMS", printNMS },
	{ "NRI", printNRI },
	{ "NSB", printNSB },
	{ "NST", printNST },
	{ "NTI", printNTI },

	/* 25 */
	{ "NTM", printNTM },
	{ "NTR", printNTR },
	{ "NLU", printNLU },
	{ "PWR", printPWR },
	{ "APD", printAPD },

	/* 30 */
	{ "SLI", printSLI },
	{ "UPD", printUPD },
	{ "UPS", printUPS },
	{ "PPS", printPPS },
	{ "DIM", printDIM },

	/* 35 */
	{ "LRA", printLRA },
	{ "MGV", printMGV },
	{ "MRM", printMRM },
	{ "", NULL }
};

#define RX_COM_NJA 17
#define RX_COM_NLS 18

/***************************** External functions ****************************/

/*** Prints out message buffer data translating non printing characters ***/
void printMesg(u_char *mesg, int len)
{
	int i;
	for(i=0;i < len;++i) putchar(mesg[i] < 32 ?  '.' : mesg[i]);
	putchar('\n');
}




/*** Call pretty print function for the command in the raw data from the 
     streamer. Called from readSocket() ***/
void prettyPrint(t_iscp_data *pkt_data, int print_prompt)
{
	int mesg_len = pkt_hdr->data_len - (MESG_OFFSET + 3);
	int i;

	clearPrompt();

	/* Look for matching key in print function list */
	for(i=0;comfunc[i].func;++i)
	{
		if (!strncmp((char *)pkt_data->command,comfunc[i].com,3))
		{
			if (i != RX_COM_NJA) nja_prev = 0;
			if (i != RX_COM_NLS && FLAGISSET(FLAG_IN_MENU))
				flags ^= FLAG_IN_MENU;

			/* Call print function */
			comfunc[i].func(pkt_data->mesg,mesg_len);
			break;
		}
	}

	if (!comfunc[i].func)
	{
		printf("Unknown response: %.3s",pkt_data->command);
		printMesg(pkt_data->mesg,mesg_len);
		setUnknownKey((char *)pkt_data->command);
	}

	if (print_prompt) printPrompt();
	clearBuffer(BUFF_TCP);
}




/*** Pretty print the RX commands and data stored in the list ***/
int prettyPrintList(u_char *pat, int max)
{
	t_entry *entry;
	u_char *value;
	int total;
	int len;
	int cnt;
	int i;
	int j;

	if (max < 0)
	{
		puts("Usage: show [<pattern> [<count>]]");
		return ERR_CMD_FAIL;
	}

	printf("\n*** Processed streamer RX (%s : %sCONNECTED) ***\n\n",
		inet_ntoa(con_addr.sin_addr),tcp_sock ? "" : "DIS");

	SETFLAG(FLAG_PRETTY_PRINTING);
	for(i=cnt=total=0;i < 256;++i)
	{
		for(entry=list[i];entry;entry=entry->next)
		{
			if (entry->unknown) continue;
			++total;

			/* Just to be on the safe side */
			if (entry->value)
			{
				value = entry->value;
				len = entry->val_len;
			}
			else
			{
				value = (u_char *)"";
				len = 0;
			}

			/* Continue looping anyway to get correct total */
			if (max && cnt == max) continue;

			/* Match on raw command and value */
			if (pat && 
			    !wildMatch(entry->key,(char *)pat) &&
			    !wildMatch((char *)value,(char *)pat))
			{
				continue;
			}

			for(j=0;comfunc[j].func;++j)
			{
				if (!strcmp(entry->key,comfunc[j].com))
				{
					comfunc[j].func(value,len);
					++cnt;
					break;
				}
			}
		}
	}

	if (pat || max)
		printf("\n%d of %d entries.\n\n",cnt,total);
	else
		printf("\n%d entries.\n\n",cnt);
	flags ^= FLAG_PRETTY_PRINTING;

	return OK;
}


/****************************** Print functions ****************************/

void printAMT(u_char *mesg, uint32_t len)
{
	printf("Muting: ");
	PRINT_ON_OFF();
}




/*** Digital filter ***/
void printDGF(u_char *mesg, uint32_t len)
{
	printf("Filter: ");

	/* Values are 00, 01, 02 */
	if (mesg[0] == '0')
	{
		switch(mesg[1])
		{
		case '0': puts("SLOW");  return;
		case '1': puts("SHARP"); return;
		case '2': puts("SHORT"); return;
		}
	}
	printMesg(mesg,len);
}




void printDIR(u_char *mesg, uint32_t len)
{
	printf("Direct: ");
	PRINT_ON_OFF();
}




void printEDF(u_char *mesg, uint32_t len)
{
	printf("Memory: ");
	if (len < 8)
		puts("<unknown>");
	else
		printf("%ld MB free\n",hexToInt(mesg,8));
}




void printEDV(u_char *mesg, uint32_t len)
{
	printf("Action: ");
	if (!memcmp(mesg,"00",2)) puts("Not approved");
	else if (!memcmp(mesg,"01",2)) puts("Approved");
	else printMesg(mesg,len);
}




/*** Message is CSV, eg: NETWORK,,,,Stereo,,,0 ms,Normal, 
     N-70AE only seems to use some of the fields and it doesn't quite match
     with the documentation as PQLS field appears to be missing. ***/
void printIFA(u_char *mesg, uint32_t len)
{
	char *field[10] =
	{
		"Input port",
		"Input signal format",
		"Sampling frequency",
		"Input signal channel",
		"Listening mode",
		"Output signal channel",
		"Output sampling freq",
		"Phase control delay",
		"Phase control phase",
		"Upmix mode"
	};
	char *ptr;
	char *end;
	char c1;
	char c2;
	int i;
	
	c1 = mesg[len];
	mesg[len] = 0;
	puts("Audio info: ");

	/* Not using strtok() because it modifies the string it searches */
	for(i=0,ptr=(char *)mesg-1;i< 10 && ptr;++i,ptr=end)
	{
		++ptr;
		if ((end = (char *)strchr(ptr,SEPARATOR)))
		{
			c2 = *end;
			*end = 0;
		}
		printf("   %-21s: %s\n",field[i],ptr);
		if (end) *end = c2;
	}
	mesg[len] = c1;
}




void printFWV(u_char *mesg, uint32_t len)
{
	printf("Firmware vers: ");
	printMesg(mesg,len);
}




void printHBT(u_char *mesg, uint32_t len)
{
	printf("Hi-bit: ");
	PRINT_ON_OFF();
}




void printMDI(u_char *mesg, uint32_t len)
{
	puts("\n*** Streamer info ***\n");
	printMesg(mesg,len);
}




void printMOT(u_char *mesg, uint32_t len)
{
	printf("ASR   : ");
	PRINT_ON_OFF();
}




void printNAL(u_char *mesg, uint32_t len)
{
	printf("Album : ");
	printTranslatedMesg(mesg,len,0);
}




void printNAT(u_char *mesg, uint32_t len)
{
	printf("Artist: ");
	printTranslatedMesg(mesg,len,0);
}




/*** Usually returned after an NSV command, eg NSV1B0 for Tidal ***/
void printNCP(u_char *mesg, uint32_t len)
{
	const char *popup_type[6] =
	{ 
		"List","Menu","Playback","Popup","Keyboard","Menu list"
	};
	const char *update_type[4] =
	{
		"All","Button","Textbox","Listbox"
	};
	switch(mesg[0])
	{
	case 'X':
		/* Returned for Tidal and Deezer */
		printf("XML popup: ");
		printMesg(mesg+1,len - 1);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		/* Never seen this format returned but here for completeness.
		   Should relly do bounds checking, oh well. */
		printf("%s popup, 0x%.2s layers, ",
			popup_type[mesg[0] - '0'],
			mesg+1);
		switch(mesg[3])
		{
		case '0':
		case '1':
		case '2':
		case '3':
			printf("%s update: ",update_type[mesg[3] - '0']);
			break;
		default:
			printf("type %c update: ",mesg[3]);
			break;
		}
		printMesg(mesg+4,len - 4);
		break;
	default:
		printf("Unknown popup type: ");
		printMesg(mesg,len);
	}
}




/*** NDNQSTN always returns empty string but apparently this is another
     device name command. Perhaps for other models only ***/
void printNDN(u_char *mesg, uint32_t len)
{
	printf("Device: ");
	printMesg(mesg,len);
}




void printNDS(u_char *mesg, uint32_t len)
{
	int i;

	puts("Connector status:");
	printf("   Network  : ");

	switch(mesg[0])
	{
	case '-': printf("None");     break;
	case 'E': printf("Ethernet"); break;
	case 'W': printf("WiFi");     break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[0]);
	
	printf("   Front USB: ");
	for(i=1;i < 3;++i)
	{
		switch(mesg[i])
		{
		case '-': printf("Nothing attached");  break;
		case 'i': printf("iPod/iPhone");       break;
		case 'M': printf("Memory/NAS");        break;
		case 'W': printf("Wireless adaptor");  break;
		case 'B': printf("Bluetooth adaptor"); break;
		case 'x': printf("Disabled");
		}
		printf(" (%c)\n",mesg[i]);
		if (i == 1) printf("   Rear USB : ");
	}
}




void printNFI(u_char *mesg, uint32_t len)
{
	printf("Codec : ");
	printMesg(mesg,len);
}




void printNFN(u_char *mesg, uint32_t len)
{
	puts("Unit info:");
	/* No lookup table in docs */
	printf("   Manufacturer ID: %d\n",mesg[0]); 
	printf("   Friendly name  : ");
	printMesg(mesg+1,len - 1);
}




/*** This command sends several bits of info ***/
void printNJA(u_char *mesg, uint32_t len)
{
	if (!len) return;

	/* Don't want to print a message for every line of the image we recv */
	if (mesg[0] == nja_prev && (nja_prev == '0' || nja_prev == '1'))
		return;

	printf("Jktart: ");
	switch(mesg[0])
	{
	case '0':
		printf("BMP data (%c)\n",mesg[0]);
		break;
	case '1':
		printf("JPEG data (%c)\n",mesg[0]);
		break;
	case '2':
		printf("Image URL = ");
		printMesg(mesg+2,len-2);
		break;
	case 'B':
		puts("Enabled as BMP");
		break;
	case 'D':
		puts("Disabled");
		break;
	case 'E':
		puts("Enabled");
		break;
	case 'L':
		puts("Enabled as URL");
		break;
	case 'n':
		/* Sends 'n' when switched off */
		puts("Image not available");
		break;
	default:
		printf("Unknown response '%c'\n",mesg[0]);
	}
	nja_prev = mesg[0];
}




void printNLS(u_char *mesg, uint32_t len)
{
	int prev_menu_cursor_pos;
	int rx_cursor_pos;

	printf("Menu  : ");
	switch(mesg[0])
	{
	case 'A':
		/* Never seen the streamer send 'A' */
		switch(mesg[1])
		{
		case '-': printf("<none>");   break;
		case 'A': printf("Artist");   break;
		case 'F': printf("Folder");   break;
		case 'P': printf("Playlist"); break;
		case 'M': printf("Music");    break;
		case 'S': printf("Search");   break;
		default : printf("?");
		}
		printf(" (%c)\n",mesg[1]);
		break;
	case 'C':
		if (mesg[1] == '-')
		{
			printf("No cursor, ");
			switch(mesg[2])
			{
			case 'P': printf("page info update"); break;
			case 'C': printf("position update");  break;
			default : printf("?");
			}
			printf(" (%c)\n",mesg[2]);
			break;
		}

		printf("Cursor position %c\n",mesg[1]);

		/* Don't recalculate cursor pos if pretty printing due to
		   the show command */
		if (!FLAGISSET(FLAG_PRETTY_PRINTING) && isdigit(mesg[1]))
		{
			/* Deal with streamer sending 10+ as 0+ if more than
			   10 items (0 - 9) in the menu. Sadly there's no way
			   of getting this to work properly if we restart with
			   the cursor pointing at for example 10 as the 
			   streamer will just send 0 and we have no way of 
			   knowing it should be 10 */
			prev_menu_cursor_pos = menu_cursor_pos;
			menu_cursor_pos = rx_cursor_pos = mesg[1] - '0';

			if (!menu_cursor_pos &&
			    prev_menu_cursor_pos < menu_option_cnt - 1 &&
			    FLAGISSET(FLAG_COM_DN))
			{
				menu_cursor_pos = prev_menu_cursor_pos + 1;
			}
			else
			if (!prev_menu_cursor_pos &&
			    menu_cursor_pos < menu_option_cnt - 1 &&
			    FLAGISSET(FLAG_COM_UP))
			{
				menu_cursor_pos = menu_option_cnt - 1;
			}

			/* Things can occasionally go a bit wonky so do a
			   sanity check */
			if (menu_cursor_pos % 10 != rx_cursor_pos % 10)
				menu_cursor_pos = rx_cursor_pos;
		}
		break;

	case 'U':
		++mesg;
		printMesg(mesg,len-1);
		if (!FLAGISSET(FLAG_PRETTY_PRINTING)) addMenuOption(mesg,len-3);
		SETFLAG(FLAG_IN_MENU);
		return;
	default:
		printMesg(mesg,len);
		break;
	}
	UNSETFLAG(FLAG_IN_MENU);
}




/*** NET/USB menu status ***/
void printNMS(u_char *mesg, uint32_t len)
{
	int val;
	int i;

	puts("Net/USB menu status:");	

	printf("   Track menu    : ");
	switch(mesg[0])
	{
	case 'M': printf("Enabled");  break;
	case 'x': printf("Disabled"); break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[0]);

	printf("   F1 button icon: ");
	for(i=1;i < 5;i+=2)
	{
		/* Seems to be disabled with streamer but put options here
		   anyway. God knows what half of them mean. */
		if (!memcmp(mesg+1,"xx",2)) printf("Disabled");
		else
		{
			switch(hexToInt(mesg+i,2))
			{
			case 0x1: printf("Like");               break;
			case 0x2: printf("Don't like");         break;
			case 0x3: printf("Love");               break;
			case 0x4: printf("Ban");                break;
			case 0x5: printf("Episode");            break;
			case 0x6: printf("Ratings");            break;
			case 0x7: printf("Ban (black)");        break;
			case 0x8: printf("Ban (white)");        break;
			case 0x9: printf("Favourite (black)");  break;
			case 0xA: printf("Favourite (white)");  break;
			case 0xB: printf("Favourite (yellow)"); break;
			default : printf("?");
			}
		}
		printf(" (%.2s)\n",mesg+1);
		if (i == 1) printf("   F2 button icon: ");
	}

	printf("   Time seek     : ");
	switch(mesg[5])
	{
	case 'S': printf("Enabled "); break;
	case 'x': printf("Disabled"); break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[5]);

	printf("   Time display  : ");
	switch(mesg[6])
	{
	case '1': printf("Elapsed/total time"); break;
	case '2': printf("Elapsed time");       break;
	case 'x': printf("Disabled");           break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[6]);

	printf("   Service icon  : ");
	switch((val = (int)hexToInt(mesg+7,2)))
	{
	case 0x00: printf("Music server (DLNA)"); break;
	case 0x01: printf("My Favourite");   break;
	case 0x02: printf("vTuner");         break;
	case 0x03: printf("SiriusXM");       break;
	case 0x04: printf("Pandora");        break;
	case 0x05: printf("Rhapsody");       break;
	case 0x06: printf("Last.fm");        break;
	case 0x07: printf("Napster");        break;
	case 0x08: printf("Slacker");        break;
	case 0x09: printf("Mediafly");       break;
	case 0x0A: printf("Spotify");        break;
	case 0x0B: printf("AUPEO!");         break;
	case 0x0C: printf("radiko");         break;
	case 0x0D: printf("e-onkyo");        break;
	case 0x0E: printf("TuneIn");         break;
	case 0x0F: printf("MP3tunes");       break;
	case 0x10: printf("Simfy");          break;
	case 0x11: printf("Home Media");     break;
	case 0x12: printf("Deezer");         break;
	case 0x13: printf("iHeartRadio");    break;
	case 0x18: printf("Airplay");        break;
	case 0x1A: printf("onkyo Music");    break;
	case 0x1B: printf("Tidal");          break;
	case 0x41: printf("FireConnect");    break;
	case 0xF0: printf("USB (Front)");    break;
	case 0xF1: printf("USB (Rear)");     break;
	case 0xF2: printf("Internet radio"); break;
	case 0xF3: printf("NET");            break;
	case 0xF4: printf("Bluetooth");      break;
	default  : printf("?");
	}
	printf(" (0x%02X)\n",val);
}




/*** Current network selection info. First 2 hex digits give the service ***/
void printNLT(u_char *mesg, uint32_t len)
{
	const char *status[15] =
	{
		/* 0 - 4 */
		"None",
		"Connecting",
		"Aquiring license",
		"Buffering",
		"Cannot play",

		/* 5 - 9 */
		"Searching",
		"Profile update",
		"Operation disabled",
		"Server start-up",
		"Song rated as Favourite",

		/* 10 - 14 */
		"Song banned from station",
		"Authentication failed",
		"Spotify paused",
		"Track not available",
		"Cannot skip"
	};	
	int val;

	puts("Net/USB screen:");

	/* Info in what seems like a sane order to me, not the order its
	   presented in the string */
	printf("   Service   : ");
	switch((val = (int)hexToInt(mesg,2)))
	{
	case 0:    printf("Music server"); break;
	case 0x0A: printf("Spotify");      break;
	case 0x0E: printf("TuneIn");       break;
	case 0x12: printf("Deezer");       break;
	case 0x18: printf("Airplay");      break;
	case 0x1B: printf("Tidal");        break;
	case 0x1C: printf("Amazon Music"); break;
	case 0x1D: printf("Play Queue");   break;
	case 0x40: printf("Chromecast");   break;
	case 0x42: printf("DTS Play-Fi");  break;
	case 0x43: printf("FlareConnect"); break;
	case 0xF0: printf("USB front");    break;
	case 0xF1: printf("USB rear");     break;
	case 0xF3: printf("Network");      break;
	case 0xFF: printf("<none>");       break;
	default:   printf("?");
	}
	printf(" (0x%02X)\n",val);

	printf("   Title bar : ");
	printMesg(mesg+22,len-22);

	printf("   UI Type   : ");
	switch(mesg[2])
	{
	case '0': printf("List");      break;
	case '1': printf("Menu");      break;
	case '2': printf("Playback");  break;
	case '3': printf("Popup");     break;
	case '4': printf("Keyboard");  break;
	case '5': printf("Menu list"); break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[2]);

	val = (int)hexToInt(mesg+8,4);
	printf("   List size : %d (0x%04X)\n",val,val);
	val = (int)hexToInt(mesg+12,2);
	printf("   Layers    : %d (0x%02X)\n",val,val);
	printf("   Layer info: ");
	switch(mesg[3])
	{
	case '0': printf("NET Top");          break;
	case '1': printf("Service Top");      break;
	case '2': printf("Under 2nd layer");  break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[3]);

	printf("   First?    : %s\n",mesg[14] == '1' ? "YES" : "NO");

	/* Skip icon info and go to status */
	val = (int)hexToInt(mesg+20,2);
	printf("   Status    : %s (0x%02X)\n",val < 16 ? status[val] : "?",val);
}




void printNRI(u_char *mesg, uint32_t len)
{
	puts("\n*** Device setup ***\n");
	printMesg(mesg,len);
}




void printNSB(u_char *mesg, uint32_t len)
{
	printf("Network standby: ");
	printMesg(mesg,len);
}




void printNST(u_char *mesg, uint32_t len)
{
	puts("Net/USB play status:");

	printf("   Play   : ");
	switch(mesg[0])
	{
	case 'S': puts("STOPPED"); break;
	case 'P': puts("PLAYING"); break;
	case 'p': puts("PAUSED");  break;
	case 'F': puts("FF");      break;
	case 'R': puts("FR");      break;
	case 'E': puts("EOF");     break;
	default : printf("%c\n",mesg[0]);
	}

	printf("   Repeat : ");
	switch(mesg[1])
	{
	case '-': puts("OFF");      break;
	case 'R': puts("ALL");      break;
	case 'F': puts("FOLDER");   break;
	case '1': puts("REPEAT");   break;
	case 'x': puts("DISABLED"); break;
	default : printf("%c\n",mesg[1]);
	}

	printf("   Shuffle: ");
	switch(mesg[2])
	{
	case '-': puts("OFF");      break;
	case 'S': puts("ALL");      break;
	case 'A': puts("ALBUM");    break;
	case 'F': puts("FOLDER");   break;
	case 'x': puts("DISABLED"); break;
	default : printf("%c\n",mesg[2]);
	}
}




void printNTI(u_char *mesg, uint32_t len)
{
	printf("Title : ");
	printTranslatedMesg(mesg,len,1);
}




void printNTM(u_char *mesg, uint32_t len)
{
	printTrackTime();
}




void printNTR(u_char *mesg, uint32_t len)
{
	puts("Tracks: ");
	if (len > 3)
	{
		printf("   Current: ");
		printMesg(mesg,4);

		if (len > 8)
		{
			printf("   Total  : ");
			printMesg(mesg+5,4);
		}
	}
	else puts("   No info");
}




void printNLU(u_char *mesg, uint32_t len)
{
	puts("Net/USB list info: ");	
	if (len != 8)
	{
		printMesg(mesg,len);
		return;
	}
	printf("   Update index: %ld (0x%.4s)\n",hexToInt(mesg,4),mesg);
	printf("   Item count  : %ld (0x%.4s)\n",hexToInt(mesg+4,4),mesg+4);
}




void printPWR(u_char *mesg, uint32_t len)
{
	printf("Power : ");
	PRINT_ON_OFF();
}




void printAPD(u_char *mesg, uint32_t len)
{
	printf("Auto power down: ");
	PRINT_ON_OFF();
}




/*** Received when some hardware inputs are selected ***/
void printSLI(u_char *mesg, uint32_t len)
{
	int val;

	printf("Input : ");
	switch((val = (int)hexToInt(mesg,2)))
	{
	case 0x27: printf("Music server"); break;
	case 0x29: printf("USB front");    break;
	case 0x2A: printf("USB rear");     break;
	case 0x2B: printf("Network");      break;
	case 0x2F: printf("USB DAC");      break;
	case 0x44: printf("Digital 2");    break;
	case 0x45: printf("Digital 1");    break;
	default  : printf("?");
	}
	printf(" (0x%02X)\n",val);
}




void printUPD(u_char *mesg, uint32_t len)
{
	printf("Update status: ");
	switch(mesg[0])
	{
	case 'C':
		puts("COMPLETE");
		return;
	case 'D':
		if (isdigit(mesg[1]))
			printf("PROGRESS %.2s%%\n",mesg+1);
		else
			printf("DOWNLOAD state %.2s\n",mesg+1);
		return;
	case 'E':
		printf("ERROR %.4s\n",mesg+1);
		return;
	case '0':
		switch(mesg[1])
		{
		case '0':
			puts("No new firmware available.");
			return;
		case '1':
			puts("New firmware available.");
			return;
		case '2':
			puts("New firmware available (forced upgrade).");
			return;
		}
	}
	printMesg(mesg,len);
}




void printUPS(u_char *mesg, uint32_t len)
{
	printf("Upsampling: ");
	if (mesg[0] == '0')
	{
		/* N70 doesn't seem to support x2 and x4 but they're here
		   for completeness */
		switch(mesg[1])
		{
		case '0': printf("OFF"); break;
		case '1': printf("ON x2");  break;
		case '2': printf("ON x4");  break;
		case '3': printf("ON x8");  break;
		default : printf("?");
		}
	}
	else printf("?");
	printf(" (%.2s)\n",mesg);
}




void printPPS(u_char *mesg, uint32_t len)
{
	printf("Privacy policy status: ");
	switch(mesg[0])
	{
	case '0':
		puts("Disagree");
		break;
	case '1':
		puts("Agree");
		break;
	default:
		puts("?");
	}
}




void printDIM(u_char *mesg, uint32_t len)
{
	printf("Dimmer: ");
	if (mesg[0] == '0')
	{
		switch(mesg[1])
		{
		case '0': printf("Bright");   break;
		case '1': printf("Dim");      break;
		case '2': printf("Dark");     break;
		case '3': printf("Shut-off"); break;
		case '8': printf("LED-OFF");  break;
		default : printf("?");
		}
	}
	else printf("?");
	printf(" (%.3s)\n",mesg);
}




void printLRA(u_char *mesg, uint32_t len)
{
	printf("LRA   : %.*s\n",len,mesg);
}




void printMGV(u_char *mesg, uint32_t len)
{
	printf("Multiroom group version: ");
	if (len > 2 && isNumberWithLen(mesg,len))
		printf("%c.%.*s\n", mesg[0],len-1,mesg+1);
	else
		printf("%.*s\n",len,mesg);
}




/*** The bits have meaning but never seem to be set on the N-70 so just
     print the hex ***/
void printMRM(u_char *mesg, uint32_t len)
{
	printf("Multiroom info: ");
	if (len > 1)
		printf("0x%02x%02x\n",mesg[0] - '0',mesg[1] - '0');
	else 
		printf("%.*s\n",len,mesg);
}




/*** Not an RX decoder but needs the comfunc[] array ***/
void printRXCommands(u_char *pat)
{
	int cnt;
	int i;

	puts("\n*** RX streamer commands parsed by this client ***\n");
	for(i=cnt=0;comfunc[i].func;++i)
	{
		if (!pat || wildMatch(comfunc[i].com,(char *)pat))
		{
			if (cnt && !(cnt % 10)) putchar('\n');
			printf("%s  ",comfunc[i].com);
			++cnt;
		}
	}
	if (pat)
		printf("\n\n%d of %d commands.\n\n",cnt,i);
	else
		printf("\n\n%d commands.\n\n",cnt);
}




void printTrackTime()
{
	/* The 2 strings are set in readSocket() in network.c */
	printf("Track time  : %s",track_time_str);
	if (track_len_str[0] != '-')
		printf(" of %s\n",track_len_str);
	else
		puts(" (no track length)");
}


/***************************** Internal functions *****************************/

/*** Translate HTML ampersand codes if flag set ***/
void printTranslatedMesg(u_char *mesg, uint32_t len, int add_title)
{
	u_char *tstr;

	if (len && FLAGISSET(FLAG_TRANS_HTML_AMPS)) 
	{
		tstr = replaceAmpCodes(mesg,&len);
		printMesg(tstr,len);
		if (add_title) addTitle(tstr,len);
		free(tstr);
	}
	else
	{
		printMesg(mesg,len);
		if (add_title) addTitle(mesg,len);
	}
}




/*** Replace all the known HTML ampersand codes in the string and return a
     new string - ie don't change the original so we don't muck up data 
     stored in the RX list ***/
u_char *replaceAmpCodes(u_char *mesg, uint32_t *len)
{
	u_char *str;
	u_char *ptr;
	u_char *ptr2;
	u_char c;
	uint32_t i;
	uint32_t j;
	int reduce;
	int movelen;

	assert((str = (u_char *)malloc(*len)));
	memcpy(str,mesg,*len);

	for(ptr=str,i=0;i < *len;++ptr,++i)
	{
		if (*ptr != '&') continue;

		for(ptr2=ptr+1,j=1;
		    j < 9 && j < *len && *ptr2 != '&';++ptr2,++j)
		{
			if (*ptr2 != ';') continue;

			/* Translate code */
			*ptr2 = 0;
			c = translateAmpCode((char *)ptr+1);

			/* Get rid of code from string */
			reduce = (int)(ptr2 - ptr);
			movelen = *len - i - reduce + 1;
			memmove(ptr,ptr2,movelen);

			/* Put in code character and reduce length */
			*ptr = c;
			*len -= reduce;
			break;
		}
	}
	return str;
}




/*** Only translates to single ascii characters, not unicode ***/
u_char translateAmpCode(char *code)
{
	/* Only need a few
	   https://dev.w3.org/html5/html-author/charref */
	const char *codes[] =
	{
		"lbrace","{",
		"rbrace","}",
		"commat","@",
		"quote", "\"",
		"colon", ":",
		"semi",  ";",
		"apos",  "'",
		"lpar",  "(",
		"rpar",  ")",
		"num",   "#",
		"amp",   "&",
		"lt",    "<",
		"gt",    ">",
		NULL,    NULL
	};
	int i;
	int val;
	if (code[0] == '#')
	{
		val = atoi(code+1);
		return (val > 0 && val < 256) ? val : '?';
	}
	for(i=0;codes[i];i+=2)
		if (!strcasecmp(codes[i],code)) return codes[i+1][0];
	return '?';
}




/*** Convert hex characters to their integer value. String must be writable ***/
long hexToInt(u_char *str, int len)
{
	u_char c;
	long val;

	c = str[len];
	str[len] = 0;
	val = strtol((char *)str,NULL,16);
	str[len] = c;
	return val;
}
