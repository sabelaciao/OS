#ifndef FACTORY_MANAGER_H
#define FACTORY_MANAGER_H

#include <semaphore.h>

typedef struct {
    int id_belt;           // Belt ID (used as index in semaphore array)
    int belt_size;         // Max size of the queue (belt capacity)
    int items_to_produce;  // How many items the producer should create
} process_data_t;

extern sem_t *sem_processes;

#endif