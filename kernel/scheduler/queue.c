#include "queue.h"

// ============================================================================
// Helper Functions for Node Management
// ============================================================================

static Node node_pool[MAX_QUEUE_NODES];
static Queue free_list;

// Function to allocate a node from the free list
static Node* allocate_node(void)
{
    if (free_list.front == NULL) return NULL; // Out of nodes!

    Node *allocated = free_list.front;
    free_list.front = allocated->next;
    
    if (free_list.front == NULL) free_list.back = NULL;
    
    allocated->next = NULL;
    return allocated;
}

// Function to free a node back to the free list
static void free_node(Node *node)
{
    node->next = NULL;
    node->pid = INVALID_PID;

    if (free_list.front == NULL)
    {
        free_list.front = node;
    }
    else
    {
        free_list.back->next = node;
    }
    free_list.back = node;
}

// ============================================================================
// Queue Functions
// ============================================================================

// Function to initialize the system queue
void system_queue_init(void)
{
    init_queue(&free_list);

    // Link all pre-allocated nodes together into the free list
    for (int i = 0; i < MAX_QUEUE_NODES; i++)
    {
        node_pool[i].pid = INVALID_PID;
        node_pool[i].next = NULL;

        if (free_list.front == NULL)
        {
            free_list.front = &node_pool[i];
        }
        else
        {
            free_list.back->next = &node_pool[i];
        }
        free_list.back = &node_pool[i];
    }
}

void init_queue(Queue *q)
{
    q->front = NULL;
    q->back = NULL;
}

int is_empty(Queue *q)
{
    return (q->front == NULL);
}

unsigned int enqueue(Queue *q, unsigned int pid)
{
    Node *new_node = allocate_node();
    if (new_node == NULL)
    {
        return 0; // Failed
    }

    new_node->pid = pid;

    if (is_empty(q))
    {
        q->front = new_node;
    }
    else
    {
        q->back->next = new_node;
    }
    q->back = new_node;

    return 1; // Success
}

unsigned int dequeue(Queue *q)
{
    if (is_empty(q))
    {
        return INVALID_PID;
    }

    Node *temp = q->front;
    unsigned int pid = temp->pid;

    q->front = temp->next;
    if (q->front == NULL)
    {
        q->back = NULL;
    }

    free_node(temp); // Recycle the node memory
    return pid;
}
