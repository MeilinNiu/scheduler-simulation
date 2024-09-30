#include <stdio.h>
#include <stdlib.h>
#include <limits.h>  // For INT_MAX
#include <stdbool.h>

#define NUM_QUEUES 3
#define MAX_PROCESSES 100
#define RESET_PERIOD 50

// Process structure definition
typedef struct {
    int pid;
    int arrival_time;
    int remaining_time;
    int current_queue_idx;
    int total_time_in_level;
    int last_executed_time;
} Process;

// Define the time quantum and time allotment per queue
int time_quantum[NUM_QUEUES] = {16, 8, 4};  // Queue 2 has the largest quantum, Queue 0 the smallest
int time_allotment[NUM_QUEUES] = {INT_MAX, 20, 10};  // Queue 2 has the highest allotment

// Multi-level queue structure, using arrays
Process* queues[NUM_QUEUES][MAX_PROCESSES];
int front[NUM_QUEUES] = {0};  // Tracks the front of each queue
int rear[NUM_QUEUES] = {0};   // Tracks the rear of each queue

// Global clock
int current_time = 0;


// Function to check if all queues are empty
bool queues_empty() {
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (front[i] < rear[i]) {
            return false;
        }
    }
    return true;
}

// Function to reset priorities of all processes
void priority_reset() {
    for (int q = 1; q >= 0; q--) {  // Reset queues 1 and 0, not Queue 2 (highest priority)
        while (front[q] < rear[q]) {
            Process* p = queues[q][front[q]++];
            p->current_queue_idx = 2;  // Move all processes back to the highest priority queue (Queue 2)
            queues[2][rear[2]++] = p;
            printf("Process %d moved to Queue 2 during reset\n", p->pid);
        }
    }
}

// Function to create a process
Process* create_process(int pid, int arrival_time, int burst_time) {
    Process* p = (Process*)malloc(sizeof(Process));
    p->pid = pid;
    p->arrival_time = arrival_time;
    p->remaining_time = burst_time;
    p->current_queue_idx = 2;  // Start at the highest priority queue (Queue 2)
    p->total_time_in_level = 0;
    p->last_executed_time = 0;
    return p;
}

// Function to display the current state of the queues
void display_queues() {
    printf("\n--- Current Queue State at time %d ---\n", current_time);
    for (int q = NUM_QUEUES - 1; q >= 0; q--) {  // Print from Queue 2 (high priority) to Queue 0 (low priority)
        printf("Queue %d: ", q);
        for (int i = front[q]; i < rear[q]; i++) {
            printf("[P%d, Remaining Time: %d] ", queues[q][i]->pid, queues[q][i]->remaining_time);
        }
        printf("\n");
    }
    printf("-------------------------------------\n\n");
}

// Function to simulate the MLFQ scheduling
void simulate_mlfq(Process* process_list[], int num_processes) {
    int process_idx = 0;

    while (process_idx < num_processes || !queues_empty()) {
        // Check for process arrival
        while (process_idx < num_processes && process_list[process_idx]->arrival_time <= current_time) {
            Process* arriving_process = process_list[process_idx];
            process_idx++;
            printf("Process %d arrives at time %d\n", arriving_process->pid, current_time);
            queues[2][rear[2]++] = arriving_process; // Add to the highest priority queue (Queue 2)
        }

        // Display the current state of the queues
        display_queues();

        // Handle scheduling and execution
        for (int q = NUM_QUEUES - 1; q >= 0; q--) {  // Start from highest priority (Queue 2) and go down
            if (front[q] < rear[q]) {  // Check if the queue is not empty
                Process* p = queues[q][front[q]++];
                
                int exec_time = (p->remaining_time < time_quantum[q]) ? p->remaining_time : time_quantum[q];
                exec_time = (p->total_time_in_level + exec_time > time_allotment[q]) ? time_allotment[q] - p->total_time_in_level : exec_time;

                printf("Running Process %d from Queue %d for %d time units\n", p->pid, q, exec_time);

                p->remaining_time -= exec_time;
                p->total_time_in_level += exec_time;
                p->last_executed_time = current_time;
                current_time += exec_time;

                // Process completion
                if (p->remaining_time == 0) {
                    printf("Process %d completed at time %d\n", p->pid, current_time);
                    free(p); // Clean up process memory
                }
                // Process demotion after using up time allotment
                else if (p->total_time_in_level >= time_allotment[q]) {
                    p->current_queue_idx = (q > 0) ? q - 1 : 0;  // Demote to the next lower priority queue if possible
                    p->total_time_in_level = 0;  // Reset time in level
                    queues[p->current_queue_idx][rear[p->current_queue_idx]++] = p;
                    printf("Process %d demoted to Queue %d\n", p->pid, p->current_queue_idx);
                }
                // Process requeuing in the same priority level
                else {
                    queues[q][rear[q]++] = p;
                }

                break;  // Execute one process per time step, then check for arrivals
            }
        }

        // Handle priority reset
        if (current_time % RESET_PERIOD == 0 && current_time > 0) {
            printf("Priority reset at time %d\n", current_time);
            priority_reset();
        }

        current_time++; // Increment the global clock
    }
}

int main() {
    // Create a list of processes
    Process* process_list[MAX_PROCESSES];
    int num_processes = 3;
    
    process_list[0] = create_process(1, 0, 25);
    process_list[1] = create_process(2, 2, 30);
    process_list[2] = create_process(3, 5, 15);
    
    // Start simulation
    simulate_mlfq(process_list, num_processes);

    return 0;
}