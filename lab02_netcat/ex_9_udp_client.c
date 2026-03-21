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

    if (sendto(client_fd, NULL, 0, 0, (struct sockaddr*) &addr, addr_len) == -1)
    {
        perror("sendto");
        return 1;
    }

    char buffer[1024];
    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes == -1)
    {
        perror("recv");
        return 1;
    }

    buffer[bytes] = '\0';

    if (is_printable(buffer, bytes))
        printf("%s", buffer);
}