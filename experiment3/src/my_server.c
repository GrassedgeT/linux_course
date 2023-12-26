#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 10
#define PORT 8080
#define BUFFER_SIZE UINT16_MAX
#define DIST_PATH "/home/cao/Codes/C_Project/linux_course/experiment3/dist"
void start_server(int *server_fd, struct sockaddr_in *server_addr) {
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    if (bind(*server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server started at port %d\n", PORT);
}

void set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

void handle_client_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    buffer[bytes_read] = '\0';

    // 创建 buffer 的副本
    char buffer_copy[BUFFER_SIZE];
    strncpy(buffer_copy, buffer, BUFFER_SIZE);

    // 提取请求类型
    char *method = strtok(buffer_copy, " ");
    // 提取请求内容
    char *uri = strtok(NULL, " ");
   
    printf("Received request : \n\t Type:%s\n\t Target_File:%s\n\t Client_fd:%d\n", method, uri, client_fd);
    // 简单的请求解析
    if (strncmp(buffer, "GET ", 4) != 0) {
        send_400(client_fd);
    } else {
        char *file_path = buffer + 4; // 跳过 "GET "
        char *end_path = strchr(file_path, ' ');
        if (end_path) {
            *end_path = '\0'; // 终止路径字符串

            // 处理根路径映射
            if (strcmp(file_path, "/") == 0) {
                snprintf(file_path, BUFFER_SIZE, "%s/index.html", DIST_PATH);
            } else {
                char temp_path[BUFFER_SIZE];
                snprintf(temp_path, BUFFER_SIZE, "%s%s", DIST_PATH, file_path);
                strcpy(file_path, temp_path);
            }


            FILE *file = fopen(file_path, "rb");
            if (!file) {
                send_404(client_fd);
            } else {
                // 读取文件内容并发送
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);

                char *file_content = malloc(fsize + 1);
                fread(file_content, fsize, 1, file);
                fclose(file);
                file_content[fsize] = 0;

                send_response(client_fd, "HTTP/1.1 200 OK\r\n", "text/html", file_content);
                free(file_content);
            }
        } else {
            send_404(client_fd);
        }
    }

    close(client_fd);
}

void send_response(int client_fd, const char *header, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];

    sprintf(response, "%sContent-Type: %s\r\n\r\n%s", header, content_type, body);
    write(client_fd, response, strlen(response));
}

void send_custom_error(int client_fd, const char *error_page, const char *response_header) {
    char file_path[BUFFER_SIZE];
    snprintf(file_path, BUFFER_SIZE, "%s/%s", DIST_PATH, error_page);

    FILE *file = fopen(file_path, "rb");
    if (file) {
        // 获取文件大小
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // 读取文件内容
        char *file_content = malloc(fsize + 1);
        fread(file_content, fsize, 1, file);
        fclose(file);
        file_content[fsize] = 0;

        // 发送响应
        send_response(client_fd, response_header, "text/html", file_content);
        free(file_content);
    } else {
        // 如果自定义错误页面不存在，则发送简单的错误消息
        send_response(client_fd, response_header, "text/plain", response_header);
    }
}

void send_404(int client_fd) {
    send_custom_error(client_fd, "404.html", "HTTP/1.1 404 Not Found\r\n");
}

void send_400(int client_fd) {
    send_custom_error(client_fd, "400.html", "HTTP/1.1 400 Bad Request\r\n");
}


int main() {
    int server_fd;
    struct sockaddr_in server_addr;
    struct epoll_event ev, events[MAX_EVENTS];
    int epoll_fd, nfds, n, client_fd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    start_server(&server_fd, &server_addr);
    set_non_blocking(server_fd);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                client_addr_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                set_non_blocking(client_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                    perror("epoll_ctl: client_fd");
                    close(client_fd);
                    continue;
                }
            } else {
                handle_client_request(events[n].data.fd);
            }
        }
    }

    close(server_fd);
    return 0;
}