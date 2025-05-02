#ifndef FACTORY_MANAGER_H
#define FACTORY_MANAGER_H

#include <semaphore.h>

// Structure to hold process_manager parameters
typedef struct {
    int id_belt;         
    int belt_size;        
    int elements_to_generate;
} process_data_t;

extern sem_t *sem_processes;

#endif