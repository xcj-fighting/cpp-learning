#include <iostream>

#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <format>

using namespace std;

constexpr const char* SERVER_IP = "127.0.0.1";
constexpr uint16_t SERVER_PORT = 10000;
constexpr int MAX_CLIENTS = 1024;

int create_socket();

int main()
{
    int server_fd;      // 服务器监听 socket
    int max_fd;         // 最大文件描述符
    vector<int> client_fds(MAX_CLIENTS, -1); // 存放所有的客户端文件描述符

    fd_set read_fds;    // 监听集合：第i位=1表示需要监听i号fd
    fd_set temp_fds;    // 临时集合（select 会破坏原集合）

    server_fd = create_socket();
    if (server_fd <= 0) exit(1);

    FD_ZERO(&read_fds); // 把文件描述符集合 清空 / 归零
    FD_SET(server_fd, &read_fds);   // 将服务器加入监听集合
    max_fd = server_fd;

    cout << "select server running on port: " << SERVER_PORT << endl;

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1)
    {
        // 每次必须复制！因为 select 会修改集合
        temp_fds = read_fds;

        // 调用 select() 前：你手动往 temp_fds 里加需要监听的文件描述符
        // 调用 select() 后：只有就绪的 fd 留在集合里，未就绪的会被内核清空
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1)
        {
            cerr << "select 失败: " << strerror(errno) << endl;
            exit(1);
        }

        // 遍历 0 ~ max_fd 所有 fd
        for (int i=0;i <= max_fd;i++)
        {
            if (FD_ISSET(i, &temp_fds))
            {
                // 服务端套接字就绪
                if (i == server_fd)
                {
                    int new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    for (int &fd : client_fds)
                    {
                        if (fd == -1)
                        {
                            fd = new_fd;
                            break;
                        }
                    }

                    // 加入监听集合
                    FD_SET(new_fd, &read_fds);
                    if (new_fd > max_fd)
                    {
                        max_fd = new_fd;
                    }
                    cout << format("客户端[{}:{}] 已连接服务端！", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)) << endl;
                }
                else 
                {
                    // 客户端发消息
                    char buf[1024] = {0};
                    int conn_fd = i;
                    int ret = read(conn_fd, buf, sizeof(buf));
                    if (ret == 0)
                    {
                        cout << "客户端连接断开: fd = " << conn_fd << endl;
                        close(i);
                        FD_CLR(conn_fd, &read_fds); // 从监听集合中删除
                        for (int &fd : client_fds)
                        {
                            if (fd == conn_fd) fd = -1;
                        }
                        continue;
                    }
                    else if (ret < 0)
                    {
                        cerr << "fd: " << conn_fd << " read error: " << strerror(errno) << endl;
                    }
                    else
                    {
                        cout << "client[" << conn_fd << "] send message: " << buf << endl;
                        const string msg = "Received";
                        write(conn_fd, msg.c_str(), msg.size());
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cerr<< "socket 创建失败: " << strerror(errno) << endl;
        return -1;
    }

    // 设置端口复用
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};  // 自动全清 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr) <= 0)
    {
        cerr<< "ip转换失败: " << strerror(errno) << endl;
        return -1;
    }

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr<< "addr绑定失败: " << strerror(errno) << endl;
        return -1;
    }

    if (listen(sockfd, 1024) == -1)
    {
        cerr<< "监听失败: " << strerror(errno) << endl;
        return -1;
    }

    return sockfd;
}