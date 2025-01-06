#include "globals.h"

#define MAX_LINE_LEN 2000
#define MAX_BAD      5
#define WRAP_COL     79
#define INDENT       14
#define ENTER_STR    "Enter macro lines, end with a '.' on its own..."
#define CMDERR_STR   "Macros cannot have the same name or substring as a command.\n"

typedef struct
{
	char *name;
	char *comlist;
	int len;
	int running; 
} t_macro;

static t_macro *macros;
static char *home_dir;
static char *macro_line_tmp;
static char *macro_name;

int   deleteAllMacros(void);
int   findEmptySlot(void);
int   writeMacro(FILE *fp, int m, int append);
int   clearMacro(int m);
char *firstNonWhitespacePos(char *str);
char *trim(char *name);
int   validName(char *name);
int   expandPath(char *search_path, char *found_path);
int   hasWildCards(char *str);


void initMacros(void)
{
	struct passwd *pwd;

	macros = NULL;
	macro_cnt = 0;
	macro_alloc = 0;
	macro_line_tmp = NULL;
	macro_name = NULL;

	/* Get the home directory in order to substitute '~' in filenames */
	pwd = getpwuid(getuid());
	home_dir = (pwd && pwd->pw_dir) ? strdup(pwd->pw_dir) : NULL;
}




int initMultiLineMacro(char *name)
{
	if (!(name = trim(name)))
	{
		errPrintf("Empty macro name.\n");
		return ERR_MACRO;
	}
	if (!validName(name))
	{
		errPrintf("Invalid macro name.\n");
		return ERR_MACRO;
	}
	if (findMacro(name) != -1)
	{
		errPrintf("Macro \"%s\" already exists.\n",name);
		return ERR_MACRO;
	}
	/* Check the new macro name isn't already used */
	if (getCommand(name,strlen(name)) != -1)
	{
		errPrintf(CMDERR_STR);
		return ERR_MACRO;
	}
	macro_name = strdup(name);
	assert(macro_name);
	macro_line_tmp = NULL;
	input_state = INPUT_MACRO_DEF;
	puts(ENTER_STR);
	return OK;
}




int initMultiLineMacroAppend(char *name)
{
	if ((macro_append = findMacro(name)) == -1)
	{
		errPrintf("Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	macro_line_tmp = NULL;
	input_state = INPUT_MACRO_APP;
	puts(ENTER_STR);
	return OK;
}




void discardMultiLineMacro(void)
{
	switch(input_state)
	{
	case INPUT_MACRO_DEF:
		FREEIF(macro_name);
		FREEIF(macro_line_tmp);
		break;
	case INPUT_MACRO_APP:
		break;
	}
	input_state = INPUT_CMD;
}




/*** Insert a macro into the macros array ***/
int insertMacro(char *name, char *comlist)
{
	t_macro *macro;
	int m;

	if (!(name = trim(name)))
	{
		errPrintf("Empty macro name.\n");
		return ERR_MACRO;
	}
	if (!validName(name))
	{
		errPrintf("Invalid macro name.\n");
		return ERR_MACRO;
	}
	if (findMacro(name) != -1)
	{
		errPrintf("Macro \"%s\" already exists.\n",name);
		return ERR_MACRO;
	}
	if (!(comlist = trim(comlist))) 
	{
		errPrintf("Empty command list.\n");
		return ERR_MACRO;
	}
	/* Check the new macro name isn't already used */
	if (getCommand(name,strlen(name)) != -1)
	{
		errPrintf(CMDERR_STR);
		return ERR_MACRO;
	}
	if ((m = findEmptySlot()) == -1)
	{
		/* Allocate some structs */
		if (macro_cnt >= macro_alloc)
		{
			macro_alloc += ALLOC_BLOCK;
			macros = (t_macro *)realloc(macros,macro_alloc * sizeof(t_macro));
			assert(macros);
		}
		m = macro_cnt++;
	}
	macro = &macros[m];
	macro->name = strdup(name);
	macro->comlist = strdup(comlist);
	assert(macro->name && macro->comlist);
	macro->len = strlen(comlist);
	macro->running = 0;
	colPrintf("Macro \"%s\" ~FGadded.\n",name);
	return OK;
}




/*** Add or append a command line to a macro ***/
void addMacroLine(char *line, int len)
{
	char *ptr;
	char *end;
	char *tmp;
	int ret;

	/* See if we have anything */
	end = line + len;
	for(ptr=line;ptr < end && *ptr < 33;++ptr);
	if (ptr == end) return;

	if (*line == '.' && *(line+1) < 33)
	{
		/* Done */
		if (!macro_line_tmp) errPrintf("Empty macro.\n");
		else switch(input_state)
		{
		case INPUT_MACRO_DEF:
			insertMacro(macro_name,macro_line_tmp);
			break;
		case INPUT_MACRO_APP:
			appendMacroSlotComlist(macro_append,macro_line_tmp);
			macro_append = -1;
			break;
		default:
			assert(0);
		}
		FREEIF(macro_line_tmp);
		FREEIF(macro_name);
		input_state = INPUT_CMD;
		return;
	}
	if (macro_line_tmp)
	{
		ret = asprintf(&tmp,"%s; %.*s",macro_line_tmp,len,line);
		assert(ret >= 0);
		FREE(macro_line_tmp);
		macro_line_tmp = tmp;
	}
	else
	{
		ret = asprintf(&macro_line_tmp,"%.*s",len,line);
		assert(ret >= 0);
	}
}




/*** Append the command list to the given macro ***/
int appendMacroComlist(char *name, char *comlist)
{
	int m;

	if ((m = findMacro(name)) == -1)
	{
		errPrintf("Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	if (!(comlist = trim(comlist))) 
	{
		errPrintf("Empty command list.\n");
		return ERR_MACRO;
	}
	appendMacroSlotComlist(m,comlist);
	return OK;
}




/*** Append commands to a macro command list in the given slot ***/
void appendMacroSlotComlist(int m, char *comlist)
{
	t_macro *macro;
	int len;

	macro = &macros[m];
	len = macro->len + strlen(comlist) + 2; /* +2 for "; " */
	macro->comlist = (char *)realloc(macro->comlist,len + 1); 
	assert(macro->comlist);
	strcat(macro->comlist,"; ");
	strcat(macro->comlist,comlist);
	macro->len = len;
	printf("Macro \"%s\" appended.\n",macro->name);
}




int deleteMacro(char *name)
{
	int m;

	if (!strcmp(name,"*")) return deleteAllMacros();
	if ((m = findMacro(name)) == -1)
	{
		errPrintf("Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	if (clearMacro(m))
	{
		colPrintf("Macro \"%s\" ~FRdeleted.\n",name);
		return OK;
	}
	return ERR_MACRO;
}




int deleteAllMacros(void)
{
	int m;
	int cnt = 0;

	if (macros)
	{
		for(m=0;m < macro_cnt;++m) cnt += clearMacro(m);
		if (cnt == macro_cnt)
		{
			free(macros);
			initMacros();
		}
	}
	colPrintf("%d macros ~FRdeleted.\n",cnt);
	return (cnt == macro_cnt) ? OK : ERR_MACRO;
}




int runMacro(char *name)
{
	static int recurse = 0;
	t_macro *macro;
	int ret;
	int m;

	if ((m = findMacro(name)) == -1)
	{
		errPrintf("Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	macro = &macros[m];

	/* Can't recurse into the same macro or we'll end up in an endless
	   loop */
	if (macro->running)
	{
		errPrintf("Macro recursion.\n");
		return ERR_MACRO;
	}

	flags.macro_running = 1;
	recurse++;
	macro->running = 1; 

	if (flags.verbose) colPrintf("~FMRunning macro:~RS \"%s\"\n",name);
	ret = parseInputLine(macros[m].comlist,macros[m].len);

	macro->running = 0;
	if (!--recurse) flags.macro_running = 0;

	if (ret != OK && flags.on_error_stop)
	{
		errPrintf("Macro \"%s\" ~FRFAILED.\n",macro->name);
		return ERR_MACRO;
	}
	return OK;
}




/*** Load all macros from the given file ***/
int loadMacros(char *filename)
{
	struct stat st;
	/* Hopefully these are big enough. If not it'll read the rest of the
	   commands as the next name etc. Too bad */
	char name[MAX_LINE_LEN];
	char line[MAX_LINE_LEN];
	char path[PATH_MAX];
	char *ptr;
	FILE *fp;
	int get_name;
	int good;
	int bad;

	if (!expandPath(filename,path)) return ERR_MACRO;

	printf("Loading from file \"%s\"...\n",path);

	/* Annoyingly fopen() will happily open a directory to read if in
	   read only mode. Don't want to use r+ as it'll fail if a file doesn't
	   have write permission so have to fuck about with lstat() */
	if (lstat(path,&st) == -1)
	{
		errPrintf("Can't stat file: %s\n",strerror(errno));
		return ERR_MACRO;
	}
	if (!S_ISREG(st.st_mode))
	{
		errPrintf("Not a regular file.\n");
		return ERR_MACRO;
	}
	fp = fopen(path,"r");
	if (!fp)
	{
		errPrintf("Can't open file to read: %s\n",strerror(errno));
		return ERR_MACRO;
	}

	get_name = 1;
	fgets(line,sizeof(line),fp);
	for(good=bad=0;!feof(fp);)
	{
		/* Skip empty lines and comments. Comments have to be added
		   manually to the macro file and arn't saved */
		if ((ptr = firstNonWhitespacePos(line)))
		{
			if (*ptr == '#')
			{
				ptr = trim(ptr+1);
				if (*ptr) colPrintf("~FMComment:~RS %s\n",ptr);
			}
			else
			{
				if (get_name) strcpy(name,line);
				else if (insertMacro(name,line) == OK)
					++good;
				else if (++bad == MAX_BAD)
				{
					errPrintf("Too many load errors.\n");
					return ERR_MACRO;
				}
				get_name = !get_name;
			}
		}
		fgets(line,sizeof(line),fp);
	} 
	fclose(fp);

	printf("%d macros loaded, %d bad.\n",good,bad);
	return OK;
}




/*** Save a single macro to a file ***/
int saveMacro(char *filename, char *name, int append)
{
	FILE *fp;
	char path[PATH_MAX];
	int ret;
	int m;

	if ((m = findMacro(name)) == -1)
	{
		errPrintf("Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	if (!expandPath(filename,path)) return ERR_MACRO;

	if (hasWildCards(path))
	{
		errPrintf("Invalid file name.\n");
		return ERR_MACRO;
	}

	printf("Writing to file \"%s\"...\n",path);
	fp = fopen(path,append ? "a" : "w");
	if (!fp)
	{
		errPrintf("Can't open file to write: %s\n",strerror(errno));
		return ERR_MACRO;
	}
	ret = writeMacro(fp,m,append);
	fclose(fp);
	return ret;
}




int saveAllMacros(char *filename, int append)
{
	FILE *fp;
	char path[PATH_MAX];
	int cnt;
	int m;

	if (!macro_cnt)
	{
		puts("No macros defined.");
		return OK;
	}
	if (!expandPath(filename,path)) return ERR_MACRO;

	if (hasWildCards(path))
	{
		errPrintf("Invalid file name.\n");
		return ERR_MACRO;
	}

	printf("Writing to file \"%s\"...\n",path);
	fp = fopen(path,append ? "a" : "w");
	if (!fp)
	{
		errPrintf("Can't open file to write: %s\n",strerror(errno));
		return ERR_MACRO;
	}

	for(m=cnt=0;m < macro_cnt;++m)
	{
		if (macros[m].name)
		{
			if (writeMacro(fp,m,append) == ERR_MACRO) break;
			++cnt;
		}
	}
	fclose(fp);
	printf("%d macros saved.\n",cnt);
	return OK;
}




void listMacros(void)
{
	char *ptr;
	char *ptr2;
	int cnt;
	int wlen;
	int m;
	int x;

	if (!macro_cnt)
	{
		puts("No macros defined.");
		return;
	}

	colPrintf("\n~BM~FW*** Macros ***\n\n");
	colPrintf("~FB~OLName          ~FGCommands\n");
	colPrintf("~FT----          --------\n");
	for(m=cnt=0;m < macro_cnt;++m)
	{
		if (!macros[m].name) continue;

		printf("%-12s  ",macros[m].name);

		/* Wrap to 79 characters */
		for(ptr=macros[m].comlist,x=INDENT;*ptr;++ptr,++x)
		{
			/* Find where current word ends */
			for(ptr2=ptr,wlen=0;*ptr2 > 32;++ptr2,++wlen);
			if (x + wlen > WRAP_COL)
			{
				printf("\n%*.s",INDENT,"");
				x = INDENT;
			}
			putchar(*ptr);
		}
		putchar('\n');
		++cnt;
	}
	printf("\n%d defined.\n\n",cnt);
}




int findMacro(char *name)
{
	int m;
	for(m=0;m < macro_cnt && 
	        (!macros[m].name || strcmp(name,macros[m].name));++m);
	return (m < macro_cnt ? m : -1);
}


/********************************* SUPPORT ***********************************/

int findEmptySlot(void)
{
	int m;
	for(m=0;m < macro_cnt;++m) if (!macros[m].name) return m;
	return -1;
}




int writeMacro(FILE *fp, int m, int append)
{
	/* Very simple format - name on 1st line, commands on 2nd */
	if (fprintf(fp,"%s\n",macros[m].name) == -1 ||
	    fprintf(fp,"%s\n",macros[m].comlist) == -1)
	{
		errPrintf("Write failed: %s\n",strerror(errno));
		return ERR_MACRO;
	}
	printf("Macro \"%s\" %s.\n",macros[m].name,append ? "appended" : "saved");
	return OK;
}




/*** Doesn't actually delete it, just sets it to null. Saves resizing the
     macros array. Returns 1 or 0, not OK or an error code. ***/
int clearMacro(int m)
{
	int ret;
	if (macros[m].running)
	{
		errPrintf("Cannot delete running macro \"%s\".\n",macros[m].name);
		return 0;
	}
	if (macros[m].name)
	{
		FREE(macros[m].name);
		FREEIF(macros[m].comlist);
		ret = 1;
	}
	else ret = 0;

	bzero(&macros[m],sizeof(t_macro));
	return ret;
}




char *firstNonWhitespacePos(char *str)
{
	for(;*str && *str < 33;++str);
	return *str ? str : NULL;
}



char *trim(char *str)
{
	char *end;

	/* Trim start */
	if ((str = firstNonWhitespacePos(str)))
	{
		/* Trim end */
		for(end=str+strlen(str)-1;end > str && *end < 33;--end);
		*++end = 0;
	}
	return str;
}




int validName(char *name)
{
	char *p;
	for(p=name;*p > 31 && !ispunct(*p);++p);
	return !*p;
}




/*** Expands any wildcards into full path entries ***/
int expandPath(char *search_path, char *found_path)
{
	struct passwd *pwd;
	struct dirent *ds;
	DIR *dir = NULL;
	char *start;
	char *entry;
	char *ptr;
	int add_slash = 1;
	int found = 0;
	
	errno = 0;
	if (!search_path) goto ERROR;

	found_path[0] = 0;

	switch(*search_path)
	{
	case '.':
		if (search_path[1] == '/')
			start = search_path + 2;
		else
			start = search_path;
		strcpy(found_path,".");
		break;
	case '/':
		strcpy(found_path,"/");
		start = search_path + 1;
		add_slash = 0;
		break;
	case '~':
		if (search_path[1] == '/')
		{
			if (!home_dir) goto ERROR;
			/* Its our home directory */
			strcpy(found_path,home_dir);
			start = search_path + 2;
			break;
		}

		/* Get other user home directory */
		if ((ptr = strchr(search_path,'/')))
		{
			*ptr = 0;
			start = ptr + 1;
		}
		else goto ERROR;

		pwd = getpwnam(search_path + 1);
		if (pwd && pwd->pw_dir)
		{
			strcpy(found_path,pwd->pw_dir);
			break;
		}
		goto ERROR;
	default:
		strcpy(found_path,".");
		start = search_path;
	}

	while(1)
	{
		/* Get directory entry */
		if ((ptr = strchr(start,'/'))) *ptr = 0;
		entry = start;

		/* Only search for an entry if its not the last one in the
		   path or it has wildcards. Ie it doesn't matter if the last
		   entry doesn't yet exist unless it has wildCards */
		if (ptr || hasWildCards(entry))
		{
			if (!(dir = opendir(found_path))) goto ERROR;

			/* Find entry */
			for(found=0;(ds = readdir(dir));)
			{
				if (wildMatch(ds->d_name,entry)) 
				{
					entry = ds->d_name;
					found = 1;
					goto DONE;
					break;
				}
			}
			closedir(dir);
			dir = NULL;
		}
		if (!found) goto ERROR;

		DONE:
		if (add_slash) strncat(found_path,"/",PATH_MAX);
		strncat(found_path,entry,PATH_MAX);
		/* Close it after we use ds->d_name as it becomes invalid
		   once closed */
		if (dir)
		{
			closedir(dir);
			dir = NULL;
		}
		if (!ptr) break;

		start = ptr + 1;
		add_slash = 1;
	}
	return 1;

	ERROR:
	if (errno)
	{
		if (found_path[0])
		{
			errPrintf("Invalid path from \"%s\": %s\n",
				found_path,strerror(errno));
		}
		else errPrintf("Invalid path: %s\n",strerror(errno));
	}
	else if (found_path[0])
		errPrintf("Invalid path from \"%s\".\n",found_path);
	else
		errPrintf("Invalid path.\n");
	return 0;
}




int hasWildCards(char *str)
{
	char *s;
	for(s=str;*s;++s) if (*s == '*' || *s == '?') return 1;
	return 0;
}
