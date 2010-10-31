/*
 * server.h
 *
 *  Created on: Oct 15, 2010
 *      Author: kaduparag
 */

#ifndef SERVER_H_
#define SERVER_H_
//-----------------------Libraries-------------------------------------------//
#include  <unistd.h>
#include  <sys/types.h>	/* basic system data types */
#include  <sys/socket.h>	/* basic socket definitions */
#include  <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include  <errno.h>
#include  <arpa/inet.h>
#include  <sys/wait.h>
#include  "np_udp.h"
//---------------------------------------------------------------------------//

#define	MAXLINE		4096	/* max text line length */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */
#define	UDP_SERV_PORT 9877
#define	IFI_NAME	16			/* same as IFNAMSIZ in <net/if.h> */
#define	IFI_HADDR	 8			/* allow for 64-bit EUI-64 in future */
#define MAX_INTERFACE 10
#define MAX_LINE 80
#define FILE_NAME_LEN 40
#define MAX_ATTEMPT 12

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))


struct client_info{
       unsigned int ipaddr;
       unsigned short int port;
       struct client_info *next;
};



//int generate_ifi_list(struct interface_info **head);

#endif /* SERVER_H_ */
