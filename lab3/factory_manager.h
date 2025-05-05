#ifndef FACTORY_MANAGER_H
#define FACTORY_MANAGER_H

#include <semaphore.h>

// Structure to hold process_manager parameters
typedef struct {
    int id_belt;         
    int belt_size;        
    int elements_to_generate;
    sem_t semaphore_b; // Semaphore assigned to the corresponding belt
    pthread_t thread_b; // Thread assigned to the corresponding belt
} process_data_t;

// Semaphore to synchronize the factory
extern sem_t factory_semaphore;

#endif