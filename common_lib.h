#ifndef MY_LIB_H
#define MY_LIB_H

#include "server.h"
#include "client.h"
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


 int mywritel(int sockfd,void * sendline, int len,float drop_prob);
int myreadl(int sockfd, void * recvline, int maxline,float drop_prob);
int closest_match_to_interface(struct interface_info *head, char *str_ip,struct in_addr *closest);
int print_my_list(struct interface_info *head);
int generate_ifi_list(struct interface_info **head);
int isIPAddress(const char *addr);
char * Sock_ntop(const struct sockaddr *sa, socklen_t salen);
char * sock_ntop(const struct sockaddr *sa, socklen_t salen);
void err_sys_p(const char * msg);
int readable_timeout(int fd, int sec,int usec);
void insertClient(struct client_info **clientListHead, unsigned int client_ip,
               unsigned short int client_port);
void deleteClient(struct client_info **clientListHead, unsigned int client_ip,
               unsigned short int client_port);
void printClientList(struct client_info *clientListHead);
int isNewClient(struct client_info *clientListHead, unsigned int client_ip,
               unsigned short int client_port);


#endif

