#include "globals.h"

#define READBUFF_SIZE  1000
#define COM_LEN        3
#define SAVE_TIMEOUT   5
#define MAX_ATTEMPTS   (ADDR_LIST_SIZE * 2)
#define PKT_HDR_LEN    (int)sizeof(t_iscp_hdr)
#define START_CHAR     '!'

int  resolveAddress(void);
void printPacketDetails(t_iscp_hdr *hdr, t_iscp_data *data, int rx);

int networkStart(void)
{
	char *ptr;
	int cnt;

	tcp_port = TCP_PORT;
	bzero(&con_addr,sizeof(con_addr));
	con_addr.sin_family = AF_INET;
	con_addr.sin_port = htons(tcp_port);
	connect_time = 0;
	last_rx_time = 0;
	last_tx_time = 0;
	tcp_sock = 0;
	udp_sock = 0;
	rx_bytes = 0;
	rx_reads = 0;
	tx_bytes = 0;
	tx_writes = 0;
	nja_prev = 0;

	if (ipaddr)
	{
		if ((ptr = strchr(ipaddr,':')))
		{
			if ((tcp_port = atoi(ptr+1)) < 1)
			{
				errPrintf("Invalid port number.\n");
				return 0;
			}
			*ptr = 0;
		}
		if (!resolveAddress()) return 0;
		if (!connectToStreamer())
		{
			errPrintf("Connect failed.\n");
			return 0;
		}
	}
	else
	{
		if (!createUDPSocket()) return 0;
		for(cnt=0;cnt < MAX_ATTEMPTS;++cnt)
		{
			if (cnt) printf("Retry %d...\n",cnt);

			if (!getStreamerAddress())
			{
				errPrintf("Listen failed.\n");
				networkClear();
				return 0;
			}
			if (connectToStreamer()) break;
		}

		if (cnt == MAX_ATTEMPTS) 
		{
			errPrintf("Max connection attempts reached.\n");
			networkClear();
			return 0;
		}
		close(udp_sock);
		udp_sock = 0;
	}
	initRXList();
	return 1;
}




/*** Create the socket to listen for EZPROXY packets on ***/
int createUDPSocket(void)
{
	struct sockaddr_in addr;
	int on;

	printf("Creating UDP socket... ");
	fflush(stdout);

	if ((udp_sock = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		errPrintf("createUDPSocket(): socket(): %s\n",strerror(errno));
		return 0;
	}

	/* If we can't listen on a broadcast socket then can't proceed */
	on = 1;
	if (setsockopt(udp_sock,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on)) == -1)
	{
		errPrintf("createUDPSocket(): setsockopt(SO_BROADCAST): %s\n",
			strerror(errno));
		return 0;
	}

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(udp_port);
	if (bind(udp_sock,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		errPrintf("createUDPSocket(): bind(): %s\n",strerror(errno));
		return 0;
	}
	addr_list_cnt = 0;
	ok();
	return 1;
}




/*** Listen out for EZPROXY packets ***/
int getStreamerAddress(void)
{
	struct sockaddr_in addr;
	struct timeval tvs;
	struct timeval *tvp;
	socklen_t size;
	fd_set mask;
	char dummy;
	int i;

	puts("Listening for streamer... ");
	size = sizeof(struct sockaddr_in);

	while(1)
	{
		fflush(stdout);

		FD_ZERO(&mask);
		FD_SET(udp_sock,&mask);

		if (listen_timeout)
		{
			tvs.tv_sec = listen_timeout;
			tvs.tv_usec = 0;
			tvp = &tvs;
		}
		else tvp = NULL;

		switch(select(FD_SETSIZE,&mask,0,0,tvp))
		{
		case -1:
			errPrintf("getStreamerAddress(): select(): %s\n",
				strerror(errno));
			return 0;
		case 0:
			colPrintf("~FYTIMEOUT\n");
			networkClear();
			return 0;
		}
		if (recvfrom(
			udp_sock,
			&dummy,1,0,(struct sockaddr *)&addr,&size) == -1)
		{
			errPrintf("getStreamerAddress(): recvfrom(): %s\n",
				strerror(errno));
			networkClear();
			return 0;
		}
		for(i=0;i < addr_list_cnt;++i)
		{
			if (addr_list[i].addr.s_addr == addr.sin_addr.s_addr)
			{
				/* Don't mention it every packet */
				if (flags.verbose || ++addr_list[i].cnt == 10)
				{
					colPrintf("   ~FTRX:~RS %s: ~FYAddress already tried\n",
						inet_ntoa(addr.sin_addr));
					addr_list[i].cnt = 0;
				}
				break;
			}
		}
		if (i == addr_list_cnt)
		{
			/* The count will be reset when we disconnect */
			colPrintf("   ~FTRX:~RS %s: ~FMNew address\n",
				inet_ntoa(addr.sin_addr));

			addr_list[addr_list_cnt].cnt = 0;
			addr_list[addr_list_cnt].addr = addr.sin_addr;

			con_addr.sin_addr = addr.sin_addr;
			addr_list_cnt = (addr_list_cnt + 1) % ADDR_LIST_SIZE;
			return 1;
		}
	}
	return 0;
}




/*** Resolve the the command line address ***/
int resolveAddress(void)
{
	struct hostent *he;

	/* If its not numeric then resolve */
	if ((int)(con_addr.sin_addr.s_addr = inet_addr(ipaddr)) == -1)
	{
		printf("Resolving address... ");
		fflush(stdout);

		/* Not a numeric address, try to resolve DNS */
		if (!(he = gethostbyname(ipaddr)))
		{
			errPrintf("Unknown host.\n");
			return 0;
		}

		/* Just use 1st address */
		memcpy((char *)&con_addr.sin_addr,he->h_addr,he->h_length);
		ok();
	}
	return 1;
}




int connectToStreamer(void)
{
	struct timeval tvs;
	fd_set mask;
	socklen_t len;
	int sock_flags = 0; /* Keeps gcc happy in -O mode */
	int so_error;
	int val;

	printf("Connecting to %s:%d... ",inet_ntoa(con_addr.sin_addr),tcp_port);
	fflush(stdout);

	if ((tcp_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		errPrintf("connectToStreamer(): socket(): %s\n",strerror(errno));
		return 0;
	}

	/* Set keepalive so if streamer is switched off we find out about it */
	val = 1;
	if (setsockopt(tcp_sock,SOL_SOCKET,SO_KEEPALIVE,&val,sizeof(val)) == -1)
		warnPrintf("connectToStreamer(): setsockopt(SO_KEEPALIVE): %s\n",strerror(errno));
	else
	{
		/* Set the time wait in seconds from the last packet until we 
		   send the probe */
		val = 30; 
#ifdef __APPLE__
		if (setsockopt(tcp_sock,IPPROTO_TCP,TCP_KEEPALIVE,&val,sizeof(val)) == -1)
			warnPrintf("connectToStreamer(): setsockopt(TCP_KEEPALIVE): %s\n",strerror(errno));
#else
		if (setsockopt(tcp_sock,IPPROTO_TCP,TCP_KEEPIDLE,&val,sizeof(val)) == -1)
			warnPrintf("connectToStreamer(): setsockopt(TCP_KEEPIDLE): %s\n",strerror(errno));
#endif
	}

	con_addr.sin_port = htons(tcp_port);

	if (connect_timeout)
	{
		/* Set socket to non blocking so we can have our own timeout 
		   on connect */
		sock_flags = fcntl(tcp_sock,F_GETFL);
		fcntl(tcp_sock,F_SETFL,sock_flags | O_NONBLOCK);
	}

	if (connect(tcp_sock,(struct sockaddr *)&con_addr,sizeof(con_addr)) == -1)
	{
		switch(errno)
		{
		case EINPROGRESS:
			break;	
		case ECONNREFUSED:
			colPrintf("~FRREFUSED\n");
			return 0;
		case ETIMEDOUT:
			colPrintf("~FYTIMEOUT\n");
			return 0;
		default:
			errPrintf("connectToStreamer(): connect(): %s\n",strerror(errno));
			return 0;
		}
	}

	if (connect_timeout && errno != EISCONN)
	{
		tvs.tv_sec = connect_timeout;
		tvs.tv_usec = 0;

		FD_ZERO(&mask);
		FD_SET(tcp_sock,&mask);

		/* Wait for connect to complete/fail or select timeout */
		switch(select(FD_SETSIZE,0,&mask,0,&tvs))
		{
		case -1:
			errPrintf("connectToStreamer(): select(): %s\n",
				strerror(errno));
			return 0;
		case 0:
			colPrintf("~FYTIMEOUT\n");
			return 0;
		}

		/* See if connect errored */
		len = sizeof(int);
		getsockopt(tcp_sock,SOL_SOCKET,SO_ERROR,&so_error,&len);

		if (so_error)
		{
			errPrintf("connectToStreamer(): connect(): %s\n",
				strerror(so_error));
			return 0;
		}

		/* Reset socket back to blocking */
		fcntl(tcp_sock,F_SETFL,sock_flags ^ O_NONBLOCK);
	}

	colPrintf("~FGCONNECTED\n");
	connect_time = time(0);
	return 1;
}




/*** Read a packet from the socket. First read the header then get the data
     length and read up to the end of that ***/
void readSocket(int print_prompt)
{
	t_iscp_data *pkt_data;
	char readbuff[READBUFF_SIZE];
	uint32_t data_len;
	int new_data;
	int read_len;
	int len;

	/* Read header first then data */
	if (buffer[BUFF_TCP].len < PKT_HDR_LEN)
		read_len = PKT_HDR_LEN - buffer[BUFF_TCP].len;
	else 
	{
		read_len = (pkt_hdr->hdr_len + pkt_hdr->data_len) - buffer[BUFF_TCP].len;
		if (!read_len) return;
		if (read_len < 0)
		{
			nlErrPrintf("readSocket(): Bad data.\n");
			goto ERROR;
		}
	}
	if (read_len > READBUFF_SIZE) read_len = READBUFF_SIZE;

	READ:
	switch((len = read(tcp_sock,readbuff,read_len)))
	{
	case -1:
		if (errno == EINTR && flags.interrupted) goto READ;
		nlErrPrintf("readSocket(): read(): %s\n",strerror(errno));
		goto ERROR;
	case 0:
		colPrintf("\n~BR~FW*** Connection closed by streamer ***\n");
		goto ERROR;
	}
	++rx_reads;
	rx_bytes += len;
	last_rx_time = time(0);

	addToBuffer(BUFF_TCP,readbuff,len);

	if (buffer[BUFF_TCP].len == PKT_HDR_LEN)
	{
		/* Got a full header, set byte order and return to read more
		   data from socket */
		pkt_hdr->hdr_len = ntohl(pkt_hdr->hdr_len);
		pkt_hdr->data_len = ntohl(pkt_hdr->data_len);
		return;
	}

	/* Do we have the whole packet? */
	if ((uint32_t)buffer[BUFF_TCP].len < pkt_hdr->hdr_len + pkt_hdr->data_len)
		return;

	/* We do now */
	pkt_data = (t_iscp_data *)(buffer[BUFF_TCP].data + pkt_hdr->hdr_len);
	if (strncmp(pkt_hdr->iscp,"ISCP",4))
		warnPrintf("\nUnknown preamble: %.4s\n",pkt_hdr->iscp);
	if (pkt_data->start_char != START_CHAR)
		warnPrintf("\nUnknown start char: %c\n",pkt_data->start_char);

	/* Add RX to list for recall later. Returns 1 if new data */
	new_data = updateRXList(
		pkt_data->command,
		pkt_data->mesg,pkt_hdr->data_len - (MESG_OFFSET + 3));

	data_len = pkt_hdr->data_len - MESG_OFFSET;

	/* NTM is sent every second */
	if (!strncmp(pkt_data->command,"NTM",3))
	{
		if (data_len >= 8)
		{
			snprintf(
				track_time_str,
				sizeof(track_time_str),
				"%.8s",pkt_data->command+3);
		}
		else strcpy(track_time_str,TIME_DEF_STR);

		if (data_len >= 20)
		{
			snprintf(
				track_len_str,
				sizeof(track_time_str),
				"%.8s",pkt_data->command+12);
		}
		else strcpy(track_len_str,TIME_DEF_STR);

		/* Ignore unless told otherwise */
		if (!flags.show_track_time)
		{
			clearBuffer(BUFF_TCP);
			return;
		}
	}

	/* See if we're waiting for jacket art info */
	if (save_state != SAVE_INACTIVE &&
	    !strncmp(pkt_data->command,"NJA",3))
	{
		saveArtDataLine(data_len,pkt_data);
		new_data = 0; /* Otherwise it'll try to pretty print */
	}

	/* Print raw RX. Show repeated data. */
	if (raw_level)
	{
		clearPrompt();
		if (raw_level >= RAW_HIGH1)
		{
			/* Header */
			colPrintf("\n~FGRX %d bytes:\n",buffer[BUFF_TCP].len);
			printPacketDetails(pkt_hdr,pkt_data,1);
		}
		else
		{
			colPrintf("~FGRX:~RS ");
			printMesg(
				buffer[BUFF_TCP].data+PKT_HDR_LEN,
				pkt_hdr->data_len);
		}
		/* Minor issue - any ampersand codes won't get translated even
		   if translate flag set. Oh well. */
		if (data_len > 2 && !memcmp(pkt_data->command,"NTI",3))
			addTitle(pkt_data->mesg,data_len - MESG_TERM_LEN);
		clearBuffer(BUFF_TCP);
		if (print_prompt) printPrompt();

		/* 2 of the levels don't do pretty print */
		if (raw_level == RAW_LOW1 || raw_level == RAW_HIGH1) return;
	}

	/* If new data then print it otherwise dump */
	if (new_data)
		prettyPrint(pkt_data,print_prompt);
	else
		clearBuffer(BUFF_TCP);
	return;

	ERROR:
	networkClear();
	if (print_prompt) printPrompt();
	clearBuffer(BUFF_TCP);
}




/*** Write whats in the keyboard buffer as data to the streamer. The data
     must start wth the 3 letter command, eg NTC ***/
int writeSocket(char *write_data, int write_data_len)
{
	t_iscp_hdr hdr;
	t_iscp_data *data;
	char *pkt;
	char *write_ptr;
	int write_len;
	int wrote_len;
	int data_len;
	int pkt_len;
	int ret;

	if (write_data_len < COM_LEN) return 1;

	/* +3 for start char and device code at the start and \r on the end */
	data_len = write_data_len + COM_LEN;

	/* Set header */
	memcpy(hdr.iscp,"ISCP",4);
	hdr.hdr_len = htonl(PKT_HDR_LEN);
	hdr.data_len = htonl(data_len); 
	hdr.version = 1;
	hdr.reserved[0] = 0;
	hdr.reserved[1] = 0;
	hdr.reserved[2] = 0;

	/* Unfortunately it seems like the player can't handle commands split up
	   over several writes so have to put everything in one lump of memory 
	   then send it */
	pkt_len = PKT_HDR_LEN + data_len;
	if (!(pkt = (char *)malloc(pkt_len)))
	{
		nlErrPrintf("writeSocket(): malloc(): %s\n",strerror(errno));
		return 0;
	}

	/* Add header */
	memcpy(pkt,(char *)&hdr,PKT_HDR_LEN);

	/* Add start char, device code then user input and \r */
	write_data_len -= COM_LEN;
	data = (t_iscp_data *)(pkt+PKT_HDR_LEN);
	data->start_char = START_CHAR;
	data->device_code = device_code;
	memcpy(data->command,write_data,COM_LEN);
	memcpy(data->mesg,write_data+COM_LEN,write_data_len);
	data->mesg[write_data_len] = '\r';

	switch(raw_level)
	{
	case RAW_OFF:
		break;
	case RAW_LOW1:
	case RAW_LOW2:
		colPrintf("~FRTX:~RS ");
		printMesg((char *)data,data_len);
		break;
	case RAW_HIGH1:
	case RAW_HIGH2:
		colPrintf("~FRTX %d bytes:\n",pkt_len);
		/* Reset back to host ordering before printing */
		hdr.hdr_len = PKT_HDR_LEN;
		hdr.data_len = data_len;
		printPacketDetails(&hdr,(t_iscp_data *)data,0);
		break;
	default:
		assert(0);
	}

	ret = 1;

	/* If the write is split the streamer won't like it if it means 
	   seperate frames, but not much we can do */
	for(write_len=pkt_len,write_ptr=pkt;
	    write_len > 0;write_len-=wrote_len,write_ptr+=wrote_len)
	{
		WRITE:
		if ((wrote_len = write(tcp_sock,write_ptr,write_len)) == -1)
		{
			if (errno == EINTR && flags.interrupted)
				goto WRITE;
			nlErrPrintf("writeSocket(): write(): %s\n",strerror(errno));
			networkClear();
			ret = 0;
			break;
		}
		++tx_writes;
		tx_bytes += wrote_len;
		last_tx_time = time(0);
	}
	free(pkt);
	return ret;
}




/*** Lengths must be in host byte order ***/
void printPacketDetails(t_iscp_hdr *hdr, t_iscp_data *data, int rx)
{
	int bytes;
	int i;

	colPrintf("   ~FYHeader:\n");
	printf("      Header length : %d bytes\n",hdr->hdr_len);
	printf("      Data length   : %d bytes\n",hdr->data_len);
	printf("      ISCP Version  : %d\n",hdr->version);

	/* Has EOF-CR-LF on the end hence term len */
	colPrintf("   ~FMData:\n");
	printf("      Start char   : %c\n",data->start_char);
	printf("      Device code  : %c\n",data->device_code);
	printf("      Command      : %.3s\n",data->command);

 	bytes = hdr->data_len - MESG_OFFSET;
	/* 3 byte terminator for RX, only 1 for TX */
	bytes -= (rx ? MESG_TERM_LEN : 1);
	if (bytes < 0)
	{
		colPrintf("   ~FRMessage length error\n");
		return;
	}
	printf("      Message bytes: %d\n",bytes);
	printf("      Message      : ");
	printMesg(data->mesg,bytes);
	printf("      Terminator   : ");
	if (rx)
	{
		for(i=0;i < 3;++i) printf("0x%02X ",data->mesg[bytes + i]);
	}
	else printf("0x%02X",data->mesg[bytes]);
	putchar('\n');
}




void networkClear(void)
{
	if (flags.verbose) 
	{
		printf("Clearing connection... ");
		fflush(stdout);
	}
	FREEIF(ipaddr);

	if (udp_sock)
	{
		close(udp_sock);
		udp_sock = 0;
	}
	if (tcp_sock)
	{
		close(tcp_sock);
		tcp_sock = 0;
		strcpy(track_time_str,TIME_DEF_STR);
		strcpy(track_len_str,TIME_DEF_STR);
		if (flags.verbose) ok();
		if (connect_time)
		{
			colPrintf("~BM~FW*** ~LIDISCONNECTED~RS~BM ***\n");
			connect_time = 0;
		}
	}
	else if (flags.verbose) ok();
}
