#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_TEXT 1024

struct my_msg_st {
    long int my_msg_type;
    char some_text[MAX_TEXT];
};

int main() {
    int running = 1;
    int msgid;
    struct my_msg_st some_data;
    pid_t pid;

    // 创建消息队列 KEY: 1234   
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed\n");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // 子进程
        some_data.my_msg_type = 1;
        strcpy(some_data.some_text, "Hello, world, I'am a child process. And I'm writing to the message queue. This is a long message.");
        if (msgsnd(msgid, (void *)&some_data, MAX_TEXT, 0) == -1) {
            fprintf(stderr, "msgsnd failed\n");
            exit(EXIT_FAILURE);
        }
    } else { // 父进程
        if (msgrcv(msgid, (void *)&some_data, MAX_TEXT, 0, 0) == -1) {
            fprintf(stderr, "msgrcv failed\n");
            exit(EXIT_FAILURE);
        }
        printf("child wrote: %s\n", some_data.some_text);
        if (msgctl(msgid, IPC_RMID, 0) == -1) {
            fprintf(stderr, "msgctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}