//存放有关有关游戏的数据和操作
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils.h>
#define MAX_HP 150
#define MAX_ATK 50
#define MAX_ATK_RANGE 10
#define MAX_LEVEL 20
#define MAX_PLAYER_NUM 10
#define MAX_MONSTER_NUM 20

//小怪
typedef struct Monster{
    char* name;
    uint8_t Hp; //生命值
    uint8_t Atk; //攻击力
    int x; //横坐标
    int y; //纵坐标
    u_int8_t status; //状态0:死亡 1：正常
}Monster;
typedef struct Player{
    uint16_t id;
    char* name;
    uint8_t Hp; //生命值 初始值100
    uint8_t Atk; //攻击力 初始值10
    uint8_t Atk_range; //攻击范围 初始值2
    uint8_t level; //等级 初始值1
    uint8_t score; //分数 初始值0
    uint8_t exp; //经验值 初始值0
    uint8_t next_level_exp; //升级所需经验值 初始值50 逐级递增50
    int x; //横坐标
    int y; //纵坐标
    uint8_t status; //状态0: 死亡 1：正常 2：无敌
}Player;

typedef struct RoomNode
{
    uint16_t id;
    char* name;
    uint8_t player_num;
    struct Player* players;
    struct Monster* monsters;
    struct RoomNode* next;
}RoomNode;

typedef struct RoomInfo{
    uint16_t id;
    char* name;
    uint8_t player_num;
    uint8_t max_player_num;
    struct RoomInfo* next;
}RoomInfo;

//初始化房间列表
RoomNode* init_roomlist(){
    //返回一个空的头节点
    RoomNode* head = (RoomNode*)malloc(sizeof(RoomNode));
    head->id = 0;
    head->name = NULL;
    head->player_num = 0;
    head->players = NULL;
    head->monsters = NULL;
    head->next = NULL;
    return head;
}

RoomInfo* get_roominfo(RoomNode* roomlist){
    RoomInfo* roominfo = NULL;
    RoomNode* p = roomlist;
    RoomInfo* temp = roominfo;
    while(p->next != NULL){
        temp = (RoomInfo*)malloc(sizeof(RoomInfo));
        temp->id = p->next->id;
        temp->name = p->next->name;
        temp->player_num = p->next->player_num;
        temp->max_player_num = MAX_PLAYER_NUM;
        temp->next = NULL;
        p = p->next;
    }
    return roominfo;
}

//添加新房间
void add_room(RoomNode* head, char* name){
    RoomNode* p = head;
    while(p->next != NULL){
        p = p->next;
    }
    RoomNode* new_room = (RoomNode*)malloc(sizeof(RoomNode));
    new_room->id = (uint16_t)random_int(1000, 9999);
    new_room->name = name;
    new_room->player_num = 0;
    new_room->players = (Player*)malloc(sizeof(Player)*MAX_PLAYER_NUM);
    new_room->monsters = (Monster*)malloc(sizeof(Monster)*MAX_MONSTER_NUM);
    new_room->next = NULL;
    p->next = new_room;
}

void del_room(RoomNode* head, uint16_t id){
    RoomNode* p = head;
    while(p->next != NULL){
        if(p->next->id == id){
            RoomNode* q = p->next;
            p->next = p->next->next;
            free(q);
            return;
        }
        p = p->next;
    }
}

RoomNode* search_room(RoomNode* head, uint16_t id){
    RoomNode* p = head;
    while(p->next != NULL){
        if(p->next->id == id){
            return p->next;
        }
        p = p->next;
    }
    return NULL;
}

void add_player(RoomNode* room, char* name){
    Player* p = room->players;
    while(p->name != NULL){
        p++;
    }
    p->name = name;
    p->Hp = 100;
    p->Atk = 10;
    p->Atk_range = 2;
    p->level = 1;
    p->score = 0;
    p->exp = 0;
    p->next_level_exp = 50;
    p->x = 0;
    p->y = 0;
    p->status = 0;
    room->player_num++;
}

void del_player(RoomNode* room, uint16_t id){
    Player* p = room->players;
    while(p->name != NULL){
        if(p->id == id){
            p->name = NULL;
            room->player_num--;
            return;
        }
        p++;
    }    
}

Player* search_player(RoomNode* room, uint16_t id){
    Player* p = room->players;
    while(p->name != NULL){
        if(p->id == id){
            return p;
        }
        p++;
    }
    return NULL;
}




typedef struct Msg{
    char* content;
    char* sender;
    struct Msg* next;
}Msg;

typedef struct Msg_queue{
    int size;
    Msg* front;
    Msg* rear;
}Msg_queue;


//初始化消息队列
Msg_queue* init_msg_queue(){
    Msg_queue* queue = (Msg_queue*)malloc(sizeof(Msg_queue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

//消息入队
void push_msg(Msg_queue* queue, Msg* message){
    if(queue->front == NULL){
        queue->front = message;
        queue->rear = message;
    }else{
        queue->rear->next = message;
        queue->rear = message;
    }
    queue->size++;
}

//消息出队
Msg* pop_msg(Msg_queue* queue){
    if(queue->front == NULL){
        return NULL;
    }
    Msg* message = queue->front;
    queue->front = queue->front->next;
    queue->size--;
    return message;
}


