#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TOTAL_MEMORY 256
#define TIME_QUANTUM 2
#define MAX_PROCESSES 100
#define MAX_LINE_LENGTH 100

typedef struct Process {
    int pid;
    int arrival_time;
    int priority;
    int exec_time;
    int mem_req;
    int remaining_time;
    struct Process* next;
} Process;

typedef struct {
    int start;
    int size;
} MemoryBlock;

MemoryBlock free_memory[TOTAL_MEMORY];
int free_memory_count = 1;
Process* real_time_queue = NULL;
Process* priority_queues[3] = {NULL, NULL, NULL};

void insert_process(Process** queue, Process* process) {
    process->next = *queue;
    *queue = process;
}

Process* remove_process(Process** queue) {
    if (*queue == NULL) return NULL;
    Process* temp = *queue;
    *queue = (*queue)->next;
    return temp;
}

bool allocate_memory(int pid, int size) {
    for (int i = 0; i < free_memory_count; i++) {
        if (free_memory[i].size >= size) {
            free_memory[i].start += size;
            free_memory[i].size -= size;
            return true;
        }
    }
    return false;
}

void release_memory(int pid, int size) {
    free_memory[free_memory_count].start = 0;
    free_memory[free_memory_count].size = size;
    free_memory_count++;
}

void submit_job(Process* process) {
    if (!allocate_memory(process->pid, process->mem_req)) {
        printf("Process %d rejected due to insufficient memory.\n", process->pid);
        return;
    }
    if (process->priority == 0)
        insert_process(&real_time_queue, process);
    else
        insert_process(&priority_queues[0], process);
}

Process* schedule() {
    if (real_time_queue)
        return remove_process(&real_time_queue);
    for (int i = 0; i < 3; i++) {
        if (priority_queues[i])
            return remove_process(&priority_queues[i]);
    }
    return NULL;
}

void run() {
    Process* process = schedule();
    if (!process) {
        printf("No process to execute.\n");
        return;
    }
    int time_quantum = (process->priority > 0) ? TIME_QUANTUM : process->exec_time;
    process->remaining_time -= time_quantum;
    printf("Executing process %d for %d units.\n", process->pid, time_quantum);
    if (process->remaining_time > 0 && process->priority > 0) {
        int new_priority = (process->priority + 1 < 3) ? process->priority + 1 : 2;
        insert_process(&priority_queues[new_priority], process);
    } else {
        release_memory(process->pid, process->mem_req);
        printf("Process %d completed and memory released.\n", process->pid);
    }
}

void load_dispatch_list(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open dispatch list");
        exit(1);
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        Process* p = (Process*)malloc(sizeof(Process));
        if (sscanf(line, "%d %d %d %d %d", &p->pid, &p->arrival_time, &p->priority, &p->exec_time, &p->mem_req) == 5) {
            p->remaining_time = p->exec_time;
            p->next = NULL;
            submit_job(p);
        } else {
            free(p);
        }
    }
    fclose(file);
}

int main() {
    free_memory[0] = (MemoryBlock){0, TOTAL_MEMORY};
    load_dispatch_list("dispatch_list.txt");
    for (int i = 0; i < 10; i++) {
        run();
    }
    return 0;
}
