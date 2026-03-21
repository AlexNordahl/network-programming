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
        printf("Usage: %s <ip address> <port number>", argv[0]);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        perror("socket");
        return 1;
    }

    if (connect(client_fd, (const struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        perror("connect");
        return 1;
    }

    const int buf_size = 1024;
    char buffer[buf_size];
    int bytes = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes == -1)
    {
        perror("read");
        return 1;
    }

    if (is_printable(buffer, bytes))
        printf("%s", buffer);

    if (close(client_fd) == -1)
    {
        perror("close");
        return 1;
    }
}