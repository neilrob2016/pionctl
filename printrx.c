#include "globals.h"

#define PRINT_ON_OFF() \
	if (!memcmp(mesg,"00",2)) colPrintf("~FROFF\n"); \
	else if (!memcmp(mesg,"01",2)) colPrintf("~FGON\n"); \
	else printMesg(mesg,len);

#define CHECK_LEN(L) \
	if (len < L) \
	{ \
		colPrintf("~FRUnavailable\n"); \
		return; \
	} 

/* Forward declarations */
void printAMT(char *mesg, uint32_t len);
void printDGF(char *mesg, uint32_t len);
void printDIR(char *mesg, uint32_t len);
void printEDF(char *mesg, uint32_t len);
void printEDV(char *mesg, uint32_t len);
void printIFA(char *mesg, uint32_t len);
void printFWV(char *mesg, uint32_t len);
void printHBT(char *mesg, uint32_t len);
void printMDI(char *mesg, uint32_t len);
void printMOT(char *mesg, uint32_t len);
void printPWR(char *mesg, uint32_t len);
void printAPD(char *mesg, uint32_t len);
void printNAL(char *mesg, uint32_t len);
void printNAT(char *mesg, uint32_t len);
void printNCP(char *mesg, uint32_t len);
void printNDN(char *mesg, uint32_t len);
void printNDS(char *mesg, uint32_t len);
void printNFI(char *mesg, uint32_t len);
void printNFN(char *mesg, uint32_t len);
void printNJA(char *mesg, uint32_t len);
void printNLS(char *mesg, uint32_t len);
void printNMS(char *mesg, uint32_t len);
void printNLT(char *mesg, uint32_t len);
void printNRI(char *mesg, uint32_t len);
void printNSB(char *mesg, uint32_t len);
void printNST(char *mesg, uint32_t len);
void printNTI(char *mesg, uint32_t len);
void printNTM(char *mesg, uint32_t len);
void printNTR(char *mesg, uint32_t len);
void printNLU(char *mesg, uint32_t len);
void printSLI(char *mesg, uint32_t len);
void printUPD(char *mesg, uint32_t len);
void printUPS(char *mesg, uint32_t len);
void printPPS(char *mesg, uint32_t len);
void printDIM(char *mesg, uint32_t len);
void printLRA(char *mesg, uint32_t len);
void printMGV(char *mesg, uint32_t len);
void printMRM(char *mesg, uint32_t len);
void printMMT(char *mesg, uint32_t len);
void printRST(char *mesg, uint32_t len);
void printXMLField(char *field, int mac, char *mesg, uint32_t len);
void printTranslatedMesg(char *mesg, uint32_t len, int add_title);
char *replaceAmpCodes(char *str, uint32_t *len);
char translateAmpCode(char *code);
long hexToInt(char *str, int len);

/* Map of 3 character server commands/replies to functions ***/
static struct st_comfunc
{
	char *com;
	void (*func)(char *mesg, uint32_t);
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
	{ "MMT", printMMT },
	{ "RST", printRST },

	/* 40 */
	{ "", NULL }
};

#define RX_COM_NJA 17
#define RX_COM_NLS 18

/******************************** EXTERNAL *********************************/

/*** Prints out message buffer data translating non printing characters ***/
void printMesg(char *mesg, int len)
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
		if (!strncmp(pkt_data->command,comfunc[i].com,3))
		{
			if (i != RX_COM_NJA) nja_prev = 0;
			if (i != RX_COM_NLS && flags.in_menu) flags.in_menu = 0;

			/* Call print function */
			comfunc[i].func(pkt_data->mesg,mesg_len);
			break;
		}
	}

	if (!comfunc[i].func)
	{
		colPrintf("~FYUnknown response:~RS %.3s",pkt_data->command);
		printMesg(pkt_data->mesg,mesg_len);
		setUnknownRXKey(pkt_data->command);
	}

	if (print_prompt) printPrompt();
	clearBuffer(BUFF_TCP);
}




/*** Pretty print the RX commands and data stored in the list ***/
int prettyPrintRXList(char *pat, int max)
{
	t_entry *entry;
	char *value;
	int total;
	int len;
	int cnt;
	int i;
	int j;

	if (max < 0)
	{
		usagePrintf("show [<pattern> [<count>]]\n");
		return ERR_CMD_FAIL;
	}

	colPrintf("\n~BB~FW*** Processed streamer RX (%s : %sCONNECTED) ***\n\n",
		inet_ntoa(con_addr.sin_addr),tcp_sock ? "" : "DIS");

	flags.pretty_printing = 1;
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
				value = (char *)"";
				len = 0;
			}

			/* Continue looping anyway to get correct total */
			if (max && cnt == max) continue;

			/* Match on raw command and value */
			if (pat && 
			    !wildMatch(entry->key,pat) &&
			    !wildMatch(value,pat))
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
	flags.pretty_printing = !flags.pretty_printing;

	return OK;
}




/*** Not an RX decoder but needs the comfunc[] array ***/
void printRXCommands(char *pat)
{
	int cnt;
	int i;

	colPrintf("\n~BB~FW*** RX streamer commands parsed by this client ***\n\n");
	for(i=cnt=0;comfunc[i].func;++i)
	{
		if (!pat || wildMatch(comfunc[i].com,pat))
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




void printTrackTime(int rx)
{
	int i;
	int cnt;

	if (rx)
		printf("Track time: %s",track_time_str);
	else
		printf("Track  : %s",track_time_str);

	/* Streamer normally sends --:--:-- for unknown track length but do
	   sanity check for numbers anyway */
	for(i=cnt=0;i < 8;++i) cnt += isdigit(track_len_str[i]);
	if (cnt >= 6)
		printf(" of %s\n",track_len_str);
	else
		puts(" (no track length)");
}


/********************************** PRINT ***********************************/

void printAMT(char *mesg, uint32_t len)
{
	printf("Muting    : ");
	PRINT_ON_OFF();
}




/*** Digital filter ***/
void printDGF(char *mesg, uint32_t len)
{
	printf("Filter    : ");

	/* Values are 00, 01, 02 */
	if (mesg[0] == '0')
	{
		switch(mesg[1])
		{
		case '0': colPrintf("~FMSLOW\n");     return;
		case '1': colPrintf("~FB~OLSHARP\n"); return;
		case '2': colPrintf("~FGSHORT\n");    return;
		}
	}
	printMesg(mesg,len);
}




void printDIR(char *mesg, uint32_t len)
{
	printf("Direct    : ");
	PRINT_ON_OFF();
}




void printEDF(char *mesg, uint32_t len)
{
	printf("Memory    : ");
	if (len < 8)
		puts("<unknown>");
	else
		printf("%ld MB free\n",hexToInt(mesg,8));
}




void printEDV(char *mesg, uint32_t len)
{
	printf("Action    : ");
	if (!memcmp(mesg,"00",2)) colPrintf("~FRNot approved\n");
	else if (!memcmp(mesg,"01",2)) colPrintf("~FGApproved\n");
	else printMesg(mesg,len);
}




/*** Message is CSV, eg: NETWORK,,,,Stereo,,,0 ms,Normal, 
     N-70AE only seems to use some of the fields and it doesn't quite match
     with the documentation as PQLS field appears to be missing. ***/
void printIFA(char *mesg, uint32_t len)
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
	char c1 = mesg[len];
	char c2 = 0;
	int i;
	
	mesg[len] = 0;
	colPrintf("~FTAudio info:\n");

	/* Not using strtok() because it modifies the string it searches */
	for(i=0,ptr=mesg-1;i< 10 && ptr;++i,ptr=end)
	{
		++ptr;
		if ((end = (char *)strchr(ptr,',')))
		{
			c2 = *end;
			*end = 0;
		}
		printf("   %-21s: %s\n",field[i],ptr);
		if (end) *end = c2;
	}
	mesg[len] = c1;
}




void printFWV(char *mesg, uint32_t len)
{
	printf("Firmware vers: ");
	printMesg(mesg,len);
}




void printHBT(char *mesg, uint32_t len)
{
	printf("High bit  : ");
	PRINT_ON_OFF();
}




void printMDI(char *mesg, uint32_t len)
{
	colPrintf("~FTStreamer info:\n");
	printMesg(mesg,len);
}




void printMOT(char *mesg, uint32_t len)
{
	printf("ASR       : ");
	PRINT_ON_OFF();
}




void printNAL(char *mesg, uint32_t len)
{
	printf("Album name: ");
	printTranslatedMesg(mesg,len,0);
}




void printNAT(char *mesg, uint32_t len)
{
	printf("Artist    : ");
	printTranslatedMesg(mesg,len,0);
}




/*** Usually returned after an NSV command, eg NSV1B0 for Tidal ***/
void printNCP(char *mesg, uint32_t len)
{
	const char *popup_type[6] =
	{ 
		"List",
		"Menu",
		"Playback",
		"Popup",
		"Keyboard",
		"Menu list"
	};
	const char *update_type[4] =
	{
		"All",
		"Button",
		"Textbox",
		"Listbox"
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
void printNDN(char *mesg, uint32_t len)
{
	printf("Device    : ");
	printMesg(mesg,len);
}




void printNDS(char *mesg, uint32_t len)
{
	int i;

	colPrintf("~FTConnector status: ");
	CHECK_LEN(1);
	colPrintf("\n");

	printf("   Network  : ");

	switch(mesg[0])
	{
	case '-': colPrintf("~FRNone~RS"); break;
	case 'E': printf("Ethernet");      break;
	case 'W': printf("WiFi");          break;
	default : printf("?");
	}
	printf(" (%c)\n",mesg[0]);
	
	if (len < 3) return;

	printf("   Front USB: ");
	for(i=1;i < 3;++i)
	{
		switch(mesg[i])
		{
		case '-': colPrintf("~FYNothing attached~RS"); break;
		case 'i': printf("iPod/iPhone");       break;
		case 'M': printf("Memory/NAS");        break;
		case 'W': printf("Wireless adaptor");  break;
		case 'B': printf("Bluetooth adaptor"); break;
		case 'x': colPrintf("~FRDisabled\n");
		}
		printf(" (%c)\n",mesg[i]);
		if (i == 1) printf("   Rear USB : ");
	}
}




void printNFI(char *mesg, uint32_t len)
{
	printf("Codec type: ");
	printMesg(mesg,len);
}




void printNFN(char *mesg, uint32_t len)
{
	colPrintf("~FTIdentity  :\n");
	/* No lookup table in docs */
	printf("   Manufacturer ID: %d\n",mesg[0]); 
	printf("   Friendly name  : ");
	printMesg(mesg+1,len - 1);
}




/*** This command sends several bits of info ***/
void printNJA(char *mesg, uint32_t len)
{
	if (!len) return;

	/* Don't want to print a message for every line of the image we recv */
	if (mesg[0] == nja_prev && (nja_prev == '0' || nja_prev == '1'))
		return;

	printf("Jacket art: ");
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
		colPrintf("~FGEnabled~RS as BMP\n");
		break;
	case 'D':
		colPrintf("~FRDisabled\n");
		break;
	case 'E':
		colPrintf("~FGEnabled\n");
		break;
	case 'L':
		colPrintf("~FGEnabled~RS as URL\n");
		break;
	case 'n':
		/* Sends 'n' when switched off */
		colPrintf("~FRImage not available\n");
		break;
	default:
		printf("Unknown response \"%c\"\n",mesg[0]);
	}
	nja_prev = mesg[0];
}




void printNLS(char *mesg, uint32_t len)
{
	int prev_menu_cursor_pos;
	int rx_cursor_pos;

	/* Only used by wait_for_menu so reset there */
	flags.rx_menu = 1;

	printf("Menu      : ");
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
		if (!flags.pretty_printing && isdigit(mesg[1]))
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
			    flags.com_dn)
			{
				menu_cursor_pos = prev_menu_cursor_pos + 1;
			}
			else
			if (!prev_menu_cursor_pos &&
			    menu_cursor_pos < menu_option_cnt - 1 &&
			    flags.com_up)
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
		if (!flags.pretty_printing) addMenuOption(mesg,len-3);
		flags.in_menu = 1;
		return;
	default:
		printMesg(mesg,len);
		break;
	}
	flags.in_menu = 0;
}




/*** NET/USB menu status ***/
void printNMS(char *mesg, uint32_t len)
{
	int val;
	int i;

	colPrintf("~FTNet/USB menu status: ");
	CHECK_LEN(1);
	colPrintf("\n");

	printf("   Track menu    : ");
	switch(mesg[0])
	{
	case 'M': colPrintf("~FGEnabled");  break;
	case 'x': colPrintf("~FRDisabled"); break;
	default : printf("?");
	}
	colPrintf("~RS (%c)\n",mesg[0]);

	if (len < 5) return;

	printf("   F1 button icon: ");
	for(i=1;i < 5;i+=2)
	{
		/* Seems to be disabled with streamer but put options here
		   anyway. God knows what half of them mean. */
		if (!memcmp(mesg+1,"xx",2)) colPrintf("~FRDisabled~RS");
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

	if (len < 6) return;

	printf("   Time seek     : ");
	switch(mesg[5])
	{
	case 'S': colPrintf("~FGEnabled "); break;
	case 'x': colPrintf("~FRDisabled"); break;
	default : printf("?");
	}
	colPrintf("~RS (%c)\n",mesg[5]);

	if (len < 7) return;

	printf("   Time display  : ");
	switch(mesg[6])
	{
	case '1': printf("Elapsed/total time"); break;
	case '2': printf("Elapsed time");       break;
	case 'x': colPrintf("~FRDisabled");     break;
	default : printf("?");
	}
	colPrintf("~RS (%c)\n",mesg[6]);

	if (len < 8) return;

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
void printNLT(char *mesg, uint32_t len)
{
	const char *status[15] =
	{
		/* 0 - 4 */
		"~FYNone",
		"Connecting",
		"Aquiring license",
		"Buffering",
		"~FRCannot play",

		/* 5 - 9 */
		"Searching",
		"Profile update",
		"Operation disabled",
		"Server start-up",
		"Song rated as Favourite",

		/* 10 - 14 */
		"Song banned from station",
		"~FRAuthentication failed",
		"Spotify paused",
		"Track not available",
		"~FRCannot skip"
	};	
	int val;

	colPrintf("~FTNet/USB screen:\n");

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

	colPrintf("   First?    : %s~RS\n",
		mesg[14] == '1' ? "~FGYES" : "~FRNO");

	/* Skip icon info and go to status */
	val = (int)hexToInt(mesg+20,2);
	colPrintf("   Status    : %s~RS (0x%02X)\n",
		val < 16 ? status[val] : "?",val);
}




/*** Don't reset nri_command so that if user calls "show rx" it'll show 
     whichever NRI command they last requested ***/
void printNRI(char *mesg, uint32_t len)
{
	switch(nri_command)
	{
	case COM_SERIAL:
		/* Colour output because it depends on last command */
		colPrintf("~FB~OLSerial num:~RS ");
		printXMLField("deviceserial",0,mesg,len);
		break;
	case COM_ETHMAC:
		/* Ethernet MAC, not wifi MAC. The latter isn't given in the
		   setup data and there doesn't seem to be any alternative way
		   to get it */
		colPrintf("~FB~OLEther MAC :~RS ");
		printXMLField("macaddress",1,mesg,len);
		break;
	case COM_ICONURL:
		colPrintf("~FB~OLIcon URL  :~RS ");
		printXMLField("modeliconurl",0,mesg,len);
		break;
	case COM_MODEL:
		colPrintf("~FB~OLModel type:~RS ");
		printXMLField("model",0,mesg,len);
		break;
	case COM_TIDALVER:
		colPrintf("~FB~OLTidal vers:~RS ");
		printXMLField("tidaloauthversion",0,mesg,len);
		break;
	case COM_ECOVER:
		colPrintf("~FB~OLEcosys ver:~RS ");
		printXMLField("ecosystemversion",0,mesg,len);
		break;
	case COM_PRODID:
		colPrintf("~FB~OLProduct ID:~RS ");
		printXMLField("productid",0,mesg,len);
		break;
	default:
		colPrintf("~FB~OLDevice setup:\n");
		printMesg(mesg,len);
	}
}




void printNSB(char *mesg, uint32_t len)
{
	printf("Network standby: ");
	printMesg(mesg,len);
}




void printNST(char *mesg, uint32_t len)
{
	colPrintf("~FTNet/USB play status: ");
	CHECK_LEN(1);
	colPrintf("\n");

	printf("   Play   : ");
	switch(mesg[0])
	{
	case 'S': colPrintf("~FRSTOPPED\n"); break;
	case 'P': colPrintf("~FGPLAYING\n"); break;
	case 'p': colPrintf("~FYPAUSED\n");  break;
	case 'F': colPrintf("~FB~OLFF\n");   break;
	case 'R': colPrintf("~FMFR\n");      break;
	case 'E': colPrintf("~FYEOF\n");     break;
	default : printf("%c\n",mesg[0]);
	}

	if (len < 2) return;

	printf("   Repeat : ");
	switch(mesg[1])
	{
	case '-': colPrintf("~FMOFF\n");       break;
	case '1': colPrintf("~FB~OLREPEAT\n"); break;
	case 'F': colPrintf("~FYFOLDER\n");    break;
	case 'R': colPrintf("~FGALL\n");       break;
	case 'x': colPrintf("~FRDISABLED\n");  break;
	default : printf("%c\n",mesg[1]);
	}

	if (len < 3) return;

	printf("   Shuffle: ");
	switch(mesg[2])
	{
	case '-': colPrintf("~FMOFF\n");      break;
	case 'A': colPrintf("~FB~OLALBUM\n"); break;
	case 'F': colPrintf("~FYFOLDER\n");   break;
	case 'S': colPrintf("~FGALL\n");      break;
	case 'x': colPrintf("~FRDISABLED\n"); break;
	default : printf("%c\n",mesg[2]);
	}
}




void printNTI(char *mesg, uint32_t len)
{
	printf("Trak title: ");
	printTranslatedMesg(mesg,len,1);
}




void printNTM(char *mesg, uint32_t len)
{
	printTrackTime(1);
}




void printNTR(char *mesg, uint32_t len)
{
	colPrintf("~FTTracks    :\n");
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




void printNLU(char *mesg, uint32_t len)
{
	colPrintf("~FTNet/USB list info:\n");
	if (len != 8)
	{
		printMesg(mesg,len);
		return;
	}
	printf("   Update index: %ld (0x%.4s)\n",hexToInt(mesg,4),mesg);
	printf("   Item count  : %ld (0x%.4s)\n",hexToInt(mesg+4,4),mesg+4);
}




void printPWR(char *mesg, uint32_t len)
{
	printf("Power stat: ");
	PRINT_ON_OFF();
}




void printAPD(char *mesg, uint32_t len)
{
	printf("Auto power down: ");
	PRINT_ON_OFF();
}




/*** Received when some hardware inputs are selected ***/
void printSLI(char *mesg, uint32_t len)
{
	int val;

	printf("Input type: ");
	CHECK_LEN(2);

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




void printUPD(char *mesg, uint32_t len)
{
	printf("Upd status: ");
	switch(mesg[0])
	{
	case 'C':
		colPrintf("~FGCOMPLETE\n");
		return;
	case 'D':
		if (isdigit(mesg[1]))
			printf("PROGRESS %.2s%%\n",mesg+1);
		else
			printf("DOWNLOAD state %.2s\n",mesg+1);
		return;
	case 'E':
		colPrintf("~FRERROR~RS %.4s\n",mesg+1);
		return;
	case '0':
		switch(mesg[1])
		{
		case '0':
			colPrintf("~FYNo new firmware available.\n");
			return;
		case '1':
			colPrintf("~FB~OLNew firmware available.\n");
			return;
		case '2':
			colPrintf("~FMNew firmware available (forced upgrade).\n");
			return;
		}
	}
	printMesg(mesg,len);
}




void printUPS(char *mesg, uint32_t len)
{
	printf("Upsampling: ");
	CHECK_LEN(1);

	if (mesg[0] == '0')
	{
		/* N70 doesn't seem to support x2 and x4 but they're here
		   for completeness */
		switch(mesg[1])
		{
		case '0': colPrintf("~FROFF"); break;
		case '1': colPrintf("~FMON x2");  break;
		case '2': colPrintf("~FYON x4");  break;
		case '3': colPrintf("~FGON x8");  break;
		default : colPrintf("?");
		}
	}
	else printf("?");
	colPrintf("~RS (%.2s)\n",mesg);
}




void printPPS(char *mesg, uint32_t len)
{
	printf("Privacy policy status: ");
	CHECK_LEN(1);

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




void printDIM(char *mesg, uint32_t len)
{
	printf("Dimmer lev: ");
	CHECK_LEN(1);

	if (mesg[0] == '0')
	{
		switch(mesg[1])
		{
		case '0': colPrintf("~FGBright~RS");   break;
		case '1': colPrintf("~FYDim~RS");      break;
		case '2': colPrintf("~FMDark~RS");     break;
		case '3': colPrintf("~FRShut-off~RS"); break;
		case '8': colPrintf("~FRLED-OFF~RS");  break;
		default : printf("?");
		}
	}
	else printf("?");
	printf(" (%.3s)\n",mesg);
}




void printLRA(char *mesg, uint32_t len)
{
	printf("LRA level : ");
	printMesg(mesg,len);
}




void printMGV(char *mesg, uint32_t len)
{
	printf("Multiroom group version: ");
	if (len > 2 && isNumberWithLen(mesg,len))
		printf("%c.%.*s\n", mesg[0],len-1,mesg+1);
	else
		printMesg(mesg,len);
}




/*** The bits have meaning but never seem to be set on the N-70 so just
     print the hex ***/
void printMRM(char *mesg, uint32_t len)
{
	printf("Multiroom info: ");
	if (len > 1)
		printf("0x%02x%02x\n",mesg[0] - '0',mesg[1] - '0');
	else 
		printMesg(mesg,len);
}




/*** No idea what this is reporting but the N70 responds to QSTN ***/
void printMMT(char *mesg, uint32_t len)
{
	const char *mesgstr[10] =
	{
		/* 0 */
		"Possible to send",
		"AirPlay can't resend",
		"HDMI input content = other than PCM",
		"HDMI ACP packet type = other than Generic Audio",
		"OPT/COAX input content = other than PCM",

		/* 5 */
		"Circuit configuration factor",
		"Inputting HMDI 3G or more",
		"Signal exceeding 48Khz is input",
		"Disabled NET/USB playback format",
		"Master device grouped"
	};
	int mnum;

	printf("Multiroom master TX command: ");
	if (len != 2 || mesg[0] != '0')
	{
		printMesg(mesg,len);
		return;
	}
	mnum = mesg[1] - '0';
	if (mnum >= 0 && mnum < 10)
		printf("%s",mesgstr[mnum]);
	else
		putchar('?');
	
	colPrintf(" (%.2s)\n",mesg);
}




/*** Will only normally get a RST response if you manually send RSTQSTN ***/
void printRST(char *mesg, uint32_t len)
{
	printf("Reset     : ");
	printMesg(mesg,len);
}


/********************************** SUPPORT ***********************************/

void printXMLField(char *field, int mac, char *mesg, uint32_t len)
{
	char *ptr;
	char *mptr;
	char *mend;
	char *fstart;
	char *fptr;
	int cnt;

	/* Might find substring of longer field name so need to loop until 
	   we find it or the end of the message */
	mend = mesg + len;
	for(mptr=mesg;mptr < mend;mptr=fstart + 1)
	{
		if (!(ptr = strnstr(mptr,field,len)) ||
		     (fstart = ptr + strlen(field)) >= mend) break;

		if (*fstart == '>')
		{
			++fstart;
			cnt = 0;
			for(fptr=fstart;
			    fptr < mend && *fptr != '<';++fptr)
			{
				putchar(*fptr);
				/* Put the colons in the MAC address */
				if (mac && cnt < 10 && !(++cnt % 2))
					putchar(':');
			}
			putchar('\n');
			return;
		}
	}
	puts("?");
}




/*** Translate HTML ampersand codes if flag set ***/
void printTranslatedMesg(char *mesg, uint32_t len, int add_title)
{
	char *tstr;

	if (len && flags.trans_html_amps)
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
char *replaceAmpCodes(char *mesg, uint32_t *len)
{
	char *str;
	char *ptr;
	char *ptr2;
	char c;
	uint32_t i;
	uint32_t j;
	int reduce;
	int movelen;

	assert((str = (char *)malloc(*len)));
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
			c = translateAmpCode(ptr+1);

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
char translateAmpCode(char *code)
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
long hexToInt(char *str, int len)
{
	char c;
	long val;

	c = str[len];
	str[len] = 0;
	val = strtol(str,NULL,16);
	str[len] = c;
	return val;
}
