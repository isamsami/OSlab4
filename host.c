#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MEMORY_SIZE 1024

// Process Control Block (PCB)
typedef struct {
    int id;
    int arrival_time;
    int priority;
    int execution_time;
    int memory_required;
    int resources;
} Process;

// Queue structure
typedef struct {
    Process processes[MAX_PROCESSES];
    int front, rear;
} Queue;

// Initialize queue
void initQueue(Queue *q) {
    q->front = q->rear = -1;
}

// Check if queue is empty
int isEmpty(Queue *q) {
    return q->front == -1;
}

// Enqueue process
void enqueue(Queue *q, Process p) {
    if (q->rear == MAX_PROCESSES - 1) {
        printf("Queue overflow!\n");
        return;
    }
    if (isEmpty(q)) q->front = 0;
    q->processes[++q->rear] = p;
}

// Dequeue process
Process dequeue(Queue *q) {
    if (isEmpty(q)) {
        printf("Error: Attempted to dequeue from an empty queue!\n");
        exit(1);  // Prevent segmentation fault
    }
    Process p = q->processes[q->front];
    if (q->front >= q->rear) initQueue(q);  // Reset queue if last element is dequeued
    else q->front++;
    return p;
}

// Memory allocation
int memory[MEMORY_SIZE] = {0};
int allocateMemory(int size) {
    for (int i = 0; i < MEMORY_SIZE - size; i++) {
        int free = 1;
        for (int j = 0; j < size; j++) {
            if (memory[i + j]) {
                free = 0;
                break;
            }
        }
        if (free) {
            for (int j = 0; j < size; j++) memory[i + j] = 1;
            return i;
        }
    }
    return -1;
}

// Load processes into the correct queues
void loadProcesses(const char *filename, Queue *real_time, Queue *priority_queues[3]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file!\n");
        return;
    }
    Process p;
    while (fscanf(file, "%d,%d,%d,%d,%d,%d", &p.id, &p.arrival_time, &p.priority, &p.execution_time, &p.memory_required, &p.resources) == 6) {
        if (p.priority == 0) {
            enqueue(real_time, p);
        } else if (p.priority >= 1 && p.priority <= 3) {
            enqueue(&priority_queues[p.priority - 1], p);
        } else {
            printf("Invalid priority level for Process %d\n", p.id);
        }
    }
    fclose(file);
}

// Dispatcher
void dispatcher(Queue *real_time, Queue *priority_queues[3]) {
    while (!isEmpty(real_time) || !isEmpty(priority_queues[0]) || !isEmpty(priority_queues[1]) || !isEmpty(priority_queues[2])) {
        if (!isEmpty(real_time)) {
            Process p = dequeue(real_time);
            printf("Executing Real-Time Process %d\n", p.id);
        } else {
            for (int i = 0; i < 3; i++) {
                if (!isEmpty(priority_queues[i])) {
                    Process p = dequeue(priority_queues[i]);
                    printf("Executing User Process %d from Priority %d Queue\n", p.id, i + 1);
                    break;
                }
            }
        }
    }
}

int main() {
    Queue real_time, priority_queues[3];
    initQueue(&real_time);
    for (int i = 0; i < 3; i++) initQueue(&priority_queues[i]);

    // Load processes from file
    loadProcesses("dispatch_list.txt", &real_time, priority_queues);
    
    // Dispatch processes
    dispatcher(&real_time, priority_queues);
    return 0;
}
