cc=gcc
CFLAG=-g  -Wall
LIBS=-pthread
all: server client

server:	server.o common.o
	${cc} ${CFLAG} -o server server.o common.o ${LIBS}

client:	client.o common.o
	${cc} ${CFLAG} -o client client.o common.o ${LIBS}

server.o: server.c common.h
	${cc} ${CFLAG} -c -o server.o server.c

client.o: client.c common.h
	${cc} ${CFLAG} -c -o client.o client.c

common.o: common.c common.h
	${cc} ${CFLAG} -c -o common.o common.c

.PHONY clean:
	rm -f *.o server client
