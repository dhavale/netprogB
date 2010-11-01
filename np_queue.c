#include <stdio.h>
#include "np_queue.h"

struct np_queue* createQueue(int max_size)
{
	struct np_queue* que = (struct np_queue* )calloc(1,sizeof(struct np_queue));
	que->send_buff = (struct udp_datagram *) calloc(max_size,sizeof(struct udp_datagram));
	que->data_flag = (int *)calloc(max_size,sizeof(int));
	printf("Created queue with size %d\n",max_size);
	que->front =0;
	que->rear =0;
	que->size =0;
	que->max_size= max_size;	
	return que;
}

int isQueueEmpty(struct np_queue* que)
{
	if(que->size==0)
	return 1;
	else return 0;

}
int isQueueFull(struct np_queue* que)
{
	if(que->size==que->max_size)
	return 1;
	else return 0;
	
}
//int insertQueue(struct np_queue*, struct udp_datagram *item);
//int deleteQueue(struct np_queue*, int upto);

int queueRear(struct np_queue* que)
{
	return ((que->rear)%(que->max_size));
}

int queueFront(struct np_queue* que)
{
	return ((que->front)%(que->max_size));
}

int setFront(struct np_queue *que,int set)
{
	if(set > que->front)
	{
		que->size -= (set -que->front);
		que->front = set;
		printf("Front is now %d\n",que->front);
	}
	return que->front;
}

struct udp_datagram* queueItem(struct np_queue* que,int index)
{
//	printf("Requested %d item from queue\n",index);
	return &que->send_buff[index%que->max_size];
}

int isDataRecv(struct np_queue* que,int index)
{
	return que->data_flag[index%que->max_size];	
}

int setFlag(struct np_queue* que,int index)
{
	que->data_flag[index%que->max_size]=1;	
}

int clearFlag(struct np_queue* que,int index)
{
	if(index>=0)
	que->data_flag[index%que->max_size]=0;
}


//que->front = movefront
int moveFront(struct np_queue* que)
{
	int ret;
//	setFlag(que,index);

	ret= que->front;
	while(ret<=que->rear)
	{
		if(!isDataRecv(que,ret))
			break;
		ret++;
	}

	return ret;
}


//que->rear = moverear
int moveRear(struct np_queue *que)
{
	int ret;
	
	ret= que->rear;
	while(!isDataRecv(que,ret) && ((ret-que->front)<que->max_size))
	{
		ret++;
	}
	return ret-1;
}

