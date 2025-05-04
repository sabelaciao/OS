/*
 *
 * process_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <pthread.h>
 #include <semaphore.h>
 #include "queue.h"
 #include "process_manager.h"
 #include "factory_manager.h"
 
 void *producer(void *arg) {
    printf("hey7!\n");
    process_data_t *producer_data = (process_data_t *)arg;
    if (producer_data == NULL) {
        printf("[ERROR][process_manager] Arguments not valid.\n");
        pthread_exit((void *)-1);
    }

    int id_belt = producer_data->id_belt;
    int items_to_produce = producer_data->elements_to_generate;
    queue_t *belt_queue = producer_data->queue; // Access the belt queue directly

    printf("hey8!\n");
    for (int i = 0; i < items_to_produce; i++) {
        element_t *e = malloc(sizeof(element_t));
        e->num_edition = i;
        e->id_belt = id_belt;
        e->last = (i == items_to_produce - 1) ? 1 : 0;

		pthread_mutex_lock(&belt_queue->mutex); // Lock the mutex to ensure exclusive access to the queue
        printf("hey9!\n");
        // Wait if the queue is full
        while (queue_full(belt_queue)) {
			pthread_cond_wait(&belt_queue->not_full, &belt_queue->mutex); // Wait until the queue is not full
		}
        printf("hey10!\n");

        if (queue_put(belt_queue, e) != 0) {
            printf("[ERROR][queue] Error adding item to queue for belt %d.\n", id_belt);
            free(e);
            pthread_exit((void *)-1);
        }
        printf("hey11!\n");

		pthread_mutex_unlock(&belt_queue->mutex); // Unlock the mutex
        printf("[OK][queue] Introduced element with id %d in belt %d.\n", e->num_edition, e->id_belt);
    }

    pthread_exit(NULL);
}

 
void *consumer(void *arg) {
    printf("hey12!\n");
    process_data_t *consumer_data = (process_data_t *)arg;
    if (consumer_data == NULL) {
        printf("[ERROR][process_manager] Arguments not valid.\n");
        pthread_exit((void *)-1);
    }
    queue_t *belt_queue = consumer_data->queue; // Access the belt queue directly

    for (int i = 0; i < consumer_data->elements_to_generate; i++) {
		pthread_mutex_lock(&belt_queue->mutex); // Lock the mutex to ensure exclusive access to the queue
        printf("hey13!\n");
        while (queue_empty(belt_queue)) {
			pthread_cond_wait(&belt_queue->not_empty, &belt_queue->mutex); // Wait until the queue is not empty
		}

        printf("hey14!\n");

        // Dequeue an element
        element_t *e = queue_get(belt_queue);

        if (e == NULL) {
            printf("[ERROR][queue] Error getting item from queue for belt %d.\n", consumer_data->id_belt);
            pthread_exit((void *)-1);
        }
        printf("hey15!\n");
		pthread_mutex_unlock(&belt_queue->mutex); // Unlock the mutex

        printf("[OK][queue] Obtained element with id %d in belt %d.\n", e->num_edition, e->id_belt);
        free(e); // Free the allocated memory for the element
    }

    pthread_exit(NULL);
}

 
 
void *process_manager(void *arg) {
    process_data_t *element = (process_data_t *)arg;
    element->queue = malloc(sizeof(queue_t));

    if (!element->queue) {
        fprintf(stderr, "[ERROR] Could not allocate queue_t struct.\n");
        exit(EXIT_FAILURE);
    }

    if (element == NULL) {
        printf("[ERROR][process_manager] Arguments not valid.\n");
        pthread_exit((void *)-1); // Exit with error
    }

    int id_belt = element->id_belt;
    int belt_size = element->belt_size;
    int items_to_produce = element->elements_to_generate;

    // Wait until the semaphore is signaled
    if (sem_wait(&element->semaphore_b) == -1) {
        printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
        pthread_exit((void *)-1); // Exit with error
    }

    printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id_belt, items_to_produce);

    printf("hey!\n");
    // Signal the factory semaphore to indicate that the process_manager is ready
    if (sem_post(&factory_semaphore) != 0) {
        printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
        pthread_exit((void *)-1); // Exit with error
    }
    printf("hey2!\n");
    // Wait for the factory semaphore to be signaled
    if (sem_wait(&element->semaphore_b) != 0) {
        printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
        pthread_exit((void *)-1); // Exit with error
    }
    printf("hey3!\n");

    // Initialize the queue (conveyor belt)
    if (queue_init(element->queue, belt_size) != 0) {
        printf("[ERROR][queue] Error initializing queue for belt %d.\n", id_belt);
        pthread_exit((void *)-1); // Exit with error
    }
    printf("hey4!\n");
    printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id_belt, belt_size);

    // Create the producer and consumer threads
    pthread_t producer_thread;
    pthread_t consumer_thread;

    if (pthread_create(&producer_thread, NULL, producer, (void *)element) != 0 ||
        pthread_create(&consumer_thread, NULL, consumer, (void *)element) != 0) {
        printf("[ERROR][process_manager] Error creating producer or consumer thread for belt %d.\n", id_belt);
        queue_destroy(element->queue);
        pthread_exit((void *)-1);
    }
    printf("hey5!\n");

    // Wait for the producer and consumer threads to finish
    if (pthread_join(producer_thread, NULL) != 0 || pthread_join(consumer_thread, NULL) != 0) {
        printf("[ERROR][process_manager] Error joining producer or consumer thread for belt %d.\n", id_belt);
        queue_destroy(element->queue);
        pthread_exit((void *)-1);
    }

    printf("hey6!\n");

    printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id_belt, items_to_produce);

    
    // Remove the queue
    if (queue_destroy(element->queue) != 0) {
        printf("[ERROR][queue] Error destroying queue for belt %d.\n", id_belt);
        pthread_exit((void *)-1);
    }  
    printf("hey20!\n");

    pthread_exit(NULL); // Exit the thread
}
