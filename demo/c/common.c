#include "common.h"


Line** lines;

long long line_num = 0;

void die_with_message(char* msg)
{
    perror(msg);
    exit(-1);
}


void fuck_line(Line* line)
{
    int len = line->len;
    char *start =  line->data;
    char *end = start + len - 1;
    int p = len/3;
    char *delete_begin = line->data + p ; 
    char *delete_end = delete_begin + p - 1;

    line->buf_len = sprintf(line->buf, "%d", line->line_no);
    
    char* pbuf = line->buf + line->buf_len;

    // erase & reverse the string
    for( ;start <= end; end--) {
        if (end == delete_end){
            end = delete_begin;
            continue;
        }
        *pbuf = *end; 
        pbuf++;
    }
    *pbuf = '\0';
    line->buf_len = pbuf - line->buf;

}

int read_data(char* fname)
{
    FILE * fp = NULL;
    line_num = 0;
    char* data = NULL;
    size_t len;
    ssize_t readsz;
    int i;
    const int memory_block = 1024*1024;
    int lines_allocated = memory_block;

    fp = fopen(fname, "r");
    if (fp == NULL) {
        die_with_message("read data file error");
    }

    lines = (Line**)malloc(lines_allocated * sizeof(Line*));
    if (lines == NULL){
        die_with_message("out of memory");
    }

    for (i=0;1;i++) {
        if ((readsz = getline(&data, &len, fp)) == -1) {
            break; 
        }

        Line* pline  = (Line*)malloc(sizeof(Line));
        if (pline == NULL){
            die_with_message("out of memory");
        }
        pline->len = readsz;
        pline->data = data;
        pline->line_no = i;
        /*remove "/r/n" */
        for(data = pline->data+readsz-1; *data=='\n'||*data=='\r'; data--, pline->len--);
        *(data+1) = '\0';

        /*add 10 more for the line no*/
        pline->buf = malloc(pline->len + 10);
        pline->buf_len = 0;

        if (i >= lines_allocated) {
            int new_size = lines_allocated + memory_block; 
            lines = (Line**)realloc(lines, new_size * sizeof(Line*));
            if (lines == NULL){
                die_with_message("out of memory");
            }
            lines_allocated = new_size;
        }

        lines[i] = pline;

        data = NULL;
        line_num++;
    }

    return 0;
}


void get_page_range(int page, int total, int* start, int* end)
{
    long long size = line_num / total;

    *start = (page-1) * size;
    if (page < total ){
        *end = *start + size -1; 
    }else{
        *end = line_num - 1;
    }
}

    static int
make_socket(int fd, char blocking)
{
    int flags, s;

    flags = fcntl (fd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    if (blocking) {
        flags &= (~O_NONBLOCK);
    }else{
        flags |= O_NONBLOCK;
    }

    s = fcntl (fd, F_SETFL, flags);
    if (s == -1) {
        perror ("fcntl");
        return -1;
    }

    return 0;

}
int make_socket_non_blocking (int fd)
{
    return make_socket(fd, 0);
}

int make_socket_blocking (int fd)
{
    return make_socket(fd, 1);
}

#define OPTION_ENABLE   1
#define OPTION_DISABLE  2

int set_socket_keepalive(int fd, int keep_alive) 
{
    int opt_val = keep_alive ? OPTION_ENABLE : OPTION_DISABLE;
    socklen_t opt_len = sizeof(int);

    if ( setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt_val, opt_len) != 0 ) {
        perror("set SO_KEEPALIVE");
        return -1;
    }

    return 0;
}

int set_socket_linger(int fd, int linger) 
{
    struct linger lg;
    if (linger) {
        lg.l_onoff = OPTION_ENABLE;
        lg.l_linger = 0;
    } else {
        lg.l_onoff = OPTION_DISABLE;
    }

    if ( setsockopt(fd, SOL_SOCKET, SO_LINGER, &lg, sizeof(struct linger)) != 0 ) {
        perror("set SO_LINGER");
        return -1;
    }
    return 0;
}

int set_socket_recv_buffer(int fd, int size) 
{
    if ( setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) != 0 ) {
        perror("set SO_RCVBUF");
        return -1;
    }
    return 0;
}

int set_socket_send_buffer(int fd, int size) 
{
    if ( setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int)) != 0 ) {
        perror("set SO_SNDBUF");
        return -1;
    }
    return 0;
}

#define MAXSNDBUF 16777216
#define MAXRCVBUF 16777216
int set_socket_max_buffer(int fd)
{
    if (set_socket_send_buffer(fd, MAXSNDBUF)<-1 || set_socket_recv_buffer(fd, MAXRCVBUF)<0){
         return -1;
    }
    return 0;
}

int set_socket_nodelay(int fd, int no_delay) 
{
    int opt_val = no_delay ? OPTION_ENABLE : OPTION_DISABLE;
    if ( setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(int)) != 0 ) {
        perror("set TCP_NODELAY"); 
        return -1;
    }
    return 0;
}


