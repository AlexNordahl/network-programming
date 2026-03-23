#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool is_printable(const char* begin, int len)
{
    const char* end = begin + len;
    while (begin != end)
    {
        if (!isprint((unsigned char)*begin) && *begin != '\n' && *begin != '\r' && *begin != '\t')
            return false;

        ++begin;
    }
    return true;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip address> <port number>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    socklen_t addr_len = sizeof(addr);

    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd == -1)
    {
        perror("socket");
        return 1;
    }

    const int msg_size = 1025;
    char message[msg_size];
    for (size_t i = 0; i < msg_size; i++)
    {
        if (i % 2 == 0)
            message[i] = 'a';
        else
            message[i] = ' ';
    }
    message[msg_size - 2] = 'a';
    message[msg_size - 1] = '\0';

    printf("%s\n", message);

    if (sendto(client_fd, message, sizeof(message), 0, (struct sockaddr*) &addr, addr_len) == -1)
    {
        perror("sendto");
        return 1;
    }

    char reply[1024];
    int bytes = recv(client_fd, reply, sizeof(reply) - 1, 0);
    if (bytes == -1)
    {
        perror("recv");
        return 1;
    }

    reply[bytes] = '\0';

    if (is_printable(reply, bytes))
        printf("%s", reply);
}