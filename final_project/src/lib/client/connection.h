#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>    
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  

#define SERVER_IP "127.0.0.1" // 服务端IP地址
#define SERVER_PORT 8080      // 服务端监听端口

int connect_to_server(){
    //连接服务器
    int sockfd;
    struct sockaddr_in server_addr;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 清零server_addr结构体
    memset(&server_addr, 0, sizeof(server_addr));

    // 设置服务器的地址和端口
    server_addr.sin_family = AF_INET; // 使用IPv4地址
    server_addr.sin_port = htons(SERVER_PORT); // 端口
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // IP地址

    // 发起连接请求
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

