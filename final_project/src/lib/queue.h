#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void push(Queue* q, int value) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->data = value;
    temp->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

Node* search_node(Queue* q, int value) {
    Node* p = q->front;
    while (p != NULL) {
        if (p->data == value)
            return p;
        p = p->next;
    }
    return NULL;
}

int del_node(Queue* q, int value) {
    Node* p = q->front;
    Node* prev = NULL;
    while (p != NULL) {
        if (p->data == value) {
            if (prev == NULL) {
                q->front = p->next;
                if (q->front == NULL)
                    q->rear = NULL;
            }
            else {
                prev->next = p->next;
                if (prev->next == NULL)
                    q->rear = prev;
            }
            free(p);
            return 1;
        }
        prev = p;
        p = p->next;
    }
    return 0;
}

int pop(Queue* q) {
    if (q->front == NULL)
        return -1;

    Node* temp = q->front;
    int value = temp->data;

    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);

    return value;
}