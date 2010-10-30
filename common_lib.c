/*
 * server_lib.c
 *
 *  Created on: Oct 15, 2010
 *      Author: kaduparag
 */
//#include "server.h"
#include "client.h"
#include "unpifiplus.h"
int readable_timeout(int fd, int sec,int usec) {
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	return (select(fd + 1, &rset, NULL, NULL, &tv));
	/* 4> 0 if descriptor is readable */
}

//--------------------------------------------------------------------------------
void err_sys_p(const char * msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}
//------------------------------------------------------------------------------
char *
sock_ntop(const struct sockaddr *sa, socklen_t salen) {
	char portstr[8];
	static char str[128]; /* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return (NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return (str);
	}
		/* end sock_ntop */

	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				sa->sa_family, salen);
		return (str);
	}
	return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen) {
	char *ptr;

	if ((ptr = sock_ntop(sa, salen)) == NULL)
		err_sys("sock_ntop error"); /* inet_ntop() sets errno */
	return (ptr);
}



int isIPAddress(const char *addr) {
   int status=1,i;
   int len=strlen(addr);
   for(i=0;i<len;i++){
      if(addr[i]=='.' || (addr[i] >= '0' && addr[i]  <= '9')){
        continue;
      }else{
    	  status=0;
    	  break;
      }

   }
   return status;
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
		//closest->s_addr = match->ifi_addr.sin_addr.s_addr;
		memcpy(closest,&match->ifi_addr,sizeof(struct sockaddr_in));
	}
	else if(localhost)
	{
		printf("IP address match as well!! Use localhost!\n");
//		inet_aton("127.0.0.1",closest);
		closest->s_addr= 0x7f000001;
		return 1; //use localhost for server only.
	}
	else
	{
		printf("\nnothing matched, use interface whichever u want\n");
		match=head;
		while((match!=NULL)||(match->ifi_addr.sin_addr.s_addr!=0x7f000001)) /*skip localhost*/
			match= match->ifi_next;
		closest->s_addr = match->ifi_addr.sin_addr.s_addr;
	}
	
	return 0;
}



