//存放应用层二进制协议相关的定义
#include <stdint.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <game.h>
#include <math.h>
#include <queue.h>
#define NULL_DATA UINT8_MAX //空数据

//客户端操作类型定义
#define CREATE_ROOM 1 //创建房间: 0000 0001
#define JOIN_ROOM 2 //加入房间: 0000 0010
#define EXIT 3 //退出: 0000 0011
#define MOVE_UP 4 //向上移动: 0000 0100
#define MOVE_DOWN 5 //向下移动: 0000 0101
#define MOVE_LEFT 6 //向左移动: 0000 0110
#define MOVE_RIGHT 7 //向右移动: 0000 0111
#define ATTACK 8 //攻击: 0000 1000
#define SEND_MSG 9 //发送消息: 0000 1001
#define GET_MSG 10  //获取消息: 0000 1010
#define REBORN 11   //复活: 0000 1011
#define GET_ROOM_LIST 12 //获取房间列表: 0000 1100
#define CLIENT_OPT_MASK 0x0f //客户端操作掩码: 0000 1111

#define FIREST 0 //第一象限方向 0000 0000
#define SECOND 16 //第二象限方向 0001 0000
#define THIRD 32 //第三象限方向 0010 0000
#define FOURTH 48 //第四象限方向 0011 0000

#define CARRY_ROOM 64 //携带目标房间号 0100 0000
#define CARRY_PLAYER 128 //携带目标玩家 1000 0000

//服务器响应类型定义
#define UPDATE_DATA 1 //更新房间信息: 0000 0001
#define UPDATE_MSG 2 //更新消息: 0000 0010
#define UPDATE_ROOM_LIST 3 //更新房间列表: 0000 0011
#define JOIN_ROOM_SUCCESS 4 //服务器响应: 0000 0100
#define JOIN_ROOM_FAIL 5 //服务器响应: 0000 0101

#define SERVER_RSP_MASK 0x0f //服务器响应掩码: 0000 1111

Queue* all_client_fd; // 所有客户端的fd

typedef struct{
    int len; //数据长度
    uint8_t* data; //数据
}Data;

Data get_roomlist(){
    uint8_t opt = GET_ROOM_LIST;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    Data data = {sizeof(uint8_t), buf};
    return data;
}

Data create_room(char* room_name){
    uint8_t opt = CREATE_ROOM;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*2+strlen(room_name));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);;
    memcpy(temp, room_name, strlen(room_name)*sizeof(char));
    temp+=strlen(room_name)*sizeof(char);
    Data data = {sizeof(uint8_t)*2+strlen(room_name), buf};
    return data;
}

Data join_room(uint16_t room_id, char* player_name){
    uint8_t opt = JOIN_ROOM;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)+sizeof(uint16_t)+11*sizeof(char));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, &room_id, sizeof(uint16_t));
    temp+=sizeof(uint16_t);
    char* name = (char*)malloc(sizeof(char)*11);
    strcpy(name, player_name);
    memcpy(temp, name, sizeof(char)*11);
    temp+=strlen(player_name)*sizeof(char);
    Data data = {sizeof(uint8_t)+sizeof(uint16_t)+11*sizeof(char), buf};
    return data;
}

int send_boardcast(Data data){
    int client_fd;
    Node* p = all_client_fd->front;
    while(p != NULL){
        client_fd = p->data;
        write(client_fd, data.data, data.len);
        p = p->next;
    }
}

Data update_roomlist(RoomInfo* roominfo, uint8_t room_num){
    uint8_t opt = UPDATE_ROOM_LIST;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*2+sizeof(RoomInfo)*room_num);
    int len = sizeof(uint8_t)*2+sizeof(RoomInfo)*room_num;
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    if(room_num != 0){
        memcpy(temp, &room_num, sizeof(uint8_t));
        temp+=sizeof(uint8_t);    
    }else{
        uint8_t null_data = NULL_DATA;
        memcpy(temp, &null_data, sizeof(uint8_t));
        temp+=sizeof(uint8_t);
    }
    if(room_num != 0){
        memcpy(temp, roominfo, sizeof(RoomInfo)*room_num);
        temp+=sizeof(RoomInfo)*room_num;       
    }
    Data data = {len, buf};
    return data;
}

Data update_roomdata(RoomData* roomdata){
    uint8_t opt = UPDATE_DATA;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)+sizeof(RoomData));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, roomdata, sizeof(RoomData));
    temp+=sizeof(RoomData);
    Data data = {sizeof(uint8_t)+sizeof(RoomData), buf};
    return data;
}

Data send_join_fail(){
    uint8_t opt = JOIN_ROOM_FAIL;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    Data data = {sizeof(uint8_t), buf};
    return data;
}

Data send_join_success(){
    uint8_t opt = JOIN_ROOM_SUCCESS;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    Data data = {sizeof(uint8_t), buf};
    return data;
}


typedef struct temp_result
{
    uint8_t opt;
    uint8_t* data;
}temp_result;


temp_result get_opt(uint8_t* buf){
    struct temp_result result;
    memcpy(&result.opt, buf, sizeof(uint8_t));
    result.data = buf+sizeof(uint8_t);
    return result;
}

RoomInfo* get_room_list(uint8_t* data){
    uint8_t room_num;
    memcpy(&room_num, data, sizeof(uint8_t));
    RoomInfo* roomlist = (RoomInfo*)malloc(sizeof(RoomInfo)*room_num);
    memcpy(roomlist, data+sizeof(uint8_t), sizeof(RoomInfo)*room_num);
    return roomlist;
}

void* control_monsters(RoomNode* roomNode){
    while(1){
        sleep(1);
        int alive_player_num = 0;
        pthread_mutex_lock(&roomNode->player_mutex);
        Player* temp = roomNode->players;
        while(temp->next != NULL){
            if(temp->next->status == 1){
                alive_player_num++;
            }
            temp = temp->next;
        }   
        if(alive_player_num == 0){
            pthread_mutex_unlock(&roomNode->player_mutex);
            continue;
        }

        Player* p = roomNode->players;
        pthread_mutex_lock(&roomNode->monster_mutex);
        for(int i=0;i<20;i++){
            if(roomNode->monsters[i].status == 0){
                //复活怪物
                reborn_monster(&roomNode->monsters[i]);
                continue;
            }
            int min_distance = INT32_MAX;
            Player* nearest_player = NULL;
            Player* p = roomNode->players;
            while(p->next != NULL){
                if(p->next->status == 0){
                    p = p->next;
                    continue;
                }
                //距离不按照直线距离计算，而是按照水平距离+垂直距离计算
                int distance = abs(roomNode->monsters[i].x - p->next->x) + abs(roomNode->monsters[i].y - p->next->y);
                if(distance < min_distance){
                    min_distance = distance;
                    nearest_player = p->next;
                }
                p = p->next;
            }
            if(nearest_player != NULL){
                // 让怪物向最近的玩家移动。
                if(nearest_player->x > roomNode->monsters[i].x){
                    roomNode->monsters[i].x++;
                }else if(nearest_player->x < roomNode->monsters[i].x){
                    roomNode->monsters[i].x--;
                }
                if(nearest_player->y > roomNode->monsters[i].y){
                    roomNode->monsters[i].y++;
                }else if(nearest_player->y < roomNode->monsters[i].y){
                    roomNode->monsters[i].y--;
                }
                // 如果怪物与玩家的坐标重合，怪物攻击玩家且怪物死亡
                if(nearest_player->x == roomNode->monsters[i].x && nearest_player->y == roomNode->monsters[i].y){
                    if(nearest_player->Hp > roomNode->monsters[i].Atk){
                        nearest_player->Hp -= roomNode->monsters[i].Atk;
                    }else{
                        nearest_player->Hp = 0;
                        nearest_player->status = 0;
                    }
                    roomNode->monsters[i].status = 0;
                }   
            }
        }
        pthread_mutex_unlock(&roomNode->monster_mutex);
        pthread_mutex_unlock(&roomNode->player_mutex);
        send_boardcast(update_roomdata(get_roomdata(roomNode)));
    }
}