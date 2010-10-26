#ifndef NETPROG_B_H
#define NETPROG_B_H
#include "unpifiplus.h"
#include "unp.h"

struct interface_info {
char ifi_name[IFI_NAME];	/* interface name, null-terminated */
struct sockaddr_in ifi_addr;	/* primary address */
struct sockaddr_in ifi_net_mask; /* destination address */
struct sockaddr_in ifi_subnet_addr; /* obtained by anding of ip and subnet*/
int	sockfd;
struct interface_info *ifi_next;	/* next of these structures */
};

#endif

