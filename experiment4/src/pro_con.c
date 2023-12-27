#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_ITEMS 10

struct item {
    int value;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main() {
    int shmid;//共享内存id
    int semid;//信号量id
    struct item *shared_memory;
    union semun sem_union;
    struct sembuf sem_b;
    pid_t pid;

    // 创建共享内存
    shmid = shmget((key_t)1234, sizeof(struct item) * MAX_ITEMS, 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    // 将共享内存连接到当前进程的地址空间
    shared_memory = (struct item *)shmat(shmid, 0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    // 创建信号量
    semid = semget((key_t)5678, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        fprintf(stderr, "semget failed\n");
        exit(EXIT_FAILURE);
    }

    // 初始化信号量
    sem_union.val = 1;
    if (semctl(semid, 0, SETVAL, sem_union) == -1) {
        fprintf(stderr, "semctl failed\n");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // 子进程，作为消费者
        for (int i = 0; i < MAX_ITEMS; i++) {
            // 等待信号量
            sem_b.sem_num = 0;
            sem_b.sem_op = -1;
            sem_b.sem_flg = SEM_UNDO;
            semop(semid, &sem_b, 1);

            // 消费数据
            printf("Consumed: %d\n", shared_memory[i].value);

            // 释放信号量
            sem_b.sem_op = 1;
            semop(semid, &sem_b, 1);
        }
    } else { // 父进程，作为生产者
        for (int i = 0; i < MAX_ITEMS; i++) {
            // 等待信号量
            sem_b.sem_num = 0;
            sem_b.sem_op = -1;
            sem_b.sem_flg = SEM_UNDO;
            semop(semid, &sem_b, 1);

            // 生产数据
            shared_memory[i].value = i;
            sleep(1);

            // 释放信号量
            sem_b.sem_op = 1;
            semop(semid, &sem_b, 1);
        }
    }

    // 断开共享内存连接
    shmdt(shared_memory);

    // 删除共享内存
    shmctl(shmid, IPC_RMID, 0);

    // 删除信号量
    semctl(semid, 0, IPC_RMID, sem_union);

    exit(EXIT_SUCCESS);
}