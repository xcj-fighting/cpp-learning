#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <format>

using namespace std;

constexpr int MAX_EVENTS = 1024;
const char* const SERVER_IP = "127.0.0.1";
constexpr uint16_t SERVER_PORT = 10000;

int main()
{
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd < 0)
    {
        cerr << format("create socket error: {}", strerror(errno)) << endl;
        exit(1);
    }

    // 设置地址和端口复用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        cerr << format("setsockopt error: {}", strerror(errno)) << endl;
        close(server_fd);
        exit(1);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << format("server bind error: {}", strerror(errno)) << endl;
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 1024) < 0)
    {
        cerr << format("server listen error: {}", strerror(errno)) << endl;
        close(server_fd);
        exit(1);
    }

    // 创建 epoll 实例
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1)
    {
        cerr << format("epoll_create1 error: {}", strerror(errno)) << endl;
        close(server_fd);
        exit(1);
    }

    // 将server_fd加入epoll监听（读事件）
    struct epoll_event event{};
    event.events = EPOLLIN | EPOLLET; // 监听可读事件
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0)
    {
        std::cerr << std::format("epoll add server error: {}", strerror(errno)) << '\n';
        close(epoll_fd);
        close(server_fd);
        exit(1);
    }

    // 用于接收 epoll 事件的数组    
    struct epoll_event events[MAX_EVENTS];
    while (true)
    {
        int nfds;
        do {
            nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        } while(nfds == -1 && errno == EINTR);

        if (nfds == -1)
        {
            std::cerr << std::format("epoll_wait fatal error: {}\n", strerror(errno));
            break;
        }

        for (int i=0;i < nfds;i++)
        {
            int fd = events[i].data.fd;
            uint32_t revents = events[i].events;

            if (revents & (EPOLLERR | EPOLLHUP))
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                cout << std::format("fd {} disconnect(err/hup)\n", fd);
                continue;
            }

            if (fd == server_fd)
            {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int conn_fd = accept4(fd, (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
                if (conn_fd < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                    cerr << format("accept error: {}", strerror(errno)) << endl;
                    continue;
                }

                cout << format("New client connected: {}:{}\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // 加入监听（读事件）
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = conn_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) == -1)
                {
                    cerr << format("add client epoll_ctl error: {}", strerror(errno)) << endl;
                    close(conn_fd);
                    continue;
                }
            }
            else
            {
                char buf[1024];
                int data_len = read(fd, buf, sizeof(buf) - 1);
                if (data_len == 0 || data_len < 0)
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);

                    if (data_len == 0) cout << format("客户端[{}] 已主动断开！", fd) << endl;
                    if (data_len < 0) cerr << format("fd[{}] read error: {}", fd, strerror(errno)) << endl;
                    continue;
                }

                cout << format("client[{}] send message: {}", fd, buf) << endl;

                string msg = "Server received!";
                write(fd, msg.c_str(), msg.size());
            }
        }
    }

    close(epoll_fd);
    close(server_fd);
    return 0;
}