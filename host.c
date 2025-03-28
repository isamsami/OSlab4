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

void submit_job(Process* process) {
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

void execute() {
    Process* process = schedule();
    if (!process) {
        printf("No process to execute.\n");
        return;
    }
    int time_quantum = (process->priority > 0) ? TIME_QUANTUM : process->exec_time;
    process->remaining_time -= time_quantum;
    printf("Executing process %d for %d units. Remaining time: %d\n", process->pid, time_quantum, process->remaining_time);
    if (process->remaining_time > 0 && process->priority > 0) {
        int new_priority = (process->priority + 1 < 3) ? process->priority + 1 : 2;
        insert_process(&priority_queues[new_priority], process);
    } else {
        printf("Process %d completed.\n", process->pid);
        free(process);
    }
}

void load_dispatch_list() {
    char filename[100];
    printf("Enter the dispatcher list file path: ");
    scanf("%s", filename);
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }
    char line[MAX_LINE_LENGTH];
    printf("Loading dispatch list...\n");
    while (fgets(line, sizeof(line), file)) {
        Process* p = (Process*)malloc(sizeof(Process));
        if (sscanf(line, "%d %d %d %d %d", &p->pid, &p->arrival_time, &p->priority, &p->exec_time, &p->mem_req) == 5) {
            p->remaining_time = p->exec_time;
            p->next = NULL;
            printf("Loaded process %d\n", p->pid);
            submit_job(p);
        } else {
            free(p);
        }
    }
    fclose(file);
    printf("Dispatch list loaded.\n");
}

int main() {
    load_dispatch_list();
    while (real_time_queue || priority_queues[0] || priority_queues[1] || priority_queues[2]) {
        execute();
    }
    return 0;
}

