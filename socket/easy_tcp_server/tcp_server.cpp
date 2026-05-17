#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main()
{
    // 1.创建socket ::表示全局作用域的socket
    int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0)
    {
        printf("create socket error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    int opt = 1;
    ::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2.绑定 socket
    std::string ip = "127.0.0.1";
    int port = 8080;

    struct sockaddr_in socket_addr;
    std::memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socket_addr.sin_port = htons(port); // host to network short
    if (::bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) < 0)
    {
        printf("socket bind error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    // 3. 监听
    if (::listen(socket_fd, 1024) < 0)
    {
        printf("socket listen error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return 1;
    }

    while(true)
    {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_len = sizeof(client_addr);
        
        // 4. 接收客户端连接
        int connfd = ::accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if (connfd < 0)
        {
            printf("socket accept error: errno=%d errmsg=%s", errno, strerror(errno));
            return 1;
        }

        // 打印客户端的地址和端口
        printf("client addr=%s\n", inet_ntoa(client_addr.sin_addr));    // network to ascii
        printf("client port=%d\n", ntohs(client_addr.sin_port));    // network to host short

        char buf[1024] = {0};

        // 5. 接收客户端的数据
        ssize_t recv_len = ::recv(connfd, buf, sizeof(buf), 0);
        // recv_len == 0 表示客户端断开连接
        if (recv_len == 0)
        {
            printf("client disconnected\n");
            ::close(connfd);
            continue;
        }

        if (recv_len < 0)
        {
            printf("recv from client error: errno=%d errmsg=%s", errno, strerror(errno));
            ::close(connfd);
            continue;
        }

        printf("recv from client: %d msg_len=%ld msg=%s\n", connfd, recv_len, buf);

        // 6. 发送数据给客户端
        ssize_t send_len = ::send(connfd, buf, recv_len, 0);
        if (send_len < 0)
        {
            printf("send to client error: errno=%d errmsg=%s", errno, strerror(errno));
            ::close(connfd);
            continue;
        }
    }
    
    // 7. 关闭 socket
    ::close(socket_fd);

    return 0;
}
