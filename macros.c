#include "globals.h"

#define MAX_LINE_LEN 2000
#define WRAP_COL     79
#define INDENT       14
#define ENTER_STR    "Enter macro lines, end with a '.' on its own..."
#define CMDERR_STR   "ERROR: Macros cannot have the same name or substring as a command."

static char *home_dir;

int     findEmptySlot();
int     writeMacro(FILE *fp, int m, int append);
int     clearMacro(int m);
u_char *firstNonWhitespacePos(u_char *str);
u_char *trim(u_char *name);
char   *tildaToHomeDir(u_char *filename);


void initMacros()
{
	struct passwd *ud;

	macros = NULL;
	macro_cnt = 0;
	macro_alloc = 0;
	macro_line_tmp = NULL;
	macro_name = NULL;

	/* Get the home directory in order to substitute '~' in filenames */
	ud = getpwuid(getuid());
	home_dir = ud->pw_dir ? strdup(ud->pw_dir) : "";
}




int initMultiLineMacro(u_char *name)
{
	if (!(name = trim(name)))
	{
		puts("ERROR: Empty macro name.");
		return ERR_MACRO;
	}
	if (getCommand((char *)name,strlen((char *)name),0) != -1)
	{
		puts(CMDERR_STR);
		return ERR_MACRO;
	}
	macro_name = (u_char *)strdup((char *)name);
	assert(macro_name);
	macro_line_tmp = NULL;
	input_state = INPUT_MACRO_DEF;
	puts(ENTER_STR);
	return OK;
}




int initMultiLineMacroAppend(u_char *name)
{
	if ((macro_append = findMacro(name)) == -1)
	{
		printf("ERROR: Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	macro_line_tmp = NULL;
	input_state = INPUT_MACRO_APP;
	puts(ENTER_STR);
	return OK;
}




void discardMultiLineMacro()
{
	switch(input_state)
	{
	case INPUT_MACRO_DEF:
		FREE(macro_name);
		FREE(macro_line_tmp);
		break;
	case INPUT_MACRO_APP:
		break;
	}
	input_state = INPUT_CMD;
}




/*** Insert a macro into the macros array ***/
int insertMacro(u_char *name, u_char *comlist)
{
	t_macro *macro;
	int m;

	if (!(name = trim(name)))
	{
		puts("ERROR: Empty macro name.");
		return ERR_MACRO;
	}
	if (!(comlist = trim(comlist))) 
	{
		puts("ERROR: Empty command list.");
		return ERR_MACRO;
	}
	if (findMacro(name) != -1)
	{
		printf("ERROR: Duplicate macro name \"%s\".\n",name);
		return ERR_MACRO;
	}
	if (getCommand((char *)name,strlen((char *)name),0) != -1)
	{
		puts(CMDERR_STR);
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
	macro->name = (u_char *)strdup((char *)name);
	macro->comlist = (u_char *)strdup((char *)comlist);
	assert(macro->name && macro->comlist);
	macro->len = strlen((char *)comlist);
	macro->running = 0;
	printf("Macro \"%s\" added.\n",name);
	return OK;
}




/*** Add or append a command line to a macro ***/
void addMacroLine(u_char *line, int len)
{
	u_char *ptr;
	u_char *end;
	char *tmp;
	int ret;

	/* See if we have anything */
	end = line + len;
	for(ptr=line;ptr < end && *ptr < 33;++ptr);
	if (ptr == end) return;

	if (*line == '.' && *(line+1) < 33)
	{
		/* Done */
		if (!macro_line_tmp) puts("ERROR: Empty macro.");
		else switch(input_state)
		{
		case INPUT_MACRO_DEF:
			insertMacro(macro_name,(u_char *)macro_line_tmp);
			break;
		case INPUT_MACRO_APP:
			appendMacroSlotComlist(macro_append,(u_char *)macro_line_tmp);
			macro_append = -1;
			break;
		default:
			assert(0);
		}
		FREE(macro_line_tmp);
		FREE(macro_name);
		input_state = INPUT_CMD;
		return;
	}
	if (macro_line_tmp)
	{
		ret = asprintf(&tmp,"%s; %.*s",macro_line_tmp,len,line);
		free(macro_line_tmp);
		macro_line_tmp = (char *)tmp;
	}
	else ret = asprintf(&macro_line_tmp,"%.*s",len,line);
	assert(ret != -1);
}




/*** Append the command list to the given macro ***/
int appendMacroComlist(u_char *name, u_char *comlist)
{
	int m;

	if ((m = findMacro(name)) == -1)
	{
		printf("ERROR: Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	if (!(comlist = trim(comlist))) 
	{
		puts("ERROR: Empty command list.");
		return ERR_MACRO;
	}
	appendMacroSlotComlist(m,comlist);
	return OK;
}




/*** Append commands to a macro command list in the given slot ***/
void appendMacroSlotComlist(int m, u_char *comlist)
{
	t_macro *macro;
	int len;

	macro = &macros[m];
	len = macro->len + strlen((char *)comlist) + 2; /* +2 for "; " */
	macro->comlist = (u_char *)realloc(macro->comlist,len + 1); 
	assert(macro->comlist);
	strcat((char *)macro->comlist,"; ");
	strcat((char *)macro->comlist,(char *)comlist);
	macro->len = len;
	printf("Macro \"%s\" appended.\n",macro->name);
}




int deleteMacro(u_char *name)
{
	int m;

	if ((m = findMacro(name)) == -1)
	{
		printf("ERROR: Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	if (clearMacro(m))
	{
		printf("Macro \"%s\" deleted.\n",name);
		return OK;
	}
	return ERR_MACRO;
}




int clearMacros()
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
	printf("%d macros deleted.\n",cnt);
	return (cnt == macro_cnt) ? OK : ERR_MACRO;
}




int runMacro(u_char *name)
{
	static int recurse = 0;
	t_macro *macro;
	int ret;
	int m;

	if ((m = findMacro(name)) == -1)
	{
		printf("ERROR: Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	macro = &macros[m];

	/* Can't recurse into the same macro or we'll end up in an endless
	   loop */
	if (macro->running)
	{
		puts("ERROR: Macro recursion.");
		return ERR_MACRO;
	}

	SETFLAG(FLAG_MACRO_RUNNING);
	recurse++;
	macro->running = 1; 

	printf("Running macro: \"%s\"\n",name);
	ret = parseInputLine(macros[m].comlist,macros[m].len);

	macro->running = 0;
	if (!--recurse) UNSETFLAG(FLAG_MACRO_RUNNING);

	if (ret != OK) printf("ERROR: Macro \"%s\" FAILED\n",macro->name);
	return ret;
}




/*** Load all macros from the given file ***/
int loadMacros(u_char *filename)
{
	/* Hopefully these are big enough. If not it'll read the rest of the
	   commands as the next name etc. Too bad */
	char name[MAX_LINE_LEN];
	char line[MAX_LINE_LEN];
	char *path;
	u_char *ptr;
	FILE *fp;
	int cnt;
	int get_name;

	path = tildaToHomeDir(filename);
	printf("Loading from file \"%s\"...\n",path);
	fp = fopen(path,"r");
	free(path);
	if (!fp)
	{
		printf("ERROR: Can't open file to read: %s\n",strerror(errno));
		return ERR_MACRO;
	}

	get_name = 1;
	fgets(line,sizeof(line),fp);
	for(cnt=0;!feof(fp);)
	{
		/* Skip empty lines and comments. Comments have to be added
		   manually to the macro file and arn't saved */
		if ((ptr = firstNonWhitespacePos((u_char *)line)))
		{
			if (*ptr == '#')
			{
				ptr = trim(ptr+1);
				if (*ptr) printf("Comment: %s\n",ptr);
			}
			else
			{
				if (get_name)
					strcpy(name,line);
				else
					cnt += (insertMacro((u_char *)name,(u_char *)line) == OK);
				get_name = !get_name;
			}
		}
		fgets(line,sizeof(line),fp);
	} 
	fclose(fp);

	printf("%d macros loaded.\n",cnt);
	return OK;
}




/*** Save a single macro to a file ***/
int saveMacro(u_char *filename, u_char *name, int append)
{
	FILE *fp;
	char *path;
	int ret;
	int m;

	if ((m = findMacro(name)) == -1)
	{
		printf("ERROR: Macro \"%s\" does not exist.\n",name);
		return ERR_MACRO;
	}
	path = tildaToHomeDir(filename);
	printf("Writing to file \"%s\"...\n",path);
	fp = fopen(path,append ? "a" : "w");
	free(path);
	if (!fp)
	{
		printf("ERROR: Can't open file to write: %s\n",strerror(errno));
		return ERR_MACRO;
	}
	ret = writeMacro(fp,m,append);
	fclose(fp);
	return ret;
}




int saveAllMacros(u_char *filename, int append)
{
	FILE *fp;
	char *path;
	int cnt;
	int m;

	if (!macro_cnt)
	{
		puts("No macros defined.");
		return OK;
	}
	path = tildaToHomeDir(filename);
	printf("Writing to file \"%s\"...\n",path);
	fp = fopen(path,append ? "a" : "w");
	free(path);
	if (!fp)
	{
		printf("ERROR: Can't open file to write: %s\n",strerror(errno));
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




void listMacros()
{
	u_char *ptr;
	u_char *ptr2;
	int cnt;
	int wlen;
	int m;
	int x;

	if (!macro_cnt)
	{
		puts("No macros defined.");
		return;
	}

	puts("\n*** Macros ***\n");
	puts("Name          Commands");
	puts("----          --------");
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




int findMacro(u_char *name)
{
	int m;
	for(m=0;m < macro_cnt && 
	        (!macros[m].name ||
	         strcmp((char *)name,(char *)macros[m].name));++m);
	return (m < macro_cnt ? m : -1);
}


/********************************* SUPPORT ***********************************/

int findEmptySlot()
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
		printf("ERROR: Write failed: %s\n",strerror(errno));
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
		printf("ERROR: Cannot delete running macro \"%s\".\n",
			macros[m].name);
		return 0;
	}
	if (macros[m].name)
	{
		FREE(macros[m].name);
		FREE(macros[m].comlist);
		ret = 1;
	}
	else ret = 0;

	bzero(&macros[m],sizeof(t_macro));
	return ret;
}




u_char *firstNonWhitespacePos(u_char *str)
{
	for(;*str && *str < 33;++str);
	return *str ? str : NULL;
}



u_char *trim(u_char *str)
{
	u_char *end;

	/* Trim start */
	if ((str = firstNonWhitespacePos(str)))
	{
		/* Trim end */
		for(end=str+strlen((char *)str)-1;end > str && *end < 33;--end);
		*++end = 0;
	}
	return str;
}




char *tildaToHomeDir(u_char *filename)
{
	char *new_filename;
	int ret;

	filename = trim(filename);
	if (filename[0] == '~')
	{
		ret = asprintf(&new_filename,
			"%s/%s",home_dir,filename+(filename[1] == '/' ? 2 : 1));
		assert(ret != -1);
	}
	else
	{
		new_filename = strdup((char *)filename);
		assert(new_filename);
	}
	return new_filename;
}
