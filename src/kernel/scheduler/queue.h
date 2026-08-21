#ifndef QUEUE_H
#define QUEUE_H

#define NULL ((void*)0)
#define INVALID_PID 0xFFFFFFFF

// ============================================================================
// Queue Functions
// ============================================================================

#define MAX_QUEUE_NODES 8

typedef struct Node
{
    unsigned int pid;
    struct Node *next;
} Node;

typedef struct
{
    Node *front;    // Head
    Node *back;     // Tail
} Queue;

extern Queue ready_queue;

void system_queue_init(void);
void init_queue(Queue *q);

unsigned int enqueue(Queue *q, unsigned int pid);
unsigned int dequeue(Queue *q);
int is_empty(Queue *q);

#endif // QUEUE_H