#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "common.h"

void *thread_readData(void *file)
{
    read_data();
}

static int
create_tcp_server_socket (char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, server_sock;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port, &hints, &result);
    if (s != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        server_sock = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_sock == -1) {
            continue;
        }

        s = bind (server_sock, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close (server_sock);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }

    s = listen (server_sock, SOMAXCONN);
    if (s == -1) {
        perror ("listen");
        return -1;
    }

    freeaddrinfo (result);

    return server_sock;
}



static int
accept_tcp_connection(int server_sock)
{
    int client_sock;
    struct sockaddr_in client_addr;
    unsigned int client_addr_len = sizeof(client_addr);

    /* Wait for a client to connect */
    if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len)) < 0) {
        perror("accept() failed");
        return -1;
    }

    /* client_sock is connected to a client! */

//    printf("new client %s\n", inet_ntoa(client_addr.sin_addr));

    return client_sock;
}


static void
handle_tcp_client(int client_sock)
{
    #define BUFSIZE 32
    char recv_buf[BUFSIZE];
    int recv_size;

    int i;

    while(1) {
        /* Receive message from client */
        if ((recv_size = recv(client_sock, recv_buf, BUFSIZE, 0)) < 0){
            perror("recv() failed");
            return;
        }
        if(recv_size>0){
            recv_buf[recv_size] = '\0';  /* Terminate the string! */
            sscanf(recv_buf,"%d", &i);
        }else{
            break;
        }
        if (i>=line_num){
            break;
        }
        if (i == -1){
            char buf[10];
            int len = sprintf(buf, "%lld", line_num);
            if (send(client_sock, buf, len, 0) != len ) {
                perror("send() failed");
            }
            break;
        }

        /* Processing the Line */
        fuck_line(lines[i]);

        if (send(client_sock, lines[i]->buf, lines[i]->buf_len, 0) != lines[i]->buf_len ) {
            perror("send() failed");
            break;
        }
    }
    close(client_sock);
    return;

}

struct ThreadArgs {
    int client_sock; /* Socket descriptor for client */
};

void *thread_main(void *thread_args)
{
    int client_sock;

    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self());

    client_sock = ((struct ThreadArgs *) thread_args) -> client_sock;

    /* Deallocate memory for argument */
    free(thread_args);

    handle_tcp_client(client_sock);

    return (NULL);
}

    int
main (int argc, char *argv[])
{
    int server_sock, client_sock;
    pthread_t threadID;
    struct ThreadArgs *thread_args;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [port] [data file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server_sock = create_tcp_server_socket (argv[1]);
    if (server_sock == -1) {
        exit(-1);
    }

  //  printf("Loading the data....\n");
    read_data(argv[2]);
    printf("Load the data successfully\n");

    for (;;) {

        client_sock = accept_tcp_connection ( server_sock );

        /* Create separate memory for client argument */
        if ((thread_args = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL){
            perror("malloc() failed");
            exit(EXIT_FAILURE);
        }
        thread_args -> client_sock = client_sock;

        /* Create client thread */
        if (pthread_create(&threadID, NULL, thread_main, (void *) thread_args) != 0) {
            perror("pthread_create() failed");
            exit(EXIT_FAILURE);
        }
  //      printf("with thread %ld\n", (long int) threadID);
    }


    close (server_sock);

    return EXIT_SUCCESS;
}

