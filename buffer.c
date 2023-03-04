#include "globals.h"

void initBuffers()
{
	bzero(buffer,sizeof(buffer));
}




void copyBuffer(int buff_from, int buff_to)
{
	int datalen = buffer[buff_from].len;

	if (buffer[buff_from].alloc > buffer[buff_to].alloc)
	{
		buffer[buff_to].data = (char *)realloc(
			buffer[buff_to].data,datalen);
		buffer[buff_to].alloc = buffer[buff_from].alloc;
	}
	memcpy(buffer[buff_to].data,buffer[buff_from].data,datalen);
	buffer[buff_to].data[datalen] = 0;
	buffer[buff_to].len = datalen;
}




void addToBuffer(int buffnum, char *data, int data_len)
{
	int len;
	int alloc;
	int mult;

	len = buffer[buffnum].len + data_len;
	alloc = buffer[buffnum].alloc;

	if (len >= alloc)
	{
		mult = (len - alloc) / ALLOC_BLOCK + 1;
		alloc += (ALLOC_BLOCK * mult);
		if (!(buffer[buffnum].data = (char *)realloc(
			buffer[buffnum].data,alloc+1)))
		{
			/* Don't use errPrintf as colPrintf allocs too */
			perror("ERROR: realloc()");
			doExit(1);
		}
		buffer[buffnum].alloc = alloc;
		if (buffnum == BUFF_TCP)
			pkt_hdr = (t_iscp_hdr *)buffer[BUFF_TCP].data;
	}
	memcpy(buffer[buffnum].data+buffer[buffnum].len,data,data_len);
	buffer[buffnum].data[len] = 0;
	buffer[buffnum].len = len;
}




int delLastCharFromBuffer(int buffnum)
{
	if (buffer[buffnum].len)
	{
		buffer[buffnum].data[--buffer[buffnum].len] = 0;
		return 1;
	}
	return 0;
}




/*** Don't reset alloc - we never release buffer memory allocated ***/
void clearBuffer(int buffnum)
{
	buffer[buffnum].len = 0;
	if (buffer[buffnum].data) buffer[buffnum].data[0] = 0;
}
