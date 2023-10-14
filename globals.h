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

#define VERSION "20231013"

#define STDIN          0
#define STDOUT         1
#define UDP_PORT       10102
#define TCP_PORT       60128
#define ALLOC_BLOCK    10
#define ADDR_LIST_SIZE 20
#define MESG_OFFSET    5
#define MESG_TERM_LEN  3
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
	LAST_CLIENT_COM = COM_MACRO,

	/* 14. Streamer commands */
	COM_MENU,

	/* 15 */
	COM_MENUSTAT,
	COM_UP,
	COM_DN,
	COM_EN,
	COM_EX,

	/* 20 */
	COM_FLIP,
	COM_DS,
	COM_DSD,
	COM_DSSTAT,
	COM_FILTER,

	/* 25 */
	COM_FILSTAT,
	COM_ARTDIS,
	COM_ARTBMP,
	COM_ARTURL,
	COM_ARTSTAT,
	COM_ARTSAVE,

	/* Enums beyond saveart not required except for these */
	COM_SETNAME = 83,
	COM_SETUP   = 89,
	COM_SERIAL,
	COM_ETHMAC,
	COM_ICONURL,
	COM_MODINFO,
	COM_TIDALVER,
	COM_ECOVER,
	COM_PRODID,
	COM_LRA     = 104
};


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
	unsigned interrupted     : 1;
	unsigned rx_menu         : 1;
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
int   isNumberWithLen(char *str, int len);
int   isNumber(char *str);

/* time.c */
char *getTime(void);
char *getTimeString(time_t tm);
char *getRawTimeString(time_t tm);

/* misc.c */
void  doExit(int code);
void  sigHandler(int sig);
void  version(int print_pid);
void  ok(void);
