/*
 * server.c
 *
 *  Created on: Oct 15, 2010
 *      Author: kaduparag
 */
//-----------------------------------------------------------------------------------
#include	"server.h"
#include <sys/stat.h> 
#include <fcntl.h>
//---------------------------GLOBAL----------------------------------------------------
int interfaceCount = 0;
int socketDescriptors[MAX_INTERFACE];//TODO Make MAX_INTERFACE dynamic by using getInterfaceInfo().
int server_port, max_win_size;
int TIMEOUT_SEC=1;
int TIMEOUT_USEC=0;
struct client_info *clientListHead=NULL;
struct interface_info *ifihead=NULL;
//----------------------------------------------------------------------------------

int mydg_echo(int ,const char *);

//-------------------------------------------------------------------------------
struct interface_info * getInterfaceInfo() {
/*
	struct interface_info *ifi;
	ifi = calloc(1, sizeof(struct interface_info));
	if (ifi == NULL) {
		err_sys_p("calloc error");
	}
	char str[INET_ADDRSTRLEN];
	strcpy(ifi->ifi_name, "localhost");
	strcpy(str, "127.0.0.1");
	ifi->ifi_addr = calloc(1, sizeof(struct sockaddr));
	if (inet_pton(AF_INET, str, ifi->ifi_addr) != 1) {
		err_sys_p("Cannot convert string IP to binary IP.");
	}
	return ifi;
*/
}
//--------------------------------------------------------------------------------
int main(int argc, char **argv) {
	int i;
	const int on = 1;
	pid_t pid;
	int child_pid;
	struct interface_info *ifi,*node;
	struct sockaddr_in *sa;
	//I/O Multiplexing Select options
	int maxfdp1,j;
	fd_set rset;
	int maxSocketDescriptor = -999;
	char serverIP[INET_ADDRSTRLEN];

	printf("<start>\n");

	//	if (argc != 2)  //TODO Uncomment
	//			err_sys_p("usage: server <server.in file>");

	//Read input configuration from server.in
	char line[MAX_LINE];
	FILE* fp = fopen("server.in", "r"); //TODO read from argv[1]
	if (fgets(line, sizeof(line), fp)) {//Line 1 server port
		server_port = atoi(line);
		if (server_port == 0) {
			err_sys_p(
					"Invalid or missing server port number in the configuration file.");
		}
		printf("[INFO] Server port:%d\n", server_port);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	if (fgets(line, sizeof(line), fp)) {//Line 2 maximum sliding window size.
		max_win_size = atoi(line);
		if (max_win_size == 0) {
			err_sys_p(
					"Invalid or missing Max win size in the configuration file.");
		}
		printf("[INFO] Max win size port:%d\n", max_win_size);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	fclose(fp);

	generate_ifi_list(&ifihead);
	ifi=ifihead;	

	for (; ifi != NULL; ifi = ifi->ifi_next) {
		/*bind unicast address */
		socketDescriptors[interfaceCount] = socket(AF_INET, SOCK_DGRAM, 0);
		if (socketDescriptors[interfaceCount] < 0) {
			err_sys_p("socket error.");
		}

		if (setsockopt(socketDescriptors[interfaceCount], SOL_SOCKET,
				SO_REUSEADDR, &on, sizeof(on)))
			err_sys_p("Couldnt set Echo socket option");

		sa =  &ifi->ifi_addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(server_port);
		if (bind(socketDescriptors[interfaceCount], (struct sockaddr *) sa,	sizeof(*sa)))
			err_sys_p("Coudlnt bind socket.");

		printf("[INFO] Bounded to %s\n", Sock_ntop((struct sockaddr *) sa,sizeof(*sa)));

		interfaceCount++;
	}

	printf("[INFO] Total number of interfaces: %d\n", interfaceCount);
	//interfaceCount =1; //ugly hack
	//Using select monitor different sockets bounded to available unicast interfaces.
	for (;;) {
		//printf("[INFO] Parent server waiting for incoming requests...\n"); //TODO Uncomment info
		FD_ZERO(&rset);
		for (i = 0; i < interfaceCount; i++) {
			FD_SET(socketDescriptors[i], &rset);
			maxSocketDescriptor = max(maxSocketDescriptor,socketDescriptors[i]);
		}
		maxfdp1 = maxSocketDescriptor + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);
		for (i = 0; i < interfaceCount; i++) {
			if (FD_ISSET(socketDescriptors[i], &rset)) { // one of the socket descriptor is ready
				//fork a child
				if ((pid = fork()) == 0) { // child 
					j=0;
					node=ifihead;
					while(j<i) node= node->ifi_next;
					child_pid = mydg_echo(socketDescriptors[i],inet_ntoa(node->ifi_addr.sin_addr));
					printf("[INFO] Child server %d closed.\n", child_pid);
					exit(0);
				} else if (pid == -1) {
					err_sys_p("Couldnt fork a child.");
				} else {
					//parent. DO nothing
				} 
//			child_pid = mydg_echo(socketDescriptors[i],"127.0.0.1");
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------
int mydg_echo(int sockfd,const char * myaddr) {

	int n, i, connection_sockfd,success_flag,attempt_count,ret,on=1;
	char *filename, con_sock_port[8];//TODO con_sock_port size? FILE_NAME_LEN?
	char *sendline, *recvline;
	struct udp_datagram *sender_buffer = (struct udp_datagram *)malloc(sizeof(struct udp_datagram));
	struct udp_ack *client_ack= (struct udp_ack*)malloc(sizeof(struct udp_datagram));
	struct in_addr closest;
	socklen_t  addrlen,clilen;
	struct sockaddr_in localaddr,cliaddr;
	int num_bytes_read,fsocket;
	sendline = malloc(MAXLINE);
	recvline = malloc(MAXLINE);
	filename = malloc(MAXLINE);
	//Close all other socket descriptor
	for (i = 0; i < interfaceCount; i++) {
		if (socketDescriptors[i] != sockfd)
			close(socketDescriptors[i]);
	}

	//Read filename from client
	 clilen=sizeof(cliaddr);
	n = recvfrom(sockfd, filename, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen);
	if (n < 0) {
		err_sys_p("Data receive error.");
	}
	filename[n] = 0; //null terminate

	 //Get the ipaddr, port number from the client
	       struct sockaddr_in *sin = (struct sockaddr_in *) &cliaddr;
	
	ret= closest_match_to_interface(ifihead, inet_ntoa(sin->sin_addr),&closest);
	if(ret==1)
	{
		//localhost
	}	
	else if(ret==2)
	{
		//Dont route
	 if(setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, (void *) &on, sizeof(on)) < 0)
	    err_sys_p("Can't set SO_DONTROUTE on main socket"); 

	}
	else
	{
		//Dont care
	}
       //Check for already active client
	   if(isNewClient(clientListHead,sin->sin_addr.s_addr,ntohs(sin->sin_port))){
	       printf("[DEBUG] New client...inserting\n");
           //insert into the client list
	       insertClient(&clientListHead,sin->sin_addr.s_addr,ntohs(sin->sin_port));
	       printf("Client inserted:%d %d\n",clientListHead->ipaddr,clientListHead->port);
	   }else{ // End this child process.Client already handled by other child process.
           	printf("[DEBUG] Client already present...\n");
          	exit(0);
	   }
	printf("[INFO] Child server %d datagram from %s", getpid(), Sock_ntop(
			(struct sockaddr *) &cliaddr, clilen));
	printf(", to %s\n", myaddr);
	printf("[INFO] File requested %s\n", filename);

	//Start a new connection socket at ephemeral port.
	connection_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(connection_sockfd < 0){
		err_sys_p("Connection socket error.Cannot open the socket.");
	}
	bzero(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(0);
	if (inet_pton(AF_INET, myaddr, &localaddr.sin_addr) != 1) //Using the same IP as the parent listening socket
		err_sys_p("Cannot convert string IP to binary IP.");

	if (bind(connection_sockfd, (struct sockaddr *) &localaddr,
			sizeof(localaddr)))
		err_sys_p("Coudlnt bind socket.");
//
//	printf("[INFO] Bounded to %s\n", Sock_ntop((struct sockaddr *) &localaddr,
//					sizeof(localaddr)));
	addrlen=sizeof(localaddr);
	//Get the ephemeral port  number.
	getsockname(connection_sockfd, (struct sockaddr *) &localaddr, &addrlen);
	sprintf(con_sock_port, "%d",  ntohs(localaddr.sin_port));

	//Print connection socket information
	printf("[INFO] Connection socket IP %s", myaddr);
	printf(", ephemeral port %s\n", con_sock_port);

	 // getchar();
	//Send the new connection socket ephemeral port.
	if ((n = sendto(sockfd, con_sock_port, strlen(con_sock_port), 0, (struct sockaddr *) &cliaddr,
			clilen)) != strlen(con_sock_port)) { //using the parent listening socket
		err_sys_p("Data send error.");
	}

	success_flag=0;
	for(attempt_count=0;attempt_count<MAX_ATTEMPT;attempt_count++){
       if(readable_timeout(connection_sockfd,TIMEOUT_SEC,TIMEOUT_USEC)==0){
          printf("Socket timeout...attempt %d failed.\n",attempt_count);
          //Send the new connection socket ephemeral port.
          	if ((n = sendto(sockfd, con_sock_port, strlen(con_sock_port), 0, (struct sockaddr *) &cliaddr,
          			clilen)) != strlen(con_sock_port)) { //using the parent listening socket
          		err_sys_p("Data send error.");
          	}
       }else{
    	   success_flag=1;
         break;
       }
	}
	 if(success_flag==0){//TODO what to do if MAX_ATTEMT fail? report and end prgm?
	        err_sys_p("Socket timeout...max no. of attempt failed.\n");
	 }

  //Wait for connected ack
	n = recvfrom(connection_sockfd, recvline, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen);
	if (n < 0) {
		err_sys_p("Data receive error.");
	}
	recvline[n] = 0; //null terminate
    printf("%s\n",recvline);
    fflush(NULL);
	//Send the file on connection socket and close the parent listening socket
	close(sockfd);
/**	open a file and start reading from the file send bytes**/


	fsocket=open(filename,O_RDONLY);
	if(fsocket<0)
		err_sys_p("Unable to open file");
	
	do {
		bzero(sender_buffer,sizeof(struct udp_datagram));
		bzero(client_ack,sizeof(struct udp_ack));
		 num_bytes_read = read(fsocket, sender_buffer->data, sizeof(sender_buffer->data));

		if(num_bytes_read< 0)
			err_sys_p("file read error.");
		else
		{
			printf("sending % d bytes\n",num_bytes_read+4);

			if (sendto(connection_sockfd, sender_buffer, num_bytes_read+4, 0, (struct sockaddr *) &cliaddr, clilen) != (num_bytes_read+4)) { //using the parent 	listening socket
				err_sys_p("Data send error.");
			}
		}
		/*wait for ack*/
		n = recvfrom(connection_sockfd, client_ack, sizeof(struct udp_ack), 0, (struct sockaddr *) &cliaddr, &clilen);
		if (n < 0) {
			err_sys_p("Ack recv error.");
		}	
		else {
			printf("Seq:%d cwnd:%d\n",client_ack->seq_ack_num,client_ack->cwnd);
		}
		
	
	}while(num_bytes_read==508);
	 
	
	free(recvline);
	free(sendline);
	free(filename);
//Client handling done. Remove from the client list
       printClientList(clientListHead);
       deleteClient(&clientListHead,sin->sin_addr.s_addr,ntohs(sin->sin_port));
       printClientList(clientListHead);
	return getpid();
}
/* end mydg_echo */
