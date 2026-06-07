#include "socket/socket.h"

int main()
{
    my_socket::Socket server;
    if (!server.bind("127.0.0.1", 10000))
    {
        printf("failed to bind to port 8080\n");
        return 1;
    }

    if (!server.listen())
    {
        printf("failed to listen\n");
        return 1;
    }

    while (true)
    {
        int connfd = server.accept();
        if (connfd < 0)
        {
            printf("failed to accept\n");
            continue;
        }

        printf("client connected: connfd=%d\n", connfd);

        my_socket::Socket client_socket(connfd);
        char buf[1024] = {0};
        int recv_len = client_socket.recv(buf, sizeof(buf));
        if (recv_len < 0)
        {
            printf("failed to recv\n");
            continue;
        }
        printf("recv from client: %s\n", buf);
        
        client_socket.send(buf, recv_len);
    }

    server.close();
    
    return 0;
}