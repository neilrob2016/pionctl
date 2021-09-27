#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef MAINFILE
#define EXTERN
#else
#define EXTERN extern
#endif

#define VERSION "20210608"

#define STDIN          0
#define STDOUT         1
#define ALLOC_BLOCK    10
#define READBUFF_SIZE  100
#define START_CHAR     '!'
#define MESG_OFFSET    5
#define ADDR_LIST_SIZE 255
#define MAX_WORDS      4
#define CMD_SEPARATOR  ';'

/* Cmd line defaults */
#define UDP_PORT       10102
#define TCP_PORT       60128
#define DEVICE_CODE    '1'
#define LISTEN_TIMEOUT 60
#define TIMER_STR_DEF  "--:--:--"

#define FREE(M) if (M) free(M)

#define SETFLAG(F)   (flags |= F)
#define UNSETFLAG(F) (flags &= ~(uint32_t)F)
#define FLIPFLAG(F)  (flags ^= F)

#ifdef __APPLE__
/* Don't bitch about pragma pack every time it sees it */
#pragma clang diagnostic ignored "-Wpragma-pack"
#endif
#pragma pack(1)


enum
{
	BUFF_KEYB_FIRST,
	BUFF_KEYB_LAST = 22,
	BUFF_TCP,

	NUM_BUFFERS
};


enum
{
	CMD_MISSING,
	CMD_ERROR,
	CMD_OK
};


enum
{
	PROMPT_BASE,
	PROMPT_TIME,
	PROMPT_CONN_TIMER,
	PROMPT_STRM_TIMER,

	NUM_PROMPTS
};

#define MAX_HIST_BUFFERS BUFF_TCP


enum
{
	FLAG_SHOW_DETAIL     = 1,
	FLAG_SHOW_TIMER      = (1 << 1),
	FLAG_SHOW_RAW        = (1 << 2),
	FLAG_TRANS_HTML_AMPS = (1 << 3),
	FLAG_OFFLINE         = (1 << 4),
	FLAG_EXIT_AFTER_CMDS = (1 << 5),
	FLAG_PRETTY_PRINTING = (1 << 6),
	FLAG_IN_MENU         = (1 << 7),
	FLAG_COM_UP          = (1 << 8),
	FLAG_COM_DN          = (1 << 9)
};


typedef struct
{
	char iscp[4];
	uint32_t hdr_len;
	uint32_t data_len;
	u_char version;
	u_char reserved[3];
} t_iscp_hdr;

#define PKT_HDR_LEN (int)sizeof(t_iscp_hdr)


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


/* State for saving NJA jpeg */
enum 
{
	SAVE_INACTIVE = -1,
	SAVE_START,
	SAVE_NEXT
};

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
EXTERN time_t connect_time;
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
EXTERN char *save_filename;
EXTERN char timer_str[9];
EXTERN char nja_prev;
EXTERN char *menu_selection;
EXTERN char **menu_options;

/*** Forward declarations ***/

/* keyboard.c */
void initKeyboard();
void readKeyboard();
void parseUserInput();
void resetKeyboard();

/* commands.c */
void sortCommands();
int  parseCommand(u_char *buff, int bufflen);

/* network.c */
int  networkStart();
int  createUDPSocket();
int  getStreamerAddress();
int  connectToStreamer();
void readSocket();
int  writeSocket(u_char *write_data, int write_data_len);
void networkClear();

/* print.c */
void printMesg(u_char *mesg, int len);
void prettyPrint(t_iscp_data *pkt_data);
int  prettyPrintList(u_char *pat, int max);
void printRXCommands(u_char *pat);
void printTimes();

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
void printMenu();
void printMenuInfo();
void clearMenu();

/* save.c */
void initSave();
void resetSave();
void startSave(char *filename);
void saveArtDataLine();

/* misc.c */
int   wildMatch(char *str, char *pat);
int   isNumber(u_char *str, int len);
void  printPrompt();
void  clearPrompt();
char *getConnectTime();
char *getTime();
void  doExit(int code);
void  sigHandler(int sig);
void  version(int print_pid);
