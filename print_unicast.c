#include <stdio.h>
#include "netprogb.h"
#include "unpifiplus.h"

struct sockaddr_in get_subnet_addr(struct sockaddr_in ip_addr, struct sockaddr_in netmask);

int closest_match_to_interface(struct interface_info *head, char *str_ip,struct in_addr *closest);


int
main(int argc, char **argv)
{
	struct ifi_info	*ifi, *ifihead;
	struct sockaddr	*sa;
	u_char		*ptr;
	int		i, family, doaliases;
	char *str_ip="192.168.148.130";
		family = AF_INET;
#ifdef	IPv6
		//family = AF_INET6;
#endif
	doaliases = 2;

	for (ifihead = ifi = Get_ifi_info_plus(family, doaliases);
		 ifi != NULL; ifi = ifi->ifi_next) {
		printf("%s: ", ifi->ifi_name);
		if (ifi->ifi_index != 0)
			printf("(%d) ", ifi->ifi_index);
		printf("<");
/* *INDENT-OFF* */
		if (ifi->ifi_flags & IFF_UP)			printf("UP ");
		if (ifi->ifi_flags & IFF_BROADCAST)		printf("BCAST ");
		if (ifi->ifi_flags & IFF_MULTICAST)		printf("MCAST ");
		if (ifi->ifi_flags & IFF_LOOPBACK)		printf("LOOP ");
		if (ifi->ifi_flags & IFF_POINTOPOINT)	printf("P2P ");
		printf(">\n");
/* *INDENT-ON* */

		if ( (i = ifi->ifi_hlen) > 0) {
			ptr = ifi->ifi_haddr;
			do {
				printf("%s%x", (i == ifi->ifi_hlen) ? "  " : ":", *ptr++);
			} while (--i > 0);
			printf("\n");
		}
		if (ifi->ifi_mtu != 0)
			printf("  MTU: %d\n", ifi->ifi_mtu);

		if ( (sa = ifi->ifi_addr) != NULL)
			printf("  IP addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));

/*=================== cse 533 Assignment 2 modifications ======================*/

		if ( (sa = ifi->ifi_ntmaddr) != NULL)
			printf("  network mask: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));

/*=============================================================================*/

		if ( (sa = ifi->ifi_brdaddr) != NULL)
			printf("  broadcast addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
		if ( (sa = ifi->ifi_dstaddr) != NULL)
			printf("  destination addr: %s\n",
						Sock_ntop_host(sa, sizeof(*sa)));
	}
	free_ifi_info_plus(ifihead);
	
	printf("\nDhavale's code:\n");

	struct interface_info *head;


	generate_ifi_list(&head);
	print_my_list(head);

	struct in_addr closest;
	if(argv[1]==NULL)
		closest_match_to_interface(head, "127.0.0.1",&closest);
	else
		closest_match_to_interface(head, argv[1],&closest);
	exit(0);
}


int generate_ifi_list(struct interface_info **head)
{
	struct ifi_info *ifi_head, *ifi;
	int family= AF_INET, doaliases = 10;
	struct interface_info *node = (struct interface_info *) malloc(sizeof(struct interface_info));
	struct interface_info *next = NULL;
	*head = node;
	for(ifi_head=ifi=Get_ifi_info_plus(family,doaliases);
		ifi!=NULL;
		ifi=ifi->ifi_next)
	{
		sscanf(ifi->ifi_name,"%s",node->ifi_name);
		node->ifi_addr = *(struct sockaddr_in*)(ifi->ifi_addr);
		node->ifi_net_mask = *(struct sockaddr_in*)(ifi->ifi_ntmaddr);
	//	node->ifi_subnet_addr = get_subnet_addr(node->ifi_addr ,node->ifi_net_mask);
		
		node->ifi_subnet_addr.sin_addr.s_addr = node->ifi_addr.sin_addr.s_addr & node->ifi_net_mask.sin_addr.s_addr;

		if(ifi->ifi_next!=NULL)
		next= (struct interface_info *)malloc (sizeof(struct interface_info));
		else next=NULL;
		node->ifi_next = next;
		node = next;
	}
	node=NULL;
	free_ifi_info_plus(ifi_head);
	return 0;
}

int print_my_list(struct interface_info *head)
{
	struct interface_info *node;
	node = head;
	while(node!=NULL)
	{
		printf("\nname: %s\n",node->ifi_name);
		printf("ip: %s\n",inet_ntoa(node->ifi_addr.sin_addr));

		printf("netmask: %s\n",inet_ntoa(node->ifi_net_mask.sin_addr));
		printf("subnetaddr: %s inbyte 0x%x\n",inet_ntoa(node->ifi_subnet_addr.sin_addr),
							ntohl(node->ifi_subnet_addr.sin_addr.s_addr));
		printf("\n");
		node = node->ifi_next;
	}

	return 0;
}


int closest_match_to_interface(struct interface_info *head, char *str_ip,struct in_addr *closest)
{
	struct in_addr server_ip;
	int localhost=0;
	struct in_addr server_subnet;
	struct interface_info *match= NULL;
	inet_aton(str_ip,&server_ip);
	
	printf("passed ip %s converted to 0x%x\n",str_ip,server_ip.s_addr);

	struct interface_info *node = head;

	while(node!=NULL)
	{
		memset(&server_subnet,0,sizeof(server_subnet));
		server_subnet.s_addr = (server_ip.s_addr)&(node->ifi_net_mask.sin_addr.s_addr);
		if((server_subnet.s_addr)==(node->ifi_subnet_addr.sin_addr.s_addr))
		{
			if(match==NULL)
				match=node;
			else if(match->ifi_net_mask.sin_addr.s_addr<=
					node->ifi_net_mask.sin_addr.s_addr)
			{
				match = node;
			}
				
			printf("subnet match!! recommend interface:%s\n",inet_ntoa(node->ifi_addr.sin_addr));
			if(server_ip.s_addr==node->ifi_addr.sin_addr.s_addr)
			{
				localhost=1;
				break;
			}
		}
		node= node->ifi_next;
	}
	
	if((match!=NULL)&&!localhost)
	{
		printf("recommends: %s\n", inet_ntoa(match->ifi_addr.sin_addr));
	}
	else if(localhost)
	{
		printf("IP address match as well!! Use localhost!\n");
	}
	else
	{
		printf("\nnothing matched, use interface whichever u want\n");
	}
	
	return 0;
}
struct sockaddr_in get_subnet_addr(struct sockaddr_in ip_addr, struct sockaddr_in netmask)
{
//	int i;
//	unsigned char ch;
	printf("size:%d  and %x\n",sizeof(struct sockaddr),ip_addr.sin_addr.s_addr);
/*
	for( i=0; i<14;i++)
	{
		printf("%d:%x\n",i,(ch=ip_addr.sa_data[i]));
	}*/
	return ip_addr;
}



