#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

typedef struct {
    int id_belt;
    int belt_size;
    int items_to_produce;
} process_data_t;

typedef struct {
    int id_belt;
    int items_to_produce;
} thread_data_t;


void *process_manager(void *arg);  // Thread function declaration

#endif