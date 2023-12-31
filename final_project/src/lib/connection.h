#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>    
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdint.h>
#include <protocol.h>
#include <sys/epoll.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1" // 服务端IP地址
#define SERVER_PORT 8080      // 服务端监听端口
#define MAX_EVENTS 10

WINDOW* roomlist_win;
WINDOW* msg_win;
WINDOW* data_win;
WINDOW* stdscr_win;
WINDOW* score_win;
WINDOW* playerinfo_win;
WINDOW* map_win;
int sockfd;
char local_player_name[11];
uint16_t local_room_id;
int gameing = 0;//是否游戏中标识

void listen_response() {
    int epollfd, nfds;
    struct epoll_event ev, events[MAX_EVENTS];

    // 创建一个epoll实例
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 将sockfd添加到epoll实例中
    ev.events = EPOLLIN; // 监听输入事件（即可读事件）
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        perror("epoll_ctl: sockfd");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // 等待事件
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        // 处理事件
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sockfd) {
                // sockfd上有可读事件，读取数据
                handle_response();
            }
        }
    }
    // 关闭epoll实例
    close(epollfd);
}


int connect_to_server(){
    //连接服务器
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

    //启动新线程处理服务器响应
    pthread_t tid;
    pthread_create(&tid, NULL, listen_response, NULL);

    return sockfd;
}


int send_request(Data data){
    // 发送请求
    if (send(sockfd, data.data, data.len, 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    free(data.data);    
    return 0;
}

void handle_join_success(){
    //加入房间成功
    map_win = newwin(MAP_HEIGHT, MAP_WIDTH, 0, 0);
    int map_win_height, map_win_width;
    getmaxyx(map_win, map_win_height, map_win_width);  // 获取窗口的大小
    score_win = newwin(14, 10, 0, MAP_WIDTH-10);
    playerinfo_win = newwin(14, 15, 0, 0);
}

void handle_update_roomlist(uint8_t* buffer, WINDOW* roomlist_win){
    uint8_t* temp = buffer;
    temp+=sizeof(uint8_t);
    uint8_t room_num;
    memcpy(&room_num, temp, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    if(room_num == NULL_DATA){
        //房间列表为空
        wclear(roomlist_win);
        box(roomlist_win, 0, 0);

        int height, width;
        getmaxyx(roomlist_win, height, width);  // 获取窗口的大小

        const char* message1 = "当前暂无房间";
        const char* message2 = "点击\" c \"键创建房间，点击esc退出";
        mvwprintw(roomlist_win, height / 2, (width - strlen(message1)) / 2, "%s", message1);
        mvwprintw(roomlist_win, height / 2 + 2, (width - strlen(message2)) / 2 + 4, "%s", message2);

        wrefresh(roomlist_win);
        return;
    }
    RoomInfo* roomlist = (RoomInfo*)malloc(sizeof(RoomInfo)*room_num);
    memcpy(roomlist, temp, sizeof(RoomInfo)*room_num);
    const char* title = "房间列表";
    const char* message1 = "点击\" c \"键创建房间，点击esc退出";
    const char* message2 = "点击\" Enter \"键并输入房间名称后的四位id加入房间";
    int height, width;
    getmaxyx(roomlist_win, height, width);  // 获取窗口的大小
    wclear(roomlist_win);
    box(roomlist_win, 0, 0);
    mvwprintw(roomlist_win, height / 4, (width - strlen(title)) / 2 + 2, "%s", title);
    mvwprintw(roomlist_win, height / 4 + room_num + 4, (width - strlen(message1)) / 2 + 4, "%s", message1);
    mvwprintw(roomlist_win, height / 4 + room_num + 6, (width - strlen(message2)) / 2 + 8, "%s", message2);
    for(int i=0;i<room_num;i++){
        char* roominfo = (char*)malloc(sizeof(char)*30);
        char* roomname = (char*)malloc(sizeof(char)*11);
        sprintf(roomname, "%s_%d", roomlist[i].name, roomlist[i].id);
        sprintf(roominfo, "%-14s  %d/%d", roomname, roomlist[i].player_num, roomlist[i].max_player_num);
        mvwprintw(roomlist_win, height / 4 + i + 2, (width - strlen(roominfo)) / 2, "%s", roominfo);
        free(roominfo);
        free(roomname);
    }
    free(roomlist);
    wrefresh(roomlist_win);
    
}

void handle_response(){
    // 处理响应
    char buffer[1024];
    int len = read(sockfd, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        uint8_t opt = (uint8_t)buffer[0];
        switch (opt)
        {
            case UPDATE_ROOM_LIST:
            // printf("UPDATE_ROOM_LIST\n");
            handle_update_roomlist(buffer, roomlist_win);
            break;
            case UPDATE_MSG:
            printf("UPDATE_MSG\n");
            break;
            case UPDATE_DATA:
            printf("UPDATE_DATA\n");
            break;
        }
    } else if (len == 0) {
        printf("Server closed connection\n");
        close(sockfd);
    } else {
        perror("recv");
    }
}
