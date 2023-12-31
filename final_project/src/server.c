#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <protocol.h>

#define MAX_EVENTS UINT16_MAX
#define PORT 8080
#define BUFFER_SIZE 1024
#define DIST_PATH "/home/cao/Codes/C_Project/linux_course/experiment3/dist"

Queue* task_queue; // 任务队列
pthread_t *thread_pool; // 线程池
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 用于保护任务队列的互斥锁
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // 用于通知有新的任务的条件变量
RoomNode* room_list; // 房间列表
int room_num = 0; // 房间数量
struct epoll_event events[MAX_EVENTS];

int client_num = 0; 

void *thread_func(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (task_queue->front == NULL) {
            pthread_cond_wait(&cond, &mutex);
        }
        int client_fd = pop(task_queue);
        pthread_mutex_unlock(&mutex);

        // 处理客户端请求
        handle_client_request(client_fd);
    }
}

void start_server(int *server_fd, struct sockaddr_in *server_addr) {
    // 创建任务队列
    task_queue = createQueue();
    all_client_fd = createQueue();
    printf("任务队列初始化完成\n");

    // 创建线程池
    int thread_pool_size = 10; // 线程池的大小
    thread_pool = (pthread_t *)malloc(thread_pool_size * sizeof(pthread_t));
    for (int i = 0; i < thread_pool_size; i++) {
        pthread_create(&thread_pool[i], NULL, thread_func, NULL);
    }
    printf("线程池初始化完成\n");
    
    // 创建房间列表
    room_list = init_roomlist();

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
    uint8_t buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);

    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    buffer[bytes_read] = '\0';

    temp_result result = get_opt(buffer);
    switch (result.opt & CLIENT_OPT_MASK)
    {
        case GET_ROOM_LIST:
            Data data = update_roomlist(get_roominfo(room_list, room_num, MAX_PLAYER_NUM), room_num);
            write(client_fd, data.data, data.len);
            free(data.data);            
            break;
        case CREATE_ROOM:
            char *room_name = (char*)malloc(sizeof(char)*11);
            memcpy(room_name, result.data, sizeof(char)*11);
            RoomNode* newroom = add_room(room_list, room_name);
            pthread_t tid;
            pthread_create(&tid, NULL, control_monsters, (void*)newroom);
            room_num++;
            send_boardcast(update_roomlist(get_roominfo(room_list, room_num, MAX_PLAYER_NUM), room_num));
            free(room_name);
            break;
        case JOIN_ROOM:
            uint16_t room_id;
            memcpy(&room_id, result.data, sizeof(uint16_t));
            char *player_name = (char*)malloc(sizeof(char)*11);
            memcpy(player_name, result.data+sizeof(uint16_t), sizeof(char)*11);
            int flag = add_player(search_room(room_list, room_id), player_name);
            if(flag){
                //加入成功
                Data data = send_join_success();
                write(client_fd, data.data, data.len);
                send_boardcast(update_roomlist(get_roominfo(room_list, room_num, MAX_PLAYER_NUM), room_num));
                send_boardcast(update_roomdata(get_roomdata(search_room(room_list, room_id))));
                //广播更新房间数据
            }else{
                //加入失败
                Data data = send_join_fail();
                write(client_fd, data.data, data.len);
            }

            free(player_name);
            break;
        case MOVE_UP:
            move_player(result.data, MOVE_UP, room_list);
            break;
        case MOVE_DOWN:
            move_player(result.data, MOVE_DOWN, room_list);
            break;
        case MOVE_LEFT:
            move_player(result.data, MOVE_LEFT, room_list);
            break;
        case MOVE_RIGHT:
            move_player(result.data, MOVE_RIGHT, room_list);
            break;
        case ATTACK:
            send_boardcast(send_attack(result, room_list));
            player_attack(result.data, result.opt & (!CLIENT_OPT_MASK),room_list);
            break;
        case QUIT:
            quit_room(result.data, room_list);
            break;
        case REBORN:
            reborn_player(result.data, room_list);
            break;
    }

    // close(client_fd);
}

void send_response(int client_fd, const char *header, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];

    write(client_fd, response, strlen(response));
}

void send_custom_error(int client_fd, const char *error_page, const char *response_header) {
}



int main() {
    srand(time(NULL)); // 初始化随机数生成器
    int server_fd;
    struct sockaddr_in server_addr;
    struct epoll_event ev;
    int epoll_fd, nfds, n, client_fd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    //启动服务器
    start_server(&server_fd, &server_addr);
    //设置非阻塞
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
                // 处理新的连接
                client_addr_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }
                printf("新连接建立: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                set_non_blocking(client_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) { 
                    perror("epoll_ctl: client_fd");
                    close(client_fd);
                    continue;
                }
                push(all_client_fd, client_fd);
                client_num++;
                 
            } else {
                // 处理客户端请求
                pthread_mutex_lock(&mutex);
                push(task_queue, events[n].data.fd);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
            }
        }
    }

    close(server_fd);
    return 0;
}