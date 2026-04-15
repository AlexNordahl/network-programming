#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

int init_server(int port_number, int connections);
std::pair<int, int> count(std::string_view sv);  // plnd, words
bool is_palindrome(std::string_view sv);

class StateMachine
{
public:
    void consume(unsigned char c) noexcept;
    void reset() noexcept;

    bool isError() const noexcept { return m_is_error; }
    bool isTerminator() const noexcept { return m_state == State::TERMINATOR; }

private:
    enum class State
    {
        START,
        LETTER,
        SPACE,
        CARET,
        TERMINATOR,
    };

    State m_state{State::START};
    bool m_is_error{};
};

int main()
{
    const int port_number = 2020;
    const int connections = 100;

    int serv_fd = init_server(port_number, connections);
    if (serv_fd == -1)
        return 1;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return 1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serv_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &event) == -1)
    {
        perror("epoll_ctl");
        return 1;
    }

    struct ClientContext
    {
        StateMachine sm;
        std::string current_line;
    };

    std::unordered_map<int, ClientContext> clients;
    std::array<epoll_event, 10> events;
    std::array<unsigned char, 1024> buffer;

    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events.data(), events.size(), -1);
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
                int client_fd = accept(serv_fd, nullptr, nullptr);
                if (client_fd == -1)
                {
                    perror("accept");
                    return 1;
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
                ssize_t bytes = read(current_fd, buffer.data(), buffer.size());
                if (bytes <= 0)
                {
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr) == -1)
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
                for (ssize_t j = 0; j < bytes; ++j)
                {
                    unsigned char c = buffer[j];
                    ctx.sm.consume(c);

                    if (c != '\r' and c != '\n')
                    {
                        ctx.current_line += c;
                    }

                    if (ctx.sm.isTerminator())
                    {
                        std::string response{};
                        if (!ctx.sm.isError())
                        {
                            auto [plnd, words] = count(ctx.current_line);
                            response = std::to_string(plnd) + "/" + std::to_string(words) + "\r\n";
                        }
                        else
                        {
                            response = "ERROR\r\n";
                        }

                        ssize_t bytes_sent = write(current_fd, response.c_str(), response.length());
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

int init_server(int port_number, int connections)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_number);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd == -1)
    {
        perror("socket");
        return -1;
    }

    if (bind(serv_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        perror("bind");
        close(serv_fd);
        return -1;
    }

    if (listen(serv_fd, connections) == -1)
    {
        perror("listen");
        close(serv_fd);
        return -1;
    }

    return serv_fd;
}

bool is_palindrome(std::string_view sv)
{
    if (sv.empty())
        return true;

    std::size_t i = 0, j = sv.size() - 1;
    while (i < j)
    {
        if (std::tolower(static_cast<unsigned char>(sv[i])) != std::tolower(static_cast<unsigned char>(sv[j])))
            return false;

        ++i;
        --j;
    }

    return true;
}

std::pair<int, int> count(std::string_view sv)
{
    std::pair result{0, 0};

    const char* word = nullptr;
    std::size_t word_size = 0;
    for (std::size_t i = 0; i < sv.size(); ++i)
    {
        unsigned char c = sv[i];
        if (std::isalpha(c))
        {
            if (word_size == 0)
                word = sv.data() + i;

            ++word_size;
        }
        else if (c == ' ')
        {
            if (word_size > 0)
            {
                if (is_palindrome({word, word_size}))
                    ++result.first;

                ++result.second;
                word_size = 0;
            }
        }
    }

    if (word_size > 0)
    {
        if (is_palindrome({word, word_size}))
            ++result.first;

        ++result.second;
    }

    return result;
}

void StateMachine::consume(unsigned char c) noexcept
{
    switch (m_state)
    {
        case State::START:
        {
            if (std::isalpha(c))
                m_state = State::LETTER;
            else if (c == '\r')
                m_state = State::CARET;
            else
                m_is_error = true;

            break;
        }
        case State::LETTER:
        {
            if (std::isalpha(c))
                m_state = State::LETTER;
            else if (c == '\r')
                m_state = State::CARET;
            else if (c == ' ')
                m_state = State::SPACE;
            else
                m_is_error = true;

            break;
        }
        case State::SPACE:
        {
            if (std::isalpha(c))
                m_state = State::LETTER;
            else if (c == '\r')
            {
                m_is_error = true;
                m_state = State::CARET;
            }
            else
                m_is_error = true;

            break;
        }
        case State::CARET:
        {
            if (c == '\n')
                m_state = State::TERMINATOR;
            else
                m_is_error = true;

            break;
        }
        case State::TERMINATOR:
        {
            break;
        }
    }
}

void StateMachine::reset() noexcept
{
    m_state = State::START;
    m_is_error = false;
}
