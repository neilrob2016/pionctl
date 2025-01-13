#include "globals.h"

int runCommandFile(char *cmdfile)
{
	static int recurse = 0;
	struct stat fs;
	char *path;
	char *home;
	char *start;
	char *end;
	char *p1;
	char *p2;
	int fd;
	int len;
	int comment;
	int error;

	/* Ten seems a reasonable amount to me. Change to suit. */
	if (recurse == 10)
	{
		errPrintf("Too many nested calls.\n");
		return ERR_CMD_FAIL;
	}
	++recurse;
	if (!cmdfile)
	{
		if ((home = getenv("HOME")))
			asprintf(&path,"%s/%s",home,RC_FILENAME);
		else
			path = strdup(RC_FILENAME);
	}
	else path = cmdfile;

	printf("Opening command file \"%s\"... ",path);
	flags.cmdfile_running = 1;
	flags.do_return = 0;
	
	if ((fd = open(path,O_RDONLY)) == -1)
	{
		if (cmdfile)
		{
			errPrintf("runCommandFile(): open(): %s\n",strerror(errno));
			return ERR_FILE;
		}
		colPrintf("~FYNot found.\n");
		--recurse;
		return OK;
	}
	if (fstat(fd,&fs) == -1)
	{
		errPrintf("runCommandFile(): fstat(): %s\n",strerror(errno));
		--recurse;
		return ERR_FILE;
	}
	if ((start = (char *)mmap(
		NULL,fs.st_size,PROT_READ,MAP_PRIVATE,fd,0)) == MAP_FAILED)
	{
		errPrintf("runCommandFile(): mmap(): %s\n",strerror(errno));
		--recurse;
		return ERR_FILE;
	}
	ok();
	close(fd);
	colPrintf("~FMRunning command file...\n");

	end = start + fs.st_size;
	comment = 0;
	error = 0;

	/* Find the lines and pass them to the executor */
	for(p1=start;p1 < end;p1=p2+1)
	{
		for(p2=p1;
		    p2 < end && *p2 != '\n' && (comment || isspace(*p2));
		    ++p2);
		if (p2 == end) break;
		if (*p2 == '\n')
		{
			if (comment)
			{
				comment = 0;
				continue;
			}
		}
		else if (*p2 == '#')
		{
			comment = 1;
			continue;
		}
		comment = 0;

		/* Got start of line, find the end */
		for(p1=p2;p2 < end && *p2 != '#' && *p2 != '\n';++p2);
		len = (int)(p2 - p1);
		comment = (*p2 == '#');

		/* Just run the command, don't print the prompt or the 
		   command itself */
		if (len)
		{
			if (parseInputLine(p1,len) != OK && 
			    flags.on_error_halt)
			{
				error = 1;
				break;
			}

			/* Stop running */
			if (flags.do_halt) break;

			/* Only stop running this command file */
			if (flags.do_return)
			{
				flags.do_return = 0;
				break;
			}
		}
	}
	munmap(start,fs.st_size);
	colPrintf("Command file run of \"%s\" %s\n",
		path,error ? "~FRFAILED" : "~FGOK");
	if (!cmdfile) free(path);
	flags.cmdfile_running = 0;
	--recurse;
	return (error ? ERR_RUN : OK);
}
