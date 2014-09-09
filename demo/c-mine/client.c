#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"

const int RCVBUFSIZE  = 256;   /* Size of receive buffer */
const int THREAD_NUM  = 64;    /* Concurrent Thread connect to server*/


struct ThreadArgs
{
    char* serv_ip;
    unsigned short serv_port;
    int total_lines;
};

pthread_mutex_t work_mutex;

static int current_line;

static char **lines_data;

static int
get_total_line(char* serv_ip, unsigned short serv_port)
{
    int sock;
    struct sockaddr_in serv_addr;
    int bytes_recved;

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket() failed");
        return -1;
    }

    /* Construct the server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
    serv_addr.sin_port        = htons(serv_port);

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect() failed");
        return -1;
    }

    char buf[] = "-1";
    /* Send the string to the server */
    if (send(sock, buf, sizeof(buf), 0) != sizeof(buf))
    {
        perror("send() sent a different number of bytes than expected");
        return -1;
    }

    char recv_buffer[11];
    memset(recv_buffer, 0, sizeof(recv_buffer));
    if ((bytes_recved = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0)) <= 0)
    {
        if (bytes_recved==0)
        {
            return -1;
        }
        perror("recv() failed or connection closed prematurely");
        return -1;
    }

    int total_lines = atoi(recv_buffer);
    if (total_lines<=0)
    {
        fprintf(stderr, "error: get wrong total lines number.");
        return -1;
    }

    lines_data = malloc(total_lines * sizeof(Line*));
    if (lines_data == NULL)
    {
        perror("out of memory");
        return -1;
    }
    //memset((void*)lines_data, NULL, total_lines);
    return total_lines;
}



static int
connect_to_server(struct ThreadArgs* p)
{
    char* serv_ip = p->serv_ip;
    unsigned short serv_port = p->serv_port;
    int total_lines = p->total_lines;

    int sock;                               /* Socket descriptor */
    struct sockaddr_in serv_addr;           /* Server address */

    char ack[32];                           /* String to ack server */
    int ack_len;

    //const int memory_block = 1024*1204;

    //char recv_buffer[RCVBUFSIZE];           /* Buffer for recieve string */
    int bytes_recved;

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket() failed");
        return -1;
    }

    /* Construct the server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
    serv_addr.sin_port        = htons(serv_port);

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect() failed");
        return -1;
    }


    while(1)
    {
        int c_line;
        pthread_mutex_lock(&work_mutex);
        c_line = current_line;
        current_line ++;
        pthread_mutex_unlock(&work_mutex);

        if (c_line > total_lines) break;

        ack_len = sprintf(ack, "%d", c_line);
        //printf("--> ask: %d/%d\n", part, total_part);

        /* Send the string to the server */
        if (send(sock, ack, ack_len, 0) != ack_len)
        {
            perror("send() sent a different number of bytes than expected");
            return -1;
        }

        char *recv_buffer = malloc(RCVBUFSIZE*sizeof(char));
        if ((bytes_recved = recv(sock, recv_buffer, RCVBUFSIZE - 1, 0)) <= 0)
        {
            if (bytes_recved==0)
            {
                return 0;
            }
            perror("recv() failed or connection closed prematurely");
            return -1;
        }

        recv_buffer[bytes_recved] = '\0';
        lines_data[c_line] = recv_buffer;

    }
    return 0;

}


void *thread_main(void *thread_args)
{

    struct ThreadArgs *p = (struct ThreadArgs *) thread_args;

    connect_to_server(p);

    return NULL;
}


int main(int argc, char *argv[])
{
    clock_t tstart,tend;
    tstart=clock();
    char *serv_ip;              /* Server IP address (dotted quad) */
    unsigned short serv_port;   /* Server port */


    pthread_t *threadID;
    struct ThreadArgs thread_args;
    int thread_num = THREAD_NUM;


    if ((argc < 3 || argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port> [Thread Num]\n", argv[0]);
        exit(1);
    }

    serv_ip = argv[1];             /* server IP address (dotted quad) */
    serv_port = atoi(argv[2]);     /* server port */
    if (serv_port <=1024)
    {
        fprintf(stderr, "illegal port number <%d>\n", serv_port);
        exit(EXIT_FAILURE);
    }

    if ( argc == 4)
    {
        thread_num = atoi(argv[3]);
        if (thread_num <= 0 )
        {
            thread_num = THREAD_NUM;
        }
    }
    int total_lines;
    if ( (total_lines = get_total_line(serv_ip, serv_port)) == -1)
    {
        fprintf(stderr, "illegal total line from server\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&work_mutex, NULL);
    current_line = 0;

    threadID = malloc(thread_num * sizeof(pthread_t));
    thread_args.serv_ip     = serv_ip;
    thread_args.serv_port   = serv_port;
    thread_args.total_lines = total_lines;
    int i;
    for (i=0; i<thread_num; i++)
    {

        /* Create client thread */
        if (pthread_create(&threadID[i], NULL, thread_main, (void *) &thread_args) != 0)
        {
            perror("pthread_create() failed");
            exit(EXIT_FAILURE);
        }

    }

    /* Wait all client threads */
    for (i=0; i<thread_num; i++)
    {
        void *ret;
        pthread_join(threadID[i], &ret);
    }
    for (i=0; i<total_lines; i++)
    {
        if(i<current_line)
            printf("%s\r\n", lines_data[i]);
        else Sleep(5);
    }
    pthread_mutex_destroy(&work_mutex);

    tend=clock();
    double cost=(double)(tend-tstart)/(double)CLOCKS_PER_SEC;
    printf("%.4f\r\n",cost);

    return 0;
}

