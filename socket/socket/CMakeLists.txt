cmake_minimum_required(VERSION 3.10)
project(socket)

set(SOURCE_FILES
    socket/socket.cpp
    server.cpp
)

add_executable(server ${SOURCE_FILES})
add_executable(client socket/socket.cpp client.cpp)
