//存放有关有关游戏的数据和操作
#include <stdio.h>
#include <stdlib.h>

#define MAX_HP 150
#define MAX_ATK 50
#define MAX_ATK_RANGE 10
#define MAX_LEVEL 20
#define MAX_PLAYER_NUM 20

typedef struct Player{
    int id;
    char* name;
    int Hp; //生命值
    int Atk; //攻击力
    int Atk_range; //攻击范围
    int level; //等级
    int score; //分数
    int exp; //经验值
    int next_level_exp; //升级所需经验值
    int x; //横坐标
    int y; //纵坐标
    int status; //状态0:死亡 1：正常 2：无敌
}Player;

typedef struct Room
{
    char* name;
    int player_num;
    struct Player* players;
}Room;



//小怪
typedef struct Monster{
    char* name;
    int Hp; //生命值
    int Atk; //攻击力
    int x; //横坐标
    int y; //纵坐标
    int status; //状态0:死亡 1：正常
}Monster;

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

