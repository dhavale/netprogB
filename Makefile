
CC = gcc

LIBS = /home/sandeep/unpv13e/libunp.a -lm -lpthread

FLAGS =  -g -O2 -Wall
CFLAGS = ${FLAGS} -I/home/sandeep/unpv13e/lib

all: client server

get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c

server:	server.o common_lib.o get_ifi_info_plus.o np_queue.o myrtt.o 
	${CC} -o server server.o common_lib.o get_ifi_info_plus.o np_queue.o myrtt.o ${LIBS}

server.o: server.c
	${CC} ${CFLAGS} -c server.c

client:	client.o common_lib.o get_ifi_info_plus.o np_queue.o
	${CC} -o client client.o common_lib.o get_ifi_info_plus.o np_queue.o ${LIBS}

client.o: client.c
	${CC} ${CFLAGS} -c client.c

common_lib.o: common_lib.c
	${CC} ${CFLAGS} -c common_lib.c

np_queue.o: np_queue.c
	${CC} ${CFLAGS} -c np_queue.c

myrtt.o: myrtt.c
	${CC} ${CFLAGS} -c myrtt.c
	
clean:
	rm -rf *.o client server

