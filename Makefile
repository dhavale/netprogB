
CC = gcc

LIBS = /home/sandeep/unpv13e/libunp.a

FLAGS =  -g -O2
CFLAGS = ${FLAGS} -I/home/sandeep/unpv13e/lib

all: get_ifi_info_plus.o print_unicast.o
	${CC} -o prifinfo_plus print_unicast.o get_ifi_info_plus.o ${LIBS}


get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c

print_unicast.o: print_unicast.c
	${CC} ${CFLAGS} -c print_unicast.c

clean:
	rm prifinfo_plus print_unicast.o get_ifi_info_plus.o

