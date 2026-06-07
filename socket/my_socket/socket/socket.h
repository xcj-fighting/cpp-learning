#ifndef SOCKET_H
#define SOCKET_H

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <cstring>
#include <string>

namespace my_socket{
    class Socket{
        public:
            Socket();
            Socket(int sockfd);
            ~Socket();

            bool bind(const std::string& ip, int port);
            bool listen(int backlog = 1024);
            bool connect(const std::string& ip, int port);
            int accept();
            int send(const char* data, int len);
            int recv(char* data, int len);
            void close();

            // 设置非阻塞模式
            bool set_non_blocking();
            // 设置发送缓冲区大小
            bool set_send_buffer_size(int size);
            // 设置接收缓冲区大小
            bool set_recv_buffer_size(int size);
            // 设置linger延迟关闭时间
            bool set_linger(bool active, int seconds);
            // 设置keepalive
            bool set_keepalive();
            // 设置reuseaddr
            bool set_reuseaddr();

        protected:
            std::string m_ip;
            int m_port;
            int m_sockfd;
    };
}

#endif