#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <iostream>

int init_server(int port_number);
std::pair<int, int> count(const unsigned char* input, ssize_t size);  // plnd, words
char to_lower(const unsigned char c);
bool is_letter(const unsigned char c);

class StateMachine
{
public:
    void consume(unsigned char c);
    void reset();

    bool isError() { return is_error; }
    bool isTerminator() { return m_state == TERMINATOR; }
    int getConsumed() { return consumed; }

private:
    enum State
    {
        LETTER,
        SPACE,
        CARET,
        TERMINATOR,
    };

    State m_state{SPACE};
    bool is_error{};
    int consumed{};
};

struct ClientContext
{
    StateMachine sm;
    std::string current_line;
};

int main()
{
    const int port_number = 2020;

    int serv_fd = init_server(port_number);
    if (serv_fd == -1)
        return 1;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return 1;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serv_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &event) == -1)
    {
        perror("epoll_ctl");
        return 1;
    }

    std::unordered_map<int, ClientContext> clients;

    const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];
    unsigned char buffer[1024];

    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            perror("epoll_wait");
            return 1;
        }

        for (int i = 0; i < num_events; ++i)
        {
            int current_fd = events[i].data.fd;

            if (current_fd == serv_fd)
            {
                int client_fd = accept(serv_fd, NULL, NULL);
                if (client_fd == -1)
                {
                    perror("accept");
                    continue;
                }

                event.events = EPOLLIN;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    perror("epoll_ctl");
                    return 1;
                }

                clients[client_fd] = ClientContext{};
            }
            else
            {
                int bytes = read(current_fd, buffer, sizeof(buffer));
                if (bytes <= 0)
                {
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL) == -1)
                    {
                        perror("epoll_ctl");
                        return 1;
                    }

                    if (close(current_fd) == -1)
                    {
                        perror("close");
                        return 1;
                    }

                    clients.erase(current_fd);
                    continue;
                }

                ClientContext& ctx = clients[current_fd];
                for (int j = 0; j < bytes; ++j)
                {
                    unsigned char c = buffer[j];
                    ctx.sm.consume(c);

                    if (c != '\r' && c != '\n')
                    {
                        ctx.current_line += c;
                    }

                    if (ctx.sm.isTerminator())
                    {
                        std::string response{};
                        if (!ctx.sm.isError())
                        {
                            auto [plnd, words] = count(reinterpret_cast<const unsigned char*>(ctx.current_line.c_str()), ctx.current_line.length());
                            response = std::to_string(plnd) + "/" + std::to_string(words) + "\r\n";
                        }
                        else
                        {
                            response = "ERROR\r\n";
                        }

                        int bytes_sent = write(current_fd, response.c_str(), response.length());
                        if (bytes_sent == -1)
                        {
                            perror("write");
                            return 1;
                        }
                        
                        ctx.current_line.clear();
                        ctx.sm.reset();
                    }
                }
            }
        }
    }
}

bool is_letter(const unsigned char c) { return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z'); }

char to_lower(const unsigned char c)
{
    if (c >= 'A' and c <= 'Z')
        return c - 'A' + 'a';

    return c;
}

int init_server(int port_number)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_number);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd == -1)
    {
        perror("socket");
        return -1;
    }

    if (bind(serv_fd, (const struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return -1;
    }

    if (listen(serv_fd, 1) == -1)
    {
        perror("listen");
        return -1;
    }

    return serv_fd;
}

bool is_palindrome(const unsigned char* begin, ssize_t size)
{
    const unsigned char* end = begin + size - 1;
    while (begin < end)
    {
        if (to_lower(*begin) != to_lower(*end))
            return false;

        ++begin;
        --end;
    }

    return true;
}

std::pair<int, int> count(const unsigned char* input, ssize_t size)
{
    std::pair<int, int> result;

    const unsigned char* word = input;
    int word_size = 0;
    for (int i = 0; i < size; ++i, ++input)
    {
        if (is_letter((unsigned char)*input))
        {
            if (word_size == 0)
                word = input;

            ++word_size;
        }
        else if (*input == ' ')
        {
            if (is_palindrome(word, word_size))
                ++result.first;

            word_size = 0;
            ++result.second;
        }
    }

    if (word_size > 0)
    {
        if (is_palindrome(word, word_size))
            ++result.first;

        word_size = 0;
        ++result.second;
    }

    return result;
}

void StateMachine::consume(unsigned char c)
{
    switch (m_state)
    {
        case LETTER:
        {
            if (is_letter(c))
                m_state = LETTER;
            else if (c == '\r')
                m_state = CARET;
            else if (c == ' ')
                m_state = SPACE;
            else
                is_error = true;

            break;
        }
        case SPACE:
        {
            if (is_letter(c))
                m_state = LETTER;
            else if (c == '\r')
                m_state = CARET;
            else
                is_error = true;

            break;
        }
        case CARET:
        {
            if (c == '\n')
                m_state = TERMINATOR;
            else
                is_error = true;

            break;
        }
        case TERMINATOR:
        {
            m_state = TERMINATOR;
            break;
        }
    }

    ++consumed;
}

void StateMachine::reset()
{
    m_state = SPACE;
    is_error = false;
    consumed = 0;
}
