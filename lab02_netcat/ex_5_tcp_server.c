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

    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(serv_fd, 1) == -1)
    {
        perror("listen");
        return 1;
    }

    char* buf = "Hello, world\r\n";
    const int buf_size = strlen(buf);
    for (;;)
    {
        int write_fd = accept(serv_fd, NULL, NULL);
        if (write_fd == -1)
        {
            perror("accept");
            return 1;
        }

        int bytes = write(write_fd, buf, buf_size);
        if (bytes == -1 || bytes < buf_size)
        {
            perror("write");
            return 1;
        }

        if (close(write_fd) == -1)
        {
            perror("close");
            return 1;
        }
    }
}