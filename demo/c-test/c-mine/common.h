#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


typedef struct _line {
    char* data; /*the raw data*/
    unsigned char len;
    char* buf;  /* the buffer processing hte data*/
    unsigned char buf_len; 
    int line_no;
} Line;

extern Line** lines;
extern long long line_num;

extern void die_with_message(char* msg);
extern void fuck_line(Line* line);
extern int read_data(char* fname);
extern void get_page_range(int page, int total, int* start, int* end);

extern int make_socket_non_blocking (int fd);
extern int make_socket_blocking (int fd);
extern int set_socket_keepalive(int fd, int keep_alive);
extern int set_socket_linger(int fd, int linger);
extern int set_socket_recv_buffer(int fd, int size);
extern int set_socket_send_buffer(int fd, int size);
extern int set_socket_max_buffer(int fd);
extern int set_socket_nodelay(int fd, int no_delay);

#endif
