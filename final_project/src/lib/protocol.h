//存放应用层二进制协议相关的定义
#include <stdint.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <game.h>


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
#define UPDATE_DATA 1 //房间信息: 0000 0001
#define UPDATE_MSG 2 //消息: 0000 0010
#define UPDATE_ROOM_LIST 3 //房间列表: 0000 0011

#define SERVER_RSP_MASK 0x0f //服务器响应掩码: 0000 1111

uint8_t* get_roomlist(){
    uint8_t opt = GET_ROOM_LIST;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, "\0", sizeof(char));
    temp+=sizeof(char);
    return buf;
}

uint8_t* create_room(char* room_name){
    uint8_t opt = CREATE_ROOM;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*3+strlen(room_name));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);;
    memcpy(temp, room_name, strlen(room_name)*sizeof(char));
    temp+=strlen(room_name)*sizeof(char);
    return buf;
}

uint8_t* join_room(uint16_t room_id, char* player_name){
    uint8_t opt = JOIN_ROOM;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*3+strlen(player_name));
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, &room_id, sizeof(uint16_t));
    temp+=sizeof(uint16_t);
    memcpy(temp, player_name, strlen(player_name)*sizeof(char));
    temp+=strlen(player_name)*sizeof(char);
    return buf;
}

uint8_t* update_roomlist(RoomInfo* roomlist, uint8_t room_num){
    uint8_t opt = UPDATE_ROOM_LIST;
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*2+sizeof(RoomInfo)*room_num);
    uint8_t* temp = buf;
    memcpy(temp, &opt, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, &room_num, sizeof(uint8_t));
    temp+=sizeof(uint8_t);
    memcpy(temp, roomlist, sizeof(RoomInfo)*room_num);
    temp+=sizeof(RoomInfo)*room_num;
    memcpy(temp, "\0", sizeof(char));
    temp+=sizeof(char);
    return buf;
}

typedef struct temp_result
{
    uint8_t opt;
    uint8_t* data;
}temp_result;


temp_result get_opt(uint8_t* buf){
    struct temp_result result;
    memcpy(&result.opt, buf, sizeof(uint8_t));
    result.opt = ntohl(result.opt);
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