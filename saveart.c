#include "globals.h"

#define CMD_END_LEN 3
#define MAX_HEX_LEN 1024

void initSave(void)
{
	save_state = SAVE_INACTIVE;
	save_fd = 0;
	save_filename = NULL;
	save_rx_time = time(0);
}




void resetSave(void)
{
	if (save_state != SAVE_INACTIVE)
	{
		if (save_fd) close(save_fd);
		free(save_filename);
	}
	initSave();
}




void prepareSave(char *filename)
{
	resetSave();
	save_filename = strdup(filename ? filename : "art");
	save_state = SAVE_START;
}




void saveArtDataLine(uint32_t data_len, t_iscp_data *pkt_data)
{
	char *cmd = pkt_data->command + 3;
	char *ptr;
	char *end;
	char c;
	char byte;
	char *tmp;
	char *ext;
	int len;

	data_len -= 3;
	if (data_len < 3 || data_len > MAX_HEX_LEN)
	{
		if (data_len > 1 && cmd[0] == 'n')
			puts("\nArtwork is not available.");
		else
			nlErrPrintf("Bad art data.\n");
		goto DONE;
	}

	/* Switch on image send stage */
	switch(cmd[1])
	{
	case '0':
		if (save_state > SAVE_START)
		{
			nlErrPrintf("Invalid state %c\n",cmd[1]);
			goto DONE;
		}

		len = strlen(save_filename);
		if (len < 4 ||
		    (strcmp(save_filename+len-4,".jpg") &&
		     strcmp(save_filename+len-4,".bmp")))
		{
			/* Get image type */
			switch(cmd[0])
			{
			case '0':
				ext = "bmp";
				break;
			case '1':
				ext = "jpg";
				break;
			default:
				nlErrPrintf("Invalid image format %c\n",cmd[0]);
				goto DONE;
			}
			asprintf(&tmp,"%s.%s",save_filename,ext);
			free(save_filename);
			save_filename = tmp;
		}

		printf("\nSaving to image file \"%s\"...\n",save_filename);
		if ((save_fd = open(
			save_filename,O_WRONLY|O_CREAT|O_TRUNC,0666)) == -1)
		{
			errPrintf("saveArtDataLine(): open(): %s\n",
				strerror(errno));
			goto DONE;
		}
		save_state = SAVE_NEXT;
		break;

	case '1':		
		if (save_state != SAVE_NEXT)
		{
			nlErrPrintf("Invalid state %c\n",cmd[1]);
			goto DONE;
		}
		break;

	case '2':
		break;

	default:
		if (!strncmp(cmd,"BMP",3)) return;
		nlErrPrintf("Invalid state %c\n",cmd[1]);
		goto DONE;
	}

	/* Convert hex data to binary and write to file */
	save_rx_time = time(0);
	end = cmd + data_len;
	for(ptr=cmd+2;ptr < end;ptr+=2)
	{
		c = ptr[2];
		ptr[2] = 0;
		if (!(byte = (char)strtol(ptr,NULL,16)) && 
		    errno == EINVAL)
		{
			goto DONE;
		}
		if (write(save_fd,&byte,1) != 1) 
		{
			nlErrPrintf("saveArt(): write(): %s\n",strerror(errno));
			goto DONE;
		}
		ptr[2] = c;
	}
	if (cmd[1] != '2')
	{
		printPrompt();
		return;
	}
	puts("\nArt/logo saved.");

	DONE:
	printPrompt();
	resetSave();
}




/*** For some reason it sometimes simply won't send us the image data ***/
void checkSaveTimeout(void)
{
	if (save_state != SAVE_INACTIVE &&
	    time(0) - save_rx_time >= SAVE_TIMEOUT)
	{
		nlWarnPrintf("Save timeout - some or all image data not received.\n");
		resetSave();
		printPrompt();
	}
}
