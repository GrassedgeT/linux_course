//存放有关有关游戏的数据和操作
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils.h>
#include <pthread.h>

#define MAX_HP 150
#define MAX_ATK 50
#define MAX_ATK_RANGE 10
#define MAX_LEVEL 20
#define MAX_PLAYER_NUM 10
#define MAX_MONSTER_NUM 10
#define MAP_HEIGHT 100
#define MAP_WIDTH 250
//小怪
typedef struct Monster{
    uint16_t id;    
    uint8_t Hp; //生命值
    uint8_t Atk; //攻击力
    int x; //横坐标
    int y; //纵坐标
    u_int8_t status; //状态0:死亡 1：正常

}Monster;
typedef struct Player{
    uint16_t id;
    char name[11];
    uint8_t Hp; //生命值 初始值100
    uint8_t Atk; //攻击力 初始值10
    uint8_t Atk_range; //攻击范围 初始值2
    uint8_t level; //等级 初始值1
    uint8_t score; //分数 初始值0
    uint8_t exp; //经验值 初始值0
    uint8_t next_level_exp; //升级所需经验值 初始值10 逐级递增10击杀小怪得1击杀玩家得2
    int x; //横坐标
    int y; //纵坐标
    uint8_t status; //状态0: 死亡 1：正常 2：无敌
    struct Player* next;
}Player;

typedef struct RoomNode
{
    uint16_t id;
    char name[11];
    uint8_t player_num;
    Player* players;
    Monster monsters[MAX_MONSTER_NUM];
    pthread_mutex_t player_mutex;
    pthread_mutex_t monster_mutex;
    struct RoomNode* next;
}RoomNode;

typedef struct RoomData{
    uint16_t id;
    char name[11];
    uint8_t player_num;
    uint8_t max_player_num;
    Player players[MAX_PLAYER_NUM];
    Monster monsters[MAX_MONSTER_NUM];
}RoomData;

typedef struct RoomInfo{
    uint16_t id;
    char name[11];
    uint8_t player_num;
    uint8_t max_player_num;
    struct RoomInfo* next;
}RoomInfo;



Player* init_playerlist(){
    Player* head = (Player*)malloc(sizeof(Player));
    head->id = 0;
    strcpy(head->name, "");
    head->Hp = 0;
    head->Atk = 0;
    head->Atk_range = 0;
    head->level = 0;
    head->score = 0;
    head->exp = 0;
    head->next_level_exp = 0;
    head->x = 0;
    head->y = 0;
    head->status = 0;
    head->next = NULL;
    return head;
}

//初始化房间列表
RoomNode* init_roomlist(){
    //返回一个空的头节点
    RoomNode* head = (RoomNode*)malloc(sizeof(RoomNode));
    head->id = 0;
    strcpy(head->name, "");
    head->player_num = 0;
    head->players = NULL;
    head->next = NULL;
    return head;
}

RoomData* get_roomdata(RoomNode* roomNode){
    RoomData* roomdata = (RoomData*)malloc(sizeof(RoomData));
    pthread_mutex_lock(&roomNode->player_mutex);
    pthread_mutex_lock(&roomNode->monster_mutex);
    roomdata->id = roomNode->id;
    strcpy(roomdata->name, roomNode->name);
    roomdata->player_num = roomNode->player_num;
    memcpy(roomdata->monsters, roomNode->monsters, sizeof(Monster)*MAX_MONSTER_NUM);
    int i = 0;
    Player* p = roomNode->players;
    while(p->next != NULL){
        roomdata->players[i].id = p->next->id;
        strcpy(roomdata->players[i].name, p->next->name);
        roomdata->players[i].Hp = p->next->Hp;
        roomdata->players[i].Atk = p->next->Atk;
        roomdata->players[i].Atk_range = p->next->Atk_range;
        roomdata->players[i].level = p->next->level;
        roomdata->players[i].score = p->next->score;
        roomdata->players[i].exp = p->next->exp;
        roomdata->players[i].next_level_exp = p->next->next_level_exp;
        roomdata->players[i].x = p->next->x;
        roomdata->players[i].y = p->next->y;
        roomdata->players[i].status = p->next->status;
        p = p->next;
        i++;
    }
    pthread_mutex_unlock(&roomNode->player_mutex);
    pthread_mutex_unlock(&roomNode->monster_mutex);
    return roomdata;
}

RoomInfo* get_roominfo(RoomNode* roomlist, int room_num, int max_player_num){
    if(room_num == 0){
        return NULL;
    }
    int i = 0;
    RoomInfo* roominfo = malloc(room_num * sizeof(RoomInfo));  // 动态分配内存
    if (roominfo == NULL)
    {
        return NULL;
    }
    
    RoomNode* p = roomlist;
    
    while(p->next != NULL){
        pthread_mutex_lock(&p->next->player_mutex);
        pthread_mutex_lock(&p->next->monster_mutex);
        roominfo[i].id = p->next->id;
        strcpy(roominfo[i].name, p->next->name);
        roominfo[i].player_num = p->next->player_num;
        roominfo[i].max_player_num = max_player_num;
        i++;
        pthread_mutex_unlock(&p->next->player_mutex);
        pthread_mutex_unlock(&p->next->monster_mutex);
        p = p->next;
    }
    return roominfo;
}

//小怪死亡后重生
void* reborn_monster(Monster* monster){
    monster->Hp = random_int(50, MAX_HP);
    monster->Atk = random_int(10, MAX_ATK);
    monster->x = random_int(0, MAP_WIDTH);
    monster->y = random_int(0, MAP_HEIGHT);
    monster->status = 1;
}

//添加新房间
RoomNode* add_room(RoomNode* head, char* name){
    RoomNode* p = head;
    while(p->next != NULL){
        p = p->next;
    }
    RoomNode* new_room = (RoomNode*)malloc(sizeof(RoomNode));
    new_room->id = (uint16_t)random_int(1000, 9999);
    strcpy(new_room->name, name);
    new_room->player_num = 0;
    new_room->players = init_playerlist();
    for(int i=0;i<MAX_MONSTER_NUM;i++){
        new_room->monsters[i].id = random_int(1000, 9999);
        new_room->monsters[i].Hp = random_int(50, MAX_HP);
        new_room->monsters[i].Atk = random_int(10, MAX_ATK);
        new_room->monsters[i].x = random_int(0, MAP_WIDTH);
        new_room->monsters[i].y = random_int(0, MAP_HEIGHT);
        new_room->monsters[i].status = 1;
    }
    pthread_mutex_init(&new_room->player_mutex, NULL);
    pthread_mutex_init(&new_room->monster_mutex, NULL);
    
    new_room->next = NULL;
    p->next = new_room;
    return new_room;
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



int add_player(RoomNode* room, char* name){
    if(room == NULL){
        return 0;
    }
    pthread_mutex_lock(&room->player_mutex);
    if(room->player_num == MAX_PLAYER_NUM){
        pthread_mutex_unlock(&room->player_mutex);
        return 0;
    }
    Player* p = (Player*)malloc(sizeof(Player));
    Player* head = room->players;
    while(head->next != NULL){
        head = head->next;
    }
    strcpy(p->name, name);
    p->Hp = 100;
    p->Atk = 10;
    p->Atk_range = 2;
    p->level = 1;
    p->score = 0;
    p->exp = 0;
    p->next_level_exp = 50;
    p->x = random_int(0, MAP_WIDTH);
    p->y = random_int(0, MAP_HEIGHT);
    p->status = 1;
    room->player_num++;
    head->next = p;
    pthread_mutex_unlock(&room->player_mutex);
    return 1;
}

void del_player(RoomNode* room, uint16_t id){
    Player* p = room->players;
    pthread_mutex_lock(&room->player_mutex);
    while(p->next != NULL){
        if(p->next->id == id){
            
            pthread_mutex_unlock(&room->player_mutex);
            return;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&room->player_mutex);
}

Player* search_player(RoomNode* room, uint16_t id){
    Player* p = room->players;
    while(p->next != NULL){
        if(p->next->id == id){
            return p->next;
        }
        p = p->next;
    }
    return NULL;
}





typedef struct Msg{
    char content[100];
    char sender[11];
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


