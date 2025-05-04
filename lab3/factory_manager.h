#ifndef FACTORY_MANAGER_H
#define FACTORY_MANAGER_H

#include <semaphore.h>
#include "queue.h"
#include <pthread.h>

// Structure to hold process_manager parameters
typedef struct {
    int id_belt;          
    int belt_size;        
    int elements_to_generate;
    sem_t semaphore_b;  // Semaphore for synchronization between threads
    pthread_t thread_b;  // Thread managing the belt
    queue_t *queue;      // Queue to store produced/processed items
} process_data_t;


extern sem_t factory_semaphore; // Global variable to hold the semaphores

#endif