#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct count
{
    int plnd;
    int words;
};

bool verify(const char* input);
bool is_palindrome(const char* input, const size_t size);
struct count count(const char* input);

int main()
{
    const int port_number = 2020;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_number);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int serv_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

    char buffer[65508];
    char reply[64];

    for (;;)
    {
        int bytes = recvfrom(serv_fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &client_len);
        if (bytes == -1)
        {
            perror("recvfrom");
            return 1;
        }

        buffer[bytes] = '\0';

        if (verify(buffer))
        {
            struct count c = count(buffer);
            snprintf(reply, sizeof(reply), "%d/%d\n", c.plnd, c.words);
        }
        else
        {
            snprintf(reply, sizeof(reply), "ERROR\n");
        }

        if (sendto(serv_fd, reply, strlen(reply), 0, (struct sockaddr*) &client_addr, client_len) == -1)
        {
            perror("sendto");
            return 1;
        }
    }
}

bool verify(const char* input)
{
    if (*input == '\0')
        return true;

    enum State
    {
        LETTER,
        SPACE,
        ERROR
    };

    enum State state = SPACE;
    while (*input != '\0')
    {
        unsigned char curr = (unsigned char) *input;
        switch (state)
        {
            case LETTER:
            {
                if (isalpha(curr))
                    state = LETTER;
                else if (curr == ' ')
                    state = SPACE;
                else
                    state = ERROR;
                
                break;
            }

            case SPACE:
            {
                if (isalpha(curr))
                    state = LETTER;
                else
                    state = ERROR;

                break;
            }

            case ERROR: return false;
        }
        ++input;
    }
    
    return state == LETTER;
}

bool is_palindrome(const char* begin, const size_t size)
{
    const char* end = begin + size - 1;
    while (begin < end)
    {   
        if (tolower((unsigned char)*begin) != tolower((unsigned char)*end))
            return false;

        ++begin;
        --end;
    }

    return true;
}

struct count count(const char* input)
{
    struct count result;
    result.plnd = 0;
    result.words = 0;

    const char* word = input;
    int word_size = 0;
    for (size_t i = 0; *input != '\0'; ++i, ++input)
    {
        if (isalpha(*input))
        {   
            if (word_size == 0) 
                word = input;

            ++word_size;
        }
        else if (*input == ' ')
        {
            if (is_palindrome(word, word_size))
                ++result.plnd;

            word_size = 0;
            ++result.words;
        }
    }

    if (word_size > 0)
    {
        if (is_palindrome(word, word_size))
            ++result.plnd;
    
        word_size = 0;
        ++result.words;
    }

    return result;
}
