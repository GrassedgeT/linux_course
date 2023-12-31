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
    map_win = newpad(MAP_HEIGHT, MAP_WIDTH);
    int map_win_height, map_win_width;
    getmaxyx(stdscr, map_win_height, map_win_width);  // 获取窗口的大小
    score_win = newwin(14, 20, 0, map_win_width-20);
    playerinfo_win = newwin(12, 20, 0, 0);
    gameing = 1;
}

void* handle_update_roomdata(uint8_t* buffer){
    //处理更新房间数据
    RoomData* roomdata = (RoomData*)malloc(sizeof(RoomData));
    uint8_t* temp = buffer;
    temp+=sizeof(uint8_t);
    memcpy(roomdata, temp, sizeof(RoomData));
    temp+=sizeof(RoomData);
    if(roomdata->id != local_room_id){
        return NULL;
    }

    
    wclear(map_win);
    box(map_win, 0, 0);
    Player* myself;
    for(int i=0;i<roomdata->player_num;i++){
        if(strcmp(roomdata->players[i].name, local_player_name) == 0){
            myself = &roomdata->players[i];
        }
    }
    if(myself->status == 0){
        //玩家死亡
        wclear(map_win);
        box(map_win, 0, 0);
        wclear(score_win);
        box(score_win, 0, 0);
        wclear(playerinfo_win);
        box(playerinfo_win, 0, 0);
        //居中弹出
        WINDOW* popwin = newwin(5, 30, (LINES-5)/2, (COLS-30)/2);
        mvwprintw(popwin, 1, 1, "你已死亡，点击空格复活，点击esc退出");
        wrefresh(popwin);
        char ch = wgetch(map_win);
        if (ch == 27)
        {
            //退出
            send_request(quit(local_room_id, local_player_name));
            delwin(popwin);
            exit(0);
        }else if(ch == ' '){
            //复活
            send_request(reborn(local_room_id, local_player_name));
            wclear(popwin);
            wrefresh(popwin);
            delwin(popwin);
        }
        
 
        return NULL;
    }
 
    //刷新所有小怪的位置
    for(int i=0;i<MAX_MONSTER_NUM;i++){
        if(roomdata->monsters[i].status == 1){
            //显示小怪，小怪为红色的随机字符，头顶有当前生命值
            wattron(map_win, COLOR_PAIR(1));
            char c = random_char();
            mvwprintw(map_win, roomdata->monsters[i].y, roomdata->monsters[i].x, "%c",c);
            char* Hp = (char*)malloc(sizeof(char)*4);
            sprintf(Hp, "%u", roomdata->monsters[i].Hp);
            mvwprintw(map_win, roomdata->monsters[i].y-1, roomdata->monsters[i].x, "%s",Hp);
            free(Hp);
            wattroff(map_win, COLOR_PAIR(1));
            //为小怪添加颜色
            
        }
    }
    
    //更新所有玩家的位置
    for(int i=0;i<roomdata->player_num;i++){
        if(roomdata->players[i].status == 1){
            //显示玩家，玩家为绿色的#字符，头顶有当前名称+生命值，玩家形状为以自身坐标为中心的十字形，十字的边长为攻击范围
            wattron(map_win, COLOR_PAIR(2));
            mvwprintw(map_win, roomdata->players[i].y + 1, roomdata->players[i].x, "%s","#");
            mvwprintw(map_win, roomdata->players[i].y, roomdata->players[i].x-1, "%s","###");
            mvwprintw(map_win, roomdata->players[i].y - 1, roomdata->players[i].x, "%s","#");
            char* Hp = (char*)malloc(sizeof(char)*15);
            sprintf(Hp, "%s: HP:%u", roomdata->players[i].name,roomdata->players[i].Hp);
            wattroff(map_win, COLOR_PAIR(2));
            mvwprintw(map_win, roomdata->players[i].y-roomdata->players[i].Atk_range, roomdata->players[i].x - 5, "%s", Hp);
            free(Hp);
        }
    }

    //更新玩家信息
    wclear(playerinfo_win);
    box(playerinfo_win, 0, 0);
    mvwprintw(playerinfo_win, 1, 1, "玩家信息");
    mvwprintw(playerinfo_win, 2, 1, "等级：%u", myself->level);
    mvwprintw(playerinfo_win, 2, 1, "昵称：%s", myself->name);
    mvwprintw(playerinfo_win, 3, 1, "生命值：%u", myself->Hp);
    mvwprintw(playerinfo_win, 4, 1, "攻击力：%u", myself->Atk);
    mvwprintw(playerinfo_win, 5, 1, "攻击范围：%u", myself->Atk_range);
    mvwprintw(playerinfo_win, 6, 1, "经验值：%u/%u", myself->exp, myself->next_level_exp);
    mvwprintw(playerinfo_win, 7, 1, "位置：(%u,%u)", myself->x, myself->y);
    
    //更新分数，分数用经验值代替，从高到低排序
    wclear(score_win);
    box(score_win, 0, 0);
    mvwprintw(score_win, 1, 1, "排行榜");
    //排序
    Player* players = (Player*)malloc(sizeof(Player)*roomdata->player_num);
    memcpy(players, roomdata->players, sizeof(Player)*roomdata->player_num);
    for(int i=0;i<roomdata->player_num;i++){
        for(int j=i+1;j<roomdata->player_num;j++){
            if(players[i].exp < players[j].exp){
                Player temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }
    //显示
    for(int i=0;i<roomdata->player_num;i++){
        mvwprintw(score_win, i+2, 1, "%d-%s: %d",i+1, players[i].name, players[i].exp);
    }
    free(players);

    
    //刷新
    //以当前玩家为中心显示地图
    //更新地图
    int map_win_height, map_win_width;
    getmaxyx(stdscr, map_win_height, map_win_width);  // 获取窗口的大小
    int start_x = myself->x - map_win_width/2;
    int start_y = myself->y - map_win_height/2;
    
    pnoutrefresh(map_win, start_y, start_x, 0, 0, map_win_height-1, map_win_width-1);
    wnoutrefresh(score_win);
    wnoutrefresh(playerinfo_win);

    doupdate();
}



void handle_update_roomlist(uint8_t* buffer, WINDOW* roomlist_win){
    if(gameing){
        return;
    }
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

show_attack(uint8_t buffer){
    int x, y;
    uint8_t direction, attack_range;
    memcpy(&x, buffer+sizeof(uint8_t), sizeof(int));
    memcpy(&y, buffer+sizeof(uint8_t)+sizeof(int), sizeof(int));
    memcpy(&direction, buffer+sizeof(uint8_t)+sizeof(int)+sizeof(int), sizeof(uint8_t));
    memcpy(&attack_range, buffer+sizeof(uint8_t)+sizeof(int)+sizeof(int)+sizeof(uint8_t), sizeof(uint8_t));
    wattron(map_win, COLOR_PAIR(1));char ch[attack_range];
    for(int i=0;i<attack_range;i++){
        ch[i] = '·';
    }
    switch (direction)
    {
        case FIREST:
            for(int i = 0;i<attack_range;i++)
                mvwprintw(map_win, y+i+1, x+1, "%s", ch);
        case SECOND:
            for(int i = 0;i<attack_range;i++)
                mvwprintw(map_win, y+i-1, x-attack_range, "%s", ch);       
        case THIRD:
            for(int i = 0;i<attack_range;i++)
                mvwprintw(map_win, y-i-1, x-attack_range, "%s", ch);
        case FOURTH:
            for(int i = 0;i<attack_range;i++)
                mvwprintw(map_win, y-i+1, x+1, "%s", ch);
    }
    wattroff(map_win, COLOR_PAIR(1));
    int map_win_height, map_win_width;
    getmaxyx(stdscr, map_win_height, map_win_width);  // 获取窗口的大小
    int start_x = x - map_win_width/2;
    int start_y = y - map_win_height/2;
    
    pnoutrefresh(map_win, start_y, start_x, 0, 0, map_win_height-1, map_win_width-1);
    wnoutrefresh(score_win);
    wnoutrefresh(playerinfo_win);

    doupdate();
}

void handle_response(){
    // 处理响应
    char buffer[UINT16_MAX];
    int len = read(sockfd, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        uint8_t opt = (uint8_t)buffer[0];
        switch (opt & SERVER_RSP_MASK)
        {
            case UPDATE_ROOM_LIST:
            handle_update_roomlist(buffer, roomlist_win);
            break;
            case UPDATE_MSG:
            printf("UPDATE_MSG\n");
            break;
            case UPDATE_DATA:
            handle_update_roomdata(buffer);
            break;
            case JOIN_ROOM_SUCCESS:
            handle_join_success();
            break;
            case ATTACK:
            show_attack(buffer);
            break;
        }
    } else if (len == 0) {
        printf("Server closed connection\n");
        close(sockfd);
    } else {
        perror("recv");
    }
}
