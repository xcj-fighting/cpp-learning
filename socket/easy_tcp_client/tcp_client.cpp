#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // 1. 创建socket
    // AF_INET(Address Family Internet) 表示IPv4地址族
    int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0)
    {
        printf("create socket error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    // 2. 连接服务器
    std::string ip = "127.0.0.1";
    int port = 8080;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    if (::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect to server error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    // 3.向服务端发送数据
    std::string data = "hello world";
    ssize_t send_len = ::send(socket_fd, data.c_str(), data.size(), 0);
    if (send_len < 0)
    {
        printf("send to server error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    // 4. 接收服务端的数据
    char buf[1024] = {0};
    ssize_t recv_len = ::recv(socket_fd, buf, sizeof(buf), 0);
    if (recv_len < 0)
    {
        printf("recv from server error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }
    printf("recv from server: %s\n", buf);

    // 5. 关闭 socket
    ::close(socket_fd);

    return 0;
}