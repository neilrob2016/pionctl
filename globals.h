#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <pwd.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef MAINFILE
#define EXTERN
#else
#define EXTERN extern
#endif

#define VERSION "20211211"

#define STDIN          0
#define STDOUT         1
#define ALLOC_BLOCK    10
#define READBUFF_SIZE  100
#define START_CHAR     '!'
#define MESG_OFFSET    5
#define ADDR_LIST_SIZE 20
#define MAX_WORDS      4
#define SEPARATOR      ';'

/* Cmd line defaults */
#define UDP_PORT       10102
#define TCP_PORT       60128
#define DEVICE_CODE    '1'
#define LISTEN_TIMEOUT 60
#define TIME_DEF_STR   "--:--:--"

#define FREE(M) if (M) { free(M); M = NULL; }

#define SETFLAG(F)   (flags |= F)
#define UNSETFLAG(F) (flags &= ~(uint32_t)F)
#define FLIPFLAG(F)  (flags ^= F)
#define FLAGISSET(F) (flags & F)

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
	ERR_MACRO
};


enum
{
	PROMPT_BASE,
	PROMPT_L_TIME,
	PROMPT_C_TIME,
	PROMPT_S_TIME,
	PROMPT_L_C_TIME,
	PROMPT_L_S_TIME,
	PROMPT_C_S_TIME,

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

enum
{
	FLAG_SHOW_TRACK_TIME = 1,
	FLAG_TRANS_HTML_AMPS = (1 << 1),
	FLAG_OFFLINE         = (1 << 2),
	FLAG_EXIT_AFTER_CMDS = (1 << 3),
	FLAG_PRETTY_PRINTING = (1 << 4),
	FLAG_IN_MENU         = (1 << 5),
	FLAG_COM_UP          = (1 << 6),
	FLAG_COM_DN          = (1 << 7),
	FLAG_MACRO_VERBOSE   = (1 << 8),
	FLAG_MACRO_RUNNING   = (1 << 9),
	FLAG_INTERRUPTED     = (1 << 10)
};


/* State for saving NJA jpeg */
enum 
{
	SAVE_INACTIVE = -1,
	SAVE_START,
	SAVE_NEXT
};


typedef struct
{
	char iscp[4];
	uint32_t hdr_len;
	uint32_t data_len;
	u_char version;
	u_char reserved[3];
} t_iscp_hdr;


typedef struct
{
	u_char start_char;
	u_char device_code;
	u_char command[3];
	u_char mesg[1];
} t_iscp_data;


typedef struct st_entry
{
	char *key;
	u_char *value;
	int val_len;
	int unknown;
	struct st_entry *next;
} t_entry;


typedef struct 
{
	u_char *data;
	int len;
	int alloc;
} t_buffer;


typedef struct
{
	u_char *name;
	u_char *comlist;
	int len;
	int running;
} t_macro;


/* Cmd line params */
EXTERN char *ipaddr;
EXTERN char device_code;
EXTERN uint16_t udp_port;
EXTERN uint16_t tcp_port;
EXTERN uint32_t flags;
EXTERN int listen_timeout;
EXTERN int connect_timeout;

/* Runtime globals */
EXTERN struct termios saved_tio;
EXTERN struct in_addr addr_list[ADDR_LIST_SIZE];
EXTERN struct sockaddr_in con_addr;
EXTERN t_iscp_hdr *pkt_hdr;
EXTERN t_buffer buffer[NUM_BUFFERS];
EXTERN t_entry *list[256];
EXTERN t_macro *macros;
EXTERN time_t connect_time;
EXTERN time_t last_rx_time;
EXTERN time_t last_tx_time;
EXTERN int input_state;
EXTERN int keyb_buffnum;
EXTERN int from_buffnum;
EXTERN int udp_sock;
EXTERN int tcp_sock;
EXTERN int addr_list_cnt;
EXTERN int last_cmd_len;
EXTERN int save_stage;
EXTERN int save_fd;
EXTERN int prompt_type;
EXTERN int menu_cursor_pos;
EXTERN int menu_option_cnt;
EXTERN int macro_cnt;
EXTERN int macro_alloc;
EXTERN int macro_append;
EXTERN int raw_level;
EXTERN u_long rx_bytes;
EXTERN u_long rx_reads;
EXTERN u_long tx_bytes;
EXTERN u_long tx_writes;
EXTERN char track_time_str[9];
EXTERN char track_len_str[9];
EXTERN char nja_prev;
EXTERN char *save_filename;
EXTERN char *menu_selection;
EXTERN char **menu_options;
EXTERN char *macro_line_tmp;
EXTERN u_char *macro_name;

/*** Forward declarations ***/

/* keyboard.c */
void initKeyboard();
void readKeyboard();
void resetKeyboard();

/* commands.c */
int  parseInputLine(u_char *data, int len);
void sortCommands();
int  getCommand(char *word, int len, int expmsg);

/* network.c */
int  networkStart();
int  createUDPSocket();
int  getStreamerAddress();
int  connectToStreamer();
void readSocket(int print_prompt);
int  writeSocket(u_char *write_data, int write_data_len);
void networkClear();

/* printrx.c */
void printMesg(u_char *mesg, int len);
void prettyPrint(t_iscp_data *pkt_data, int print_prompt);
int  prettyPrintList(u_char *pat, int max);
void printRXCommands(u_char *pat);
void printTrackTime();

/* buffer.c */
void initBuffers();
void copyBuffer(int buff_from, int buff_to);
void addToBuffer(int buffnum, char *data, int data_len);
int  delLastCharFromBuffer(int buffnum);
void clearBuffer(int buffnum);

/* list.c */
void initList();
int  updateList(char *key, u_char *value, int val_len);
void setEntryValue(t_entry *entry, u_char *value, int val_len);
void setUnknownKey(char *key);
void clearValueOfKey(char *key);
void clearList();
int  dumpList(u_char *pat, int max);
t_entry *findInList(char *key);

/* titles.c */
void initTitles();
void addTitle(u_char *mesg, uint32_t len);
int  printTitles(int xtitles, u_char *param, int max);
void clearTitles();

/* menu.c */
void initMenu();
void addMenuOption();
void setMenuSelection();
void printMenuList();
void printMenuSelection();
void clearMenu();

/* save.c */
void initSave();
void resetSave();
void startSave(char *filename);
void saveArtDataLine();

/* macros.c */
void initMacros();
int  initMultiLineMacro(u_char *name);
int  initMultiLineMacroAppend(u_char *name);
void discardMultiLineMacro();
int  addMacro(u_char *name, u_char *comlist);
void addMacroLine(u_char *line, int len);
int  appendMacro(u_char *name, u_char *comlist);
void appendMacroComlist(int macro_num, u_char *comlist);
int  deleteMacro(u_char *name);
int  clearMacros();
int  runMacro(u_char *name);
int  loadMacros(u_char *filename);
int  saveMacro(u_char *filename, u_char *name, int append);
int  saveAllMacros(u_char *filename, int append);
void listMacros();
int  findMacro(u_char *name);

/* misc.c */
int   wildMatch(char *str, char *pat);
int   isNumberWithLen(u_char *str, int len);
int   isNumber(char *str);
char *getTime();
char *getTimeString(time_t tm);
void  printPrompt();
void  clearPrompt();
void  doExit(int code);
void  sigHandler(int sig);
void  version(int print_pid);
