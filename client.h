/*
 * client.h
 *
 *  Created on: Oct 15, 2010
 *      Author: kaduparag
 */

#ifndef CLIENT_H_
#define CLIENT_H_
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
#define FILE_NAME_LEN 40
#define MAX_LINE 80
#define MAX_ATTEMPT 3

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#if 0
struct interface_info {
char ifi_name[IFI_NAME];	/* interface name, null-terminated */
struct sockaddr_in ifi_addr;	/* primary address */
struct sockaddr_in ifi_net_mask; /* destination address */
struct sockaddr_in ifi_subnet_addr; /* obtained by anding of ip and subnet*/
int	sockfd;
struct interface_info *ifi_next;	/* next of these structures */
};
void err_sys_p(const char *);
#endif

#endif /* CLIENT_H_ */
