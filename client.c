#include "config.h"
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

void *sender(void *data)
{
    int64_t sock = (int64_t) data;
    assert(sock >= 0);

    char buf[CLIENT_BUFSIZE];

    while(1)
    {
        ssize_t nbytes = 0;
        if ((nbytes = read(STDIN_FILENO, buf, CLIENT_BUFSIZE)) == -1)
            break;

        if (send(sock, buf, nbytes, 0) != nbytes)
            break;
    }

    fprintf(stderr, "Sender error");

    return NULL;
}

void *receiver(void *data)
{
    int64_t sock = (int64_t) data;
    assert(sock >= 0);

    char buf[CLIENT_BUFSIZE];

    while(1)
    {
        ssize_t nbytes = 0;
        if ((nbytes = recv(sock, buf, CLIENT_BUFSIZE, 0)) == -1)
            break;

        if (write(STDOUT_FILENO, buf, nbytes) != nbytes)
            break;
    }

    fprintf(stderr, "Receiver error");

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <ip address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
    };

    if (inet_aton(argv[1], &addr.sin_addr) == 0)
    {
        fprintf(stderr, "Bad ip address\n");
        exit(EXIT_FAILURE);
    }

    int64_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        error(EXIT_FAILURE, errno, "socket");
    
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        error(EXIT_FAILURE, errno, "connect");

    pthread_t sender_id;
    pthread_t receiver_id;

    if (pthread_create(&sender_id, NULL, sender, (void *) sock) == -1)
        error(EXIT_FAILURE, errno, "pthread_create");
    if (pthread_create(&receiver_id, NULL, receiver, (void *) sock) == -1)
        error(EXIT_FAILURE, errno, "pthread_create");

    if (pthread_join(sender_id, NULL) == -1)
        error(EXIT_FAILURE, errno, "pthread_join");
    if (pthread_join(receiver_id, NULL) == -1)
        error(EXIT_FAILURE, errno, "pthread_join");
};
