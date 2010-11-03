/*
 * client.c
 *
 *  Created on: Oct 15, 2010
 *      Author: kaduparag
 */

//#include	"server.h"
#include "common_lib.h"
#include "client.h"
#include "np_queue.h"
#include <pthread.h>
#include <math.h>
//Global
char serverIP[INET_ADDRSTRLEN];
float drop_probability;
int server_port, max_win_size;
int TIMEOUT_SEC=1;
int TIMEOUT_USEC=0;
char required_file_name[FILE_NAME_LEN];
struct sockaddr_in cliaddr;
int seed_val;
int end_of_file=999999;
int mean_mue;
pthread_mutex_t protect_queue = PTHREAD_MUTEX_INITIALIZER;
int quit=0;

void* consumer_thrd_func(void*data)
{
	struct np_queue * q = (struct np_queue*)data;
	int last_read=0;
	double sleep_time=0;
	float random;
	while(1)
	{
	pthread_mutex_lock(&protect_queue);
		
	if(q->front!=0)
				{
					
					while((last_read<q->front)&&(!quit))
					{
						/*Read from last_read*/
						printf("\n\t-=-=-=-=-=-=-=-=-=-=-[CONS]:reading %d-=-=-=-=-=-=-=--=-=\t\n",
											last_read+1);
						if(write(fileno(stdout),queueItem(q,last_read)->data,
								sizeof(q->send_buff->data))<0)
						err_sys_p("stdout error!!");
						printf("\n\t-=-=-=-=-=-=-=-=-=-=-[CONS]:End of read-=-=-=-=-=-=-=--=-=\t\n");
						clearFlag(q,last_read);
						q->rear=moveRear(q);
						last_read++;
					}
				}
	if(end_of_file<q->front||quit)
			break;
	pthread_mutex_unlock(&protect_queue);
	random = (float)rand()/RAND_MAX;
	if(random==0)
		random=0.3;
	sleep_time = -1 *mean_mue* log(random);
	//printf("\ngonna sleep for %lf in ms %d for %lf log is %lf",sleep_time,(int)ceil(sleep_time/1000),random,log(random));
	sleep((int)ceil(sleep_time/1000));
	}	
		pthread_mutex_unlock(&protect_queue);
	return NULL;
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	int len,on=1;
	struct interface_info *head;
	struct sockaddr_in closest;
	generate_ifi_list(&head);
	
	print_my_list(head);

	int ret;

	//	if (argc != 2) //TODO Uncomment
	//		err_sys("usage: client <client.in file>");

	//Read input configuration from server.in
	char line[MAX_LINE];
	FILE* fp = fopen("client.in", "r"); //TODO read from argv[1]
	if (fgets(line, sizeof(line), fp)) {//Line 1 server ip
		len = strlen(line) - 1; //remove the trailing \n
		if (line[len] == '\n')
			line[len] = '\0';
		strcpy(serverIP, line);
		if (!isIPAddress(serverIP)) {
			err_sys_p("Invalid or missing server ip in the configuration file.");
		}
//		printf("[INFO] Server ip:%s\n", serverIP);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	if (fgets(line, sizeof(line), fp)) {//Line 2 server port
		server_port = atoi(line);
		if (server_port == 0) {
			err_sys(
					"Invalid or missing server port number in the configuration file.");
		}
//		printf("[INFO] Server port:%d\n", server_port);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	if (fgets(line, sizeof(line), fp)) {//Line 3 file name
		len = strlen(line) - 1; //remove the trailing \n
		if (line[len] == '\n')
			line[len] = '\0';
		strcpy(required_file_name, line);
		if (len == 0)
			err_sys("Invalid or missing file name in the configuration file.");
//		printf("[INFO] File name:%s\n", required_file_name);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	if (fgets(line, sizeof(line), fp)) {//Line 4 maximum sliding window size.
		max_win_size = atoi(line);
		if (max_win_size == 0) {
			err_sys_p(
					"Invalid or missing Max win size in the configuration file.");
		}
//		printf("[INFO] Max win size:%d\n", max_win_size);
	} else {
		err_sys_p("Invalid or missing input configuration.");
	}

	if (fgets(line, sizeof(line), fp)) {//Line 5 random seed value.
               seed_val = atoi(line);
               if (seed_val < 0) {
                       err_sys_p("Invalid or missing seed value in the configuration file.");
               }
  //             printf("[INFO] Seed Value:%d\n", seed_val);

       } else {
               err_sys("Invalid or missing input configuration.");
       }

       if (fgets(line, sizeof(line), fp)) {//Line 6 drop probability.
               sscanf(line, "%f", &drop_probability);
               if (drop_probability > 1.0) {
                       err_sys(
                                       "Invalid or missing drop probability in the configuration file.");
               }
    //           printf("[INFO] Drop probability:%f\n", drop_probability);
       } else {
               err_sys("Invalid or missing input configuration.");
       }
	if (fgets(line, sizeof(line), fp)) {//Line 7 mean "mue"
               sscanf(line, "%d", &mean_mue);
               if (mean_mue  < 0) {
                       err_sys_p(
                                       "Invalid or missing mean mue in the configuration file.");
               }
      //         printf("[INFO] Mean Mue:%d\n", mean_mue);
       } else {
               err_sys("Invalid or missing input configuration.");
       }
       fclose(fp);

       //Set random seed value
       srand(seed_val);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server_port);
	
	bzero(&closest, sizeof(closest));
	ret =closest_match_to_interface(head,serverIP,&closest.sin_addr); 

//	printf("[INFO] closest is %s\n",inet_ntoa(closest.sin_addr));
	cliaddr.sin_addr.s_addr = closest.sin_addr.s_addr;	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		err_sys_p("Socket error.");
	}

	if(ret==1)
	{
		/**returned 1, we should use localhost for both server and client**/
		if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) != 1)
		err_sys_p("Cannot convert string IP to binary IP.");

	}

	else if(ret==2)
	{
		/**then its on local n/w , use dont' route**/
		printf("Same subnet so using SO_DONTROUTE..\n");
		if(setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, (void *) &on, sizeof(on)) < 0)
	    err_sys_p("Can't set SO_DONTROUTE on main socket"); 

		if (inet_pton(AF_INET, serverIP, &servaddr.sin_addr) != 1)
		err_sys_p("Cannot convert string IP to binary IP.");

	}

	else
	{
		if (inet_pton(AF_INET, serverIP, &servaddr.sin_addr) != 1)
		err_sys_p("Cannot convert string IP to binary IP.");

	}
	
	    

	dg_client(sockfd, &servaddr, sizeof(servaddr));

	exit(0);
}

void dg_client(int sockfd, const struct sockaddr_in *pservaddr, socklen_t servlen) {
	int n,connection_sockfd,attempt_count,success_flag;
	socklen_t addrlen;
	char sendline[MAXLINE], recvline[MAXLINE + 1];
	struct sockaddr_in servaddr;
	struct udp_datagram *recv_buffer = (struct udp_datagram *)malloc(sizeof(struct udp_datagram));
	struct udp_datagram *recv_item;
	struct udp_ack *client_ack = (struct udp_ack*)malloc(sizeof(struct udp_ack));
	pthread_t consumer_thread;
	char handshake[52]={};
	cliaddr.sin_addr.s_addr = htonl(cliaddr.sin_addr.s_addr);
	struct np_queue *q = createQueue(max_win_size);
	q->front=0; q->rear=max_win_size -1;

	pthread_create(&consumer_thread,NULL,consumer_thrd_func,q);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(0);
	if (bind(sockfd, (struct sockaddr *) &cliaddr,sizeof(cliaddr)))
		err_sys_p("Coudlnt bind socket.");

   // printf("[INFO] Bounded to %s\n", (char*)Sock_ntop((struct sockaddr * ) &cliaddr,sizeof(cliaddr)));
	addrlen=sizeof(cliaddr);
	//Get the ephemeral port  number.
	getsockname(sockfd, (struct sockaddr *) &cliaddr, &addrlen);
//	printf( "%d",  ntohs(localaddr.sin_port));

	//Print connection socket information
	//printf("[INFO] Connection socket IP %s", myaddr);
	
	printf("\nClient bound to: %s ",inet_ntoa(cliaddr.sin_addr));
	printf(", ephemeral port %d\n", ntohs(cliaddr.sin_port));



	//Connect to well known port of the server
	if (connect(sockfd, (struct sockaddr *) pservaddr, servlen) < 0) {
		err_sys_p("Connect error");
	}
	addrlen=sizeof(servaddr);
	//Get the ephemeral port  number.
	getpeername(sockfd, (struct sockaddr *) &servaddr, &addrlen);
//	printf( "%d",  ntohs(localaddr.sin_port));

	//Print connection socket information
	//printf("[INFO] Connection socket IP %s", myaddr);
	
	printf("\nConnected to Server: %s ",inet_ntoa(servaddr.sin_addr));
	printf(", well-known port %d\n", ntohs(servaddr.sin_port));



	//Send the filename and max_window_size as datagram
	sprintf(handshake,"%s %d",required_file_name,max_win_size);
	int len = strlen(handshake);
	if (mywritel(sockfd, handshake, len,drop_probability) != len) {
		err_sys_p("Write error.Server is unreachable");
	}

	success_flag=0;
	for(attempt_count=0;attempt_count<MAX_ATTEMPT;attempt_count++){
       if(readable_timeout(sockfd,TIMEOUT_SEC*1000)==0){
          printf("Socket timeout...attempt %d failed.\n",attempt_count);
          if (mywritel(sockfd, handshake, len,drop_probability) != len) { //Try again
          		err_sys_p("Write error.Server is unreachable");
          	}
       }else{
    	   success_flag=1;
         break;
       }
	}
	 if(success_flag==0){//TODO what to do if MAX_ATTEMT fail? report and end prgm?
	        err_sys_p("Socket timeout...max no. of attempt failed.\n");
	 }

	//Read the new connection socket ephemeral port number. This serves as ack for filename too.
	if ((n = read(sockfd, recvline, MAXLINE)) == -1)
		err_sys_p("Read error. Server is unreachable");

	recvline[n] = 0; // null terminate
	printf("Connected to servers ephemeral port %s\n",recvline);
	printf("\n");
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(recvline));
	servaddr.sin_addr = pservaddr->sin_addr;

/*	if (inet_pton(AF_INET, serverIP, &servaddr.sin_addr) != 1)
		err_sys_p("Cannot convert string IP to binary IP.");
*/
	connection_sockfd = sockfd;
	if (connect(connection_sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {

		err_sys_p("Connect error");
	}

	//Send connected ack
	strcpy(sendline, "ACK_CONNECTED");
	if (mywritel(connection_sockfd, sendline, strlen(sendline),drop_probability) != strlen(sendline)) {
		err_sys_p("Write error.Server is unreachable");
	}
//	printf("%s\n",sendline);
	success_flag=0;
	for(attempt_count=0;attempt_count<MAX_ATTEMPT;attempt_count++){
       	if(readable_timeout(connection_sockfd,TIMEOUT_SEC*1000)==0){
          printf("Socket timeout...attempt %d failed.\n",attempt_count);
          if (mywritel(connection_sockfd, sendline, strlen(sendline),drop_probability) != strlen(sendline)) { //Try again
          		err_sys_p("Write error.Server is unreachable");
          	}
       	}else{
    	   success_flag=1;
         break;
       }
	}
	 if(success_flag==0){//TODO what to do if MAX_ATTEMT fail? report and end prgm?
	        err_sys_p("Socket timeout...max no. of attempt failed.\n");
	 }

	//	fflush(NULL);
	//Read the file and print output.
//	printf("file send using probability: %f",drop_probability);
	while (1) {
		
		bzero(recv_buffer,sizeof(struct udp_datagram));

       	if(readable_timeout(connection_sockfd,5000)==0){

		printf("Waited long enough, client exiting safely..\n");
		quit=1;
	pthread_join(consumer_thread,NULL);
		exit(0);
	}
		if ((n = myreadl(connection_sockfd, recv_buffer, sizeof(struct udp_datagram),drop_probability)) == -1)
			err_sys_p("Read error. Server is unreachable");
		else if(n==-2)/* packet should be dropped*/
			continue;
		else {
			
			printf("%d bytes recieved seq=%d \n",n,recv_buffer->seq_num);
			recv_buffer->seq_num--; /* decrement seq_num as queue is 0 indexed*/

			if(recv_buffer->seq_num<q->front)
			{
				printf("Delayed packet, have data, skip this but send ack..\n");
				client_ack->seq_ack_num= q->front+1;
				pthread_mutex_lock(&protect_queue);
					client_ack->adv_wnd = q->rear - q->front +1; 
				pthread_mutex_unlock(&protect_queue);			
				if (mywritel(connection_sockfd, client_ack, sizeof(struct udp_ack),drop_probability) != sizeof(struct udp_ack)) {
					err_sys_p("Write error.Server is unreachable");
				}

			}
			else {
//			n=write(fileno(stdout),recv_buffer->data,n-4);
				recv_item = queueItem(q,recv_buffer->seq_num);
				bzero(recv_item,sizeof(struct udp_datagram));
				memcpy(recv_item,recv_buffer,sizeof(struct udp_datagram));
				setFlag(q,recv_item->seq_num);
				q->front= moveFront(q);
				client_ack->seq_ack_num= q->front+1;
				pthread_mutex_lock(&protect_queue);
					client_ack->adv_wnd = q->rear - q->front +1; 
				pthread_mutex_unlock(&protect_queue);
				if (mywritel(connection_sockfd, client_ack, sizeof(struct udp_ack),drop_probability) != sizeof(struct udp_ack)) {
					err_sys_p("Write error.Server is unreachable");
				}
				/*consumer code*/
				
			}
		}
		
		if(n<512)
		{
			pthread_mutex_lock(&protect_queue);
			end_of_file=recv_buffer->seq_num;  /*last packet has been processed*/
			pthread_mutex_unlock(&protect_queue);
		}
		if(end_of_file< q->front)
			break;
	}
/*TIME_WAIT State handling*/
       if(readable_timeout(connection_sockfd,5000)!=0){
          printf("TIME wait activated sending ack %d.\n",end_of_file+2);
		client_ack->seq_ack_num= end_of_file+2;
		if (mywritel(connection_sockfd, client_ack, sizeof(struct udp_ack),drop_probability) != sizeof(struct udp_ack)) {
					err_sys_p("Write error.Server is unreachable");
				}
       }
	
		//while()
	pthread_join(consumer_thread,NULL);

	printf("[INFO] File transfer completed.\n");
}
