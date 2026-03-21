#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port number>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = ntohl(INADDR_ANY);

    const int serv_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serv_fd == -1)
    {
        perror("socket");
        return 1;
    }

    if (bind(serv_fd, (const struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    const char* buffer = "Hello World\n";
    const int buffer_size = strlen(buffer);

    for (;;)
    {
        int bytes = recvfrom(serv_fd, NULL, 0, 0, (struct sockaddr*) &client_addr, &client_len);
        if (bytes == -1)
        {
            perror("recvfrom");
            return 1;
        }

        if (sendto(serv_fd, buffer, buffer_size, 0, (struct sockaddr*) &client_addr, client_len) == -1)
        {
            perror("sendto");
            return 1;
        }
    }
}