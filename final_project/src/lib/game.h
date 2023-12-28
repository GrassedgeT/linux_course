//存放有关有关游戏的数据和操作
#include <stdio.h>
#include <stdlib.h>

#define MAX_HP 150
#define MAX_ATK 50
#define MAX_ATK_RANGE 10
#define MAX_LEVEL 20
#define MAX_PLAYER_NUM 20
typedef struct Room
{
    char* name;
    int player_num;
    player* players;
};

typedef struct player{
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
};

//小怪
typedef struct monster{
    char* name;
    int Hp; //生命值
    int Atk; //攻击力
    int x; //横坐标
    int y; //纵坐标
    int status; //状态0:死亡 1：正常
};

typedef struct msg{
    char* content;
    char* sender;
    msg* next;
};

typedef struct msg_queue{
    int size;
    msg* front;
    msg* rear;
};


//初始化消息队列
msg_queue* init_msg_queue(){
    msg_queue* queue = (msg_queue*)malloc(sizeof(msg_queue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

//消息入队
void push_msg(msg_queue* queue, msg* message){
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
msg* pop_msg(msg_queue* queue){
    if(queue->front == NULL){
        return NULL;
    }
    msg* message = queue->front;
    queue->front = queue->front->next;
    queue->size--;
    return message;
}

