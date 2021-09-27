#include "globals.h"

#define CMD_END_LEN 3
#define MAX_HEX_LEN 1024

void initSave()
{
	save_stage = SAVE_INACTIVE;
	save_fd = 0;
	save_filename = NULL;
}




void resetSave()
{
	if (save_stage != SAVE_INACTIVE)
	{
		if (save_fd) close(save_fd);
		free(save_filename);
	}
	initSave();
}




void startSave(char *filename)
{
	if (save_stage != SAVE_INACTIVE) resetSave();
	save_filename = strdup(filename ? filename : "art");
	save_stage = SAVE_START;
}




void saveArtDataLine(uint32_t data_len, t_iscp_data *pkt_data)
{
	u_char *cmd = pkt_data->command + 3;
	u_char *ptr;
	u_char *end;
	u_char c;
	u_char byte;
	char *tmp;
	char *ext;
	int len;

	data_len -= 3;
	if (data_len < 3 || data_len > MAX_HEX_LEN)
	{
		if (data_len > 1 && cmd[0] == 'n')
			puts("\nArtwork is not available.");
		else
			puts("\nERROR: Bad art data.");
		goto DONE;
	}

	/* Switch on image send stage */
	switch(cmd[1])
	{
	case '0':
		if (save_stage > SAVE_START)
		{
			printf("\nERROR: Invalid state %c\n",cmd[1]);
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
				printf("\nERROR: Invalid image format %c\n",cmd[0]);
				goto DONE;
			}
			asprintf(&tmp,"%s.%s",save_filename,ext);
			free(save_filename);
			save_filename = tmp;
		}

		printf("\nSaving to '%s'...\n",save_filename);
		if ((save_fd = open(
			save_filename,O_WRONLY|O_CREAT|O_TRUNC,0666)) == -1)
		{
			printf("ERROR: saveArtDataLine(): open(): %s\n",
				strerror(errno));
			goto DONE;
		}
		save_stage = SAVE_NEXT;
		break;

	case '1':		
		if (save_stage != SAVE_NEXT)
		{
			printf("\nERROR: Invalid state %c",cmd[1]);
			goto DONE;
		}
		break;

	case '2':
		break;

	default:
		if (!strncmp((char *)cmd,"BMP",3)) return;
		printf("\nERROR: Invalid state %c",cmd[1]);
		goto DONE;
	}

	/* Convert hex data to binary and write to file */
	end = cmd + data_len;
	for(ptr=cmd+2;ptr < end;ptr+=2)
	{
		c = ptr[2];
		ptr[2] = 0;
		if (!(byte = (u_char)strtol((char *)ptr,NULL,16)) && 
		    errno == EINVAL)
		{
			goto DONE;
		}
		if (write(save_fd,&byte,1) != 1) 
		{
			printf("\nERROR: saveArt(): write(): %s\n",strerror(errno));
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
