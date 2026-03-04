#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <filename1> <filename2>", argv[0]);
        return 1;
    }

    int read_fd = open(argv[1], O_RDONLY);
    if (read_fd == -1)
    {
        perror("open");
        return 1;
    }

    int write_fd = open(argv[2], O_WRONLY);

    if (write_fd == -1)
    {
        perror("open");
        return 1;
    }

    char buf[1024];

    for (;;)
    {
        int bytes = read(read_fd, buf, sizeof(buf));

        if (bytes == 0)
            break;

        if (bytes < 0)
        {
            perror("read");
            return 1;
        }

        if (write(write_fd, buf, bytes) <= 0)
        {
            perror("write");
            return 1;
        }
    }
    
    close(write_fd);
    close(read_fd);

    return 0;
}
