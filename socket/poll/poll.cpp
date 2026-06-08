#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <format>

#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const char* const SERVER_IP = "127.0.0.1";
constexpr uint16_t SERVER_PORT = 10000;
constexpr int MAX_FDS = 1024;

int main()
{
    int server_fd;
    pollfd pfds[MAX_FDS];
    int npfds = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        cerr << format("create socket error: {}", strerror(errno)) << endl;
        exit(1);
    }

    // 设置地址复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << format("server bind error: {}", strerror(errno)) << endl;
        exit(1);
    }

    if (listen(server_fd, 1024) < 0)
    {
        cerr << format("server listen error: {}", strerror(errno)) << endl;
        exit(1);
    }

    // 初始化 fds 全部为 -1（关键！）
    for (int i = 0; i < MAX_FDS; i++)
    {
        pfds[i].fd = -1;
    }

    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;
    npfds = 1;
    cout << "服务器启动，监听端口 10000..." << endl;

    while (1)
    {
        // 调用 poll，阻塞等待事件
        // 参数：数组、有效fd数量、超时(-1=永久等待)
        int ret = poll(pfds, npfds, -1);
        if (ret < 0)
        {
            cerr << format("poll error: {}", strerror(errno)) << endl;
            exit(1);
        }

        for (int i=0;i < npfds;i++)
        {
            if (pfds[i].fd == -1) continue;

            if (pfds[i].revents & POLLIN)
            {
                // 服务器socket（有新客户连接）
                if (pfds[i].fd == server_fd)
                {
                    sockaddr_in client_addr{};
                    socklen_t client_len  = sizeof(client_addr);
                    int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (conn_fd < 0)
                    {
                        cout << format("server accept error: {}", strerror(errno)) << endl;
                        exit(1);
                    }

                    if (npfds >= MAX_FDS)
                    {
                        cerr << "too many clients" << endl;
                        close(conn_fd);
                        continue;
                    }

                    pfds[npfds].fd = conn_fd;
                    pfds[npfds].events = POLLIN;
                    npfds++;

                    cout << format("客户端[{}:{}]已连接！", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)) << endl;
                }
                else
                {
                    char buf[1024] = {0};
                    int read_len = read(pfds[i].fd, buf, sizeof(buf)-1);
                    if (read_len == 0)
                    {
                        cout << format("客户端[fd:{}]主动断开连接！", pfds[i].fd) << endl;
                        close(pfds[i].fd);
                        pfds[i].fd = -1;
                        continue;
                    }

                    if (read_len < 0)
                    {
                        cerr << format("fd[{}] read error: {}", pfds[i].fd, strerror(errno)) << endl;
                        continue;
                    }

                    cout << format("客户端[fd:{}]发送消息: {}", pfds[i].fd, buf) << endl;

                    // string msg = "Server received!";
                    string msg = "服务器收到";
                    write(pfds[i].fd, msg.c_str(), msg.size());
                }
            }
        }
    }

    close(server_fd);
    return 0;
}