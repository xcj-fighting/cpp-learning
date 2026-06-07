#include "socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>

#include <stdbool.h>

using namespace my_socket;

Socket::Socket() : m_ip(""), m_port(0), m_sockfd(0)
{
    m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sockfd < 0)
    {
        printf("create socket error: errno=%d errmsg-%s\n", errno, strerror(errno));
    }
}

Socket::Socket(int sockfd) : m_ip(""), m_port(0), m_sockfd(sockfd)
{
}

Socket::~Socket()
{
    close();
}

bool Socket::bind(const std::string& ip, int port)
{
    sockaddr_in socket_addr;
    std::memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;

    if (ip.empty()) socket_addr.sin_addr.s_addr = INADDR_ANY;
    else socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    socket_addr.sin_port = htons(port);

    if (::bind(m_sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) < 0)
    {
        printf("socket bind error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }

    m_ip = ip;
    m_port = port;

    return true;
}

bool Socket::listen(int backlog)
{
    if (::listen(m_sockfd, backlog) < 0)
    {
        printf("socket listen error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }

    printf("socket listen success: ip=%s port=%d\n", m_ip.c_str(), m_port);
    return true;
}

bool Socket::connect(const std::string& ip, int port)
{
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);
    
    if (::connect(m_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("socket connect error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }

    printf("socket connect success: ip=%s port=%d\n", ip.c_str(), port);
    return true;
}

int Socket::accept()
{
    int connfd = ::accept(m_sockfd, NULL, NULL);
    if (connfd < 0)
    {
        printf("socket accept error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return -1;
    }
    return connfd;
}

int Socket::send(const char* data, int len)
{
    ssize_t send_len = ::send(m_sockfd, data, len, 0);
    if (send_len < 0)
    {
        printf("socket send error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return -1;
    }
    return send_len;
}

int Socket::recv(char* data, int len)
{
    ssize_t recv_len = ::recv(m_sockfd, data, len, 0);
    if (recv_len < 0)
    {
        printf("socket recv error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return -1;
    }
    return recv_len;
}

void Socket::close()
{
    if (m_sockfd >= 0)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
    }
}

// 设置非阻塞模式
bool Socket::set_non_blocking()
{
    int flags = ::fcntl(m_sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        printf("socket set non blocking error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }

    flags |= O_NONBLOCK;
    if (::fcntl(m_sockfd, F_SETFL, flags) < 0)
    {
        printf("socket set non blocking error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

// 设置发送缓冲区大小
bool Socket::set_send_buffer_size(int size)
{
    int buff_size = size;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, &buff_size, sizeof(buff_size)) < 0)
    {
        printf("socket set send buffer size error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

// 设置接收缓冲区大小
bool Socket::set_recv_buffer_size(int size)
{
    int buff_size = size;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(buff_size)) < 0)
    {
        printf("socket set recv buffer size error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

// 设置linger延迟关闭时间
bool Socket::set_linger(bool active, int seconds)
{
    struct linger l;
    memset(&l, 0, sizeof(l));
    l.l_onoff = active;
    l.l_linger = seconds;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0)
    {
        printf("socket set linger error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

// 设置keepalive
bool Socket::set_keepalive()
{
    int flag = 1;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0)
    {
        printf("socket set keepalive error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

// 设置reuseaddr
bool Socket::set_reuseaddr()
{
    int flag = 1;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
    {
        printf("socket set reuseaddr error: errno=%d errmsg-%s\n", errno, strerror(errno));
        return false;
    }
    return true;
}