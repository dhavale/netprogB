#ifndef NP_UDP_H
#define NP_UDP_H

#include <stdio.h>

struct udp_datagram{
	int seq_num;
	char data[508];
};

struct udp_ack{
	int seq_ack_num;
	int adv_wnd;
};


#endif
