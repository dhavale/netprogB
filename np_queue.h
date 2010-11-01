#ifndef NP_Q_H
#define NP_Q_H

#include "common_lib.h"

struct np_queue{
	struct udp_datagram * send_buff;
	int *data_flag;
	int front;
	int rear;
	int size;
	int max_size;
};

struct np_queue* createQueue(int max_size);

int isQueueEmpty(struct np_queue* );
int isQueueFull(struct np_queue* );

//int insertQueue(struct np_queue*, struct udp_datagram *item);
//int deleteQueue(struct np_queue*, int upto);


struct udp_datagram* queueItem(struct np_queue* que,int index);
int setFront(struct np_queue *que,int set);
int queueFront(struct np_queue* que);
int queueRear(struct np_queue* que);
struct udp_datagram* queueItem(struct np_queue* que,int index);
int isDataRecv(struct np_queue* que,int index);
int setFlag(struct np_queue* que,int index);
int clearFlag(struct np_queue* que,int index);
int moveFront(struct np_queue* que);
int moveRear(struct np_queue *que);



#endif

