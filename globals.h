#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef MAINFILE
#define EXTERN
#else
#define EXTERN extern
#endif

#define VERSION "20220109"

#define STDIN          0
#define STDOUT         1
#define UDP_PORT       10102
#define TCP_PORT       60128
#define ALLOC_BLOCK    10
#define ADDR_LIST_SIZE 20
#define MESG_OFFSET    5
#define SAVE_TIMEOUT   5
#define TIME_DEF_STR   "--:--:--"

#define FREE(M) if (M) { free(M); M = NULL; }

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


typedef struct
{
	char iscp[4];
	uint32_t hdr_len;
	uint32_t data_len;
	char version;
	char reserved[3];
} t_iscp_hdr;


struct st_flags
{
	/* User toggled flags */
	unsigned show_track_time : 1;
	unsigned trans_html_amps : 1;
	unsigned use_colour      : 1;
	unsigned macro_verbose   : 1;

	/* System flags */
	unsigned macro_running   : 1;
	unsigned offline         : 1;
	unsigned exit_after_cmds : 1;
	unsigned pretty_printing : 1;
	unsigned in_menu         : 1;
	unsigned com_up          : 1;
	unsigned com_dn          : 1;
	unsigned interrupted     : 1;
};


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
	char *data;
	int len;
	int alloc;
} t_buffer;


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
EXTERN struct in_addr addr_list[ADDR_LIST_SIZE];
EXTERN struct sockaddr_in con_addr;
EXTERN t_iscp_hdr *pkt_hdr;
EXTERN t_buffer buffer[NUM_BUFFERS];
EXTERN t_entry *list[256];
EXTERN t_macro *macros;
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
void initKeyboard();
void readKeyboard();
void resetKeyboard();

/* commands.c */
int  parseInputLine(char *data, int len);
int  getCommand(char *word, int len, int expmsg);
void sortCommands();

/* network.c */
int  networkStart();
int  createUDPSocket();
int  getStreamerAddress();
int  connectToStreamer();
void readSocket(int print_prompt);
int  writeSocket(char *write_data, int write_data_len);
void networkClear();

/* printrx.c */
void printMesg(char *mesg, int len);
void prettyPrint(t_iscp_data *pkt_data, int print_prompt);
int  prettyPrintRXList(char *pat, int max);
void printRXCommands(char *pat);
void printTrackTime();

/* buffer.c */
void initBuffers();
void copyBuffer(int buff_from, int buff_to);
void addToBuffer(int buffnum, char *data, int data_len);
int  delLastCharFromBuffer(int buffnum);
void clearBuffer(int buffnum);

/* rxlist.c */
void initRXList();
int  updateRXList(char *key, char *value, int val_len);
void setUnknownRXKey(char *key);
void clearValueOfRXKey(char *key);
void clearRXList();
int  dumpRXList(char *pat, int max);

/* titles.c */
void initTitles();
void addTitle(char *mesg, uint32_t len);
int  printTitles(int xtitles, char *param, int max);
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
void prepareSave(char *filename);
void saveArtDataLine();
void checkSaveTimeout();

/* macros.c */
void initMacros();
int  initMultiLineMacro(char *name);
int  initMultiLineMacroAppend(char *name);
void discardMultiLineMacro();
int  insertMacro(char *name, char *comlist);
void addMacroLine(char *line, int len);
int  appendMacroComlist(char *name, char *comlist);
void appendMacroSlotComlist(int macro_num, char *comlist);
int  deleteMacro(char *name);
int  runMacro(char *name);
int  loadMacros(char *filename);
int  saveMacro(char *filename, char *name, int append);
int  saveAllMacros(char *filename, int append);
void listMacros();
int  findMacro(char *name);

/* printf.c */
void errprintf(const char *fmt, ...);
void nlerrprintf(const char *fmt, ...);
void warnprintf(const char *fmt, ...);
void nlwarnprintf(const char *fmt, ...);
void usageprintf(const char *fmt, ...);
void exitprintf(const char *fmt, ...);
void colprintf(const char *fmt, ...);

/* prompt.c */
void  printPrompt();
void  clearPrompt();

/* misc.c */
int   wildMatch(char *str, char *pat);
int   isNumberWithLen(char *str, int len);
int   isNumber(char *str);
char *getTime();
char *getTimeString(time_t tm);
void  doExit(int code);
void  sigHandler(int sig);
void  version(int print_pid);
void  ok();
