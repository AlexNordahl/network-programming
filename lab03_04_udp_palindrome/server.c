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

bool verify(const unsigned char* input, ssize_t size);
bool is_palindrome(const unsigned char* input, ssize_t size);
bool is_letter(const unsigned char c);
struct count count(const unsigned char* input, ssize_t size);

int main()
{
    const int dgram_size = 65507;
    const int reply_size = 64;
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

    unsigned char buffer[dgram_size];
    char reply[reply_size];

    for (;;)
    {
        ssize_t bytes = recvfrom(serv_fd, buffer, dgram_size, 0, (struct sockaddr*) &client_addr, &client_len);
        if (bytes == -1)
        {
            perror("recvfrom");
            return 1;
        }

        if (verify(buffer, bytes))
        {
            struct count c = count(buffer, bytes);
            snprintf(reply, sizeof(reply), "%d/%d", c.plnd, c.words);
        }
        else
        {
            snprintf(reply, sizeof(reply), "ERROR");
        }

        if (sendto(serv_fd, reply, strlen(reply), 0, (struct sockaddr*) &client_addr, client_len) == -1)
        {
            perror("sendto");
            return 1;
        }
    }
}

bool verify(const unsigned char* input, ssize_t size)
{
    if (size == 0)
        return true;

    enum State
    {
        LETTER,
        SPACE,
        ERROR
    };

    enum State state = SPACE;
    for (int i = 0; i < size; ++i)
    {
        unsigned char curr = (unsigned char) *input;
        switch (state)
        {
            case LETTER:
            {
                if (is_letter(curr))
                    state = LETTER;
                else if (curr == ' ')
                    state = SPACE;
                else
                    state = ERROR;
                
                break;
            }

            case SPACE:
            {
                if (is_letter(curr))
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

bool is_palindrome(const unsigned char* begin, ssize_t size)
{
    const unsigned char* end = begin + size - 1;
    while (begin < end)
    {   
        if (tolower((unsigned char)*begin) != tolower((unsigned char)*end))
            return false;

        ++begin;
        --end;
    }

    return true;
}

bool is_letter(const unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

struct count count(const unsigned char* input, ssize_t size)
{
    struct count result;
    result.plnd = 0;
    result.words = 0;

    const unsigned char* word = input;
    int word_size = 0;
    for (int i = 0; i < size; ++i, ++input)
    {
        if (is_letter((unsigned char) *input))
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