#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef MAINFILE
#define EXTERN
#else
#define EXTERN extern
#endif

#define VERSION "20240301"

#define UDP_PORT       10102
#define TCP_PORT       60128
#define ALLOC_BLOCK    10
#define ADDR_LIST_SIZE 20
#define MESG_OFFSET    5
#define SAVE_TIMEOUT   5
#define TIME_DEF_STR   "--:--:--"

#define FREE(M)   { free(M); M = NULL; }
#define FREEIF(M) if (M) FREE(M)

#ifdef __APPLE__
/* Don't bitch about pragma pack every time it sees it */
#pragma clang diagnostic ignored "-Wpragma-pack"
#endif
#pragma pack(1)


enum
{
	INPUT_CMD,
	INPUT_MACRO_DEF,
	INPUT_MACRO_APP
};


enum
{
	BUFF_KEYB_FIRST,
	BUFF_KEYB_LAST = 22,
	BUFF_TCP,

	NUM_BUFFERS
};

#define MAX_HIST_BUFFERS BUFF_TCP 

enum
{
	OK,
	ERR_CMD_MISSING,
	ERR_CMD_FAIL,
	ERR_MACRO,
	ERR_FILE
};


enum
{
	PROMPT_BASE,
	PROMPT_NAME,
	PROMPT_C_TIME,
	PROMPT_T_TIME,
	PROMPT_L_TIME,
	PROMPT_C_T_TIME,
	PROMPT_C_L_TIME,
	PROMPT_T_L_TIME,
	PROMPT_C_T_L_TIME,

	NUM_PROMPTS
};


enum
{
	RAW_OFF,
	RAW_LOW1,
	RAW_LOW2,
	RAW_HIGH1,
	RAW_HIGH2,

	NUM_RAW_LEVELS
};


/* State for saving NJA jpeg */
enum 
{
	SAVE_INACTIVE = -1,
	SAVE_START,
	SAVE_NEXT
};


enum
{
	/* 0. Client commands */
	COM_QUIT,
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
	COM_WAIT_MENU,
	COM_CLS,
	COM_ECHO,
	COM_MACRO,
	COM_BACK,
	LAST_CLIENT_COM = COM_BACK,

	/* 15. Streamer commands */
	COM_MENU,
	FIRST_STREAMER_COM = COM_MENU,
	COM_MENUSTAT,
	COM_UP,
	COM_DN,
	COM_EN,

	/* 20 */
	COM_EX,
	COM_FLIP,
	COM_DS,
	COM_DSD,
	COM_DSSTAT,

	/* 25 */
	COM_FILTER,
	COM_FILSTAT,
	COM_ARTDIS,
	COM_ARTBMP,
	COM_ARTURL,

	/* 30 */
	COM_ARTSTAT,
	COM_ARTSAVE,

	/* Enums beyond saveart not required except for these */
	COM_SETNAME = 84,
	COM_SETUP   = 90,
	COM_SERIAL,
	COM_ETHMAC,
	COM_ICONURL,
	COM_MODINFO,
	COM_TIDALVER,
	COM_ECOVER,
	COM_PRODID,
	COM_LRA     = 105,

	NUM_COMMANDS
};


struct st_command
{
	char *com;
	char *data;
};

#ifdef MAINFILE
struct st_command commands[] =
{
	/* 0. Built in commands */
	{ "quit",  NULL },
	{ "toggle",NULL },
	{ "prompt",NULL },
	{ "raw",   NULL },
	{ "show",  NULL },

	/* 5 */
	{ "clear",     NULL },
	{ "help",      NULL },
	{ "connect",   NULL },
	{ "disconnect",NULL },
	{ "wait",      NULL },

	/* 10 */
	{ "wait_menu",NULL },
	{ "cls",      NULL },
	{ "echo",     NULL },
	{ "macro",    NULL },
	{ "back",     NULL },

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

	/* Content. NJADIS simply disables streamer from sending the art bitmap
	   when we initially tune in to a station. We don't use that anyway as
	   we request the art manually so not much use but can reduce network 
	   traffic. */
	{ "artdis",  "NJADIS"         },
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
	{ "mmtstat", "MMTQSTN" },

	/* Misc */
	{ "cnstat",  "NDSQSTN" },
	{ "top",     "NTCTOP"  },
	{ "dev",     "NDNQSTN" },
	{ "mem",     "EDFQSTN" },
	{ "scr",     "NLTQSTN" },
	{ "pps",     "PPSQSTN" },
	{ "fwver",   "FWVQSTN" },
	{ "xinfo",   "MDIQSTN" },
	{ "auinfo",  "IFAQSTN" },
	{ "stop",    "NTCSTOP" },
	{ "id",      "NFNQSTN" },
	{ "setname", "NFN"     },  /* NFN only here for help print out */
	{ "reset",   "RSTALL"  },
	{ "mgver",   "MGVQSTN" },
	{ "updstat", "UPDQSTN" },
	{ "codec",   "NFIQSTN" },
	{ "playstat","NSTQSTN" },

	/* NRI commands */
	{ "setup",   "NRIQSTN" },
	{ "serial",  "NRIQSTN" },
	{ "ethmac",  "NRIQSTN" },
	{ "iconurl", "NRIQSTN" },
	{ "modinfo", "NRIQSTN" },
	{ "tidalver","NRIQSTN" },
	{ "ecover",  "NRIQSTN" },
	{ "prodid",  "NRIQSTN" },

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
#else
extern struct st_command commands[];
#endif

struct st_flags
{
	/* User toggled flags */
	unsigned show_track_time : 1;
	unsigned trans_html_amps : 1;
	unsigned use_colour      : 1;
	unsigned verbose         : 1;

	/* System flags */
	unsigned macro_running   : 1;
	unsigned offline         : 1;
	unsigned exit_after_cmds : 1;
	unsigned pretty_printing : 1;
	unsigned in_menu         : 1;
	unsigned com_up          : 1;
	unsigned com_dn          : 1;
	unsigned reset_reverse   : 1;
	unsigned interrupted     : 1;
};


struct st_addr
{
	int cnt;
	struct in_addr addr;
};


struct st_buffer
{
	char *data;
	int len;
	int alloc;
};


struct st_rev_com
{
	int com;
	int repeat_cnt;
	int seq_start;
};


typedef struct
{
	char iscp[4];
	uint32_t hdr_len;
	uint32_t data_len;
	char version;
	char reserved[3];
} t_iscp_hdr;


typedef struct
{
	char start_char;
	char device_code;
	char command[3];
	char mesg[1];
} t_iscp_data;


typedef struct st_entry
{
	char *key;
	char *value;
	int val_len;
	int unknown;
	struct st_entry *next;
} t_entry;


typedef struct
{
	char *name;
	char *comlist;
	int len;
	int running;
} t_macro;

/* Cmd line params */
EXTERN struct st_flags flags;
EXTERN char *ipaddr;
EXTERN char device_code;
EXTERN uint16_t udp_port;
EXTERN uint16_t tcp_port;
EXTERN int listen_timeout;
EXTERN int connect_timeout;

/* Runtime globals */
EXTERN struct termios saved_tio;
EXTERN struct sockaddr_in con_addr;
EXTERN struct st_addr addr_list[ADDR_LIST_SIZE];
EXTERN struct st_buffer buffer[NUM_BUFFERS];
EXTERN struct st_rev_com revcom[2][MAX_HIST_BUFFERS];
EXTERN t_iscp_hdr *pkt_hdr;
EXTERN t_entry *list[256];
EXTERN t_macro *macros;
EXTERN time_t start_time;
EXTERN time_t connect_time;
EXTERN time_t last_rx_time;
EXTERN time_t last_tx_time;
EXTERN time_t save_rx_time;
EXTERN int input_state;
EXTERN int save_state;
EXTERN int save_fd;
EXTERN int keyb_buffnum;
EXTERN int from_buffnum;
EXTERN int udp_sock;
EXTERN int tcp_sock;
EXTERN int addr_list_cnt;
EXTERN int last_cmd_len;
EXTERN int prompt_type;
EXTERN int menu_cursor_pos;
EXTERN int menu_option_cnt;
EXTERN int macro_cnt;
EXTERN int macro_alloc;
EXTERN int macro_append;
EXTERN int raw_level;
EXTERN int nri_command;
EXTERN int rev_arr;
EXTERN int rev_bot[2];
EXTERN int rev_top[2];
EXTERN int rev_start[2];
EXTERN size_t rx_bytes;
EXTERN size_t tx_bytes;
EXTERN u_long rx_reads;
EXTERN u_long tx_writes;
EXTERN char track_time_str[9];
EXTERN char track_len_str[9];
EXTERN char nja_prev;
EXTERN char *save_filename;
EXTERN char *menu_selection;
EXTERN char **menu_options;
EXTERN char *macro_line_tmp;
EXTERN char *macro_name;

/*** Forward declarations ***/

/* keyboard.c */
void initKeyboard(void);
void readKeyboard(void);
void resetKeyboard(void);

/* commands.c */
int  parseInputLine(char *data, int len);
int  getCommand(char *word, int len);
int  sendCommand(int repeat_cnt, char *cmd, int cmd_len);
void sortCommands(void);


/* network.c */
int  networkStart(void);
int  createUDPSocket(void);
int  getStreamerAddress(void);
int  connectToStreamer(void);
void readSocket(int print_prompt);
int  writeSocket(char *write_data, int write_data_len);
void networkClear(void);

/* printrx.c */
void printMesg(char *mesg, int len);
void prettyPrint(t_iscp_data *pkt_data, int print_prompt);
int  prettyPrintRXList(char *pat, int max);
void printRXCommands(char *pat);
void printTrackTime(int rx);

/* buffer.c */
void initBuffers(void);
void copyBuffer(int buff_from, int buff_to);
void addToBuffer(int buffnum, char *data, int data_len);
int  delLastCharFromBuffer(int buffnum);
void clearBuffer(int buffnum);

/* rxlist.c */
void initRXList(void);
int  updateRXList(char *key, char *value, int val_len);
void setUnknownRXKey(char *key);
void clearValueOfRXKey(char *key);
void clearRXList(void);
int  dumpRXList(char *pat, int max);

/* titles.c */
void initTitles(void);
void addTitle(char *mesg, uint32_t len);
int  printTitles(int xtitles, char *param, int max);
void clearTitles(void);

/* menu.c */
void initMenu(void);
void addMenuOption(char *mesg, uint32_t len);
void setMenuSelection(void);
void printMenuList(void);
void printMenuSelection(void);
void clearMenu(int prt);

/* save.c */
void initSave(void);
void resetSave(void);
void prepareSave(char *filename);
void saveArtDataLine(uint32_t data_len, t_iscp_data *pkt_data);
void checkSaveTimeout(void);

/* macros.c */
void initMacros(void);
int  initMultiLineMacro(char *name);
int  initMultiLineMacroAppend(char *name);
void discardMultiLineMacro(void);
int  insertMacro(char *name, char *comlist);
void addMacroLine(char *line, int len);
int  appendMacroComlist(char *name, char *comlist);
void appendMacroSlotComlist(int macro_num, char *comlist);
int  deleteMacro(char *name);
int  runMacro(char *name);
int  loadMacros(char *filename);
int  saveMacro(char *filename, char *name, int append);
int  saveAllMacros(char *filename, int append);
void listMacros(void);
int  findMacro(char *name);

/* printf.c */
void errPrintf(const char *fmt, ...);
void nlErrPrintf(const char *fmt, ...);
void warnPrintf(const char *fmt, ...);
void nlWarnPrintf(const char *fmt, ...);
void usagePrintf(const char *fmt, ...);
void quitPrintf(const char *fmt, ...);
void colPrintf(const char *fmt, ...);

/* prompt.c */
void  printPrompt(void);
void  clearPrompt(void);

/* strings.c */
#ifdef __linux__
char *strnstr(char *haystack, char *needle, size_t len);
#endif
int   wildMatch(char *str, char *pat);
int   isPattern(char *str);
int   isNumberWithLen(char *str, int len);
int   isNumber(char *str);

/* time.c */
char *getTime(void);
char *getTimeString(time_t tm);
char *getRawTimeString(time_t tm);

/* reverse.c */
void initReverse();
void clearReverse();
void addReverseCom(int ra, int com, int cnt);
int  getReverseCom(int com);
void runShowReverse(int run, char *param);

/* misc.c */
int   doWait(int comnum, float secs);
void  doExit(int code);
void  sigHandler(int sig);
u_int getUsecTime(void);
void  version(int print_pid);
void  errNotConnected();
void  ok(void);
