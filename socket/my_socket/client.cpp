#include "socket/socket.h"

int main()
{
    my_socket::Socket socket;
    if (!socket.connect("127.0.0.1", 10000))
    {
        printf("failed to connect to server\n");
        return 1;
    }

    std::string data = "hello world";
    int send_len = socket.send(data.c_str(), data.size());
    if (send_len < 0)
    {
        printf("failed to send data to server\n");
        return 1;
    }

    char buf[1024] = {0};
    int recv_len = socket.recv(buf, sizeof(buf));
    if (recv_len < 0)
    {
        printf("failed to recv data from server\n");
        return 1;
    }
    printf("recv from server: %s\n", buf);

    socket.close();

    return 0;
}