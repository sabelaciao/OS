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
 
extern sem_t factory_semaphore;


void *producer(void *arg){
 	process_data_t *data = (process_data_t *)arg;
	if (data == NULL) {
		printf("[ERROR][process_manager] Arguments not valid.\n");
		pthread_exit((void *)-1);
	}
 
	int id_belt = data->id_belt;
	int items_to_produce = data->elements_to_generate;
 
	for (int i = 0; i < items_to_produce; i++) {
		// Allocate memory for the element
		struct element *e = malloc(sizeof(struct element)); 
		e->num_edition = i;
		e->id_belt = id_belt;
		if (i == items_to_produce - 1) {
			e->last = 1; // Mark the last item
		} else {
			e->last = 0; // Not the last item
		}
        
        // Enqueue the element
		if (queue_put(e) != 0) {
			printf("[ERROR][queue] There was an error while using queue with id: %d.\n", id_belt);
			free(e);
			pthread_exit((void *)-1);
		}
	}
 
	pthread_exit(NULL);
}
 
void *consumer(void *arg){
	process_data_t *data = (process_data_t *)arg;
	if (data == NULL) {
		printf("[ERROR][process_manager] Arguments not valid.\n");
		pthread_exit((void *)-1);
	}
    
	int consumed = 0;
	int total_elements = data->elements_to_generate;
 
    // 'Consume' the elements from the queue
	while (consumed < total_elements) {
 
		// Dequeue an element
		struct element *e = queue_get();
 
		if (e == NULL) {
			printf("[ERROR][queue] There was an error while using queue with id: %d.\n", data->id_belt);
			pthread_exit((void *)-1);
		}
 
		consumed++;
		free(e); // Free the allocated memory for the element
	}
 
	pthread_exit(NULL);
}
 
 
void *process_manager(void *arg) {
	process_data_t *element = (process_data_t *)arg;
	
	if (element == NULL) {
		printf("[ERROR][process_manager] Arguments not valid.\n");
		pthread_exit((void *)-1); // Exit with error
	}
	
    // Extract the parameters from the argument
	int id_belt = element->id_belt;
	int belt_size = element->belt_size;
	int elements_to_generate = element->elements_to_generate;

    // Wait for the semaphore to start the process_manager
	if (sem_wait(&factory_semaphore) != 0) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		pthread_exit((void *)-1); // Exit with error
	}
	
	printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id_belt, elements_to_generate);
	

	// Create the queue (conveyor belt)
	if (queue_init(belt_size) != 0) {
		printf("[ERROR][queue] There was an error while using queue with id: %d\n", id_belt);
		pthread_exit((void *)-1); // Exit with error
	}

	printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id_belt, belt_size);
 
 
 
	// Create the producer and consumer threads
	pthread_t producer_thread;
	pthread_t consumer_thread;
 
    // Initialize the producer and consumer threads
	if (pthread_create(&producer_thread, NULL, producer, element) != 0 || pthread_create(&consumer_thread, NULL, consumer, element) != 0) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		queue_destroy();
		pthread_exit((void *)-1);
	}
	 
	// Wait for the producer and consumer threads to finish
	if (pthread_join(producer_thread, NULL) != 0 || pthread_join(consumer_thread, NULL) != 0) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		queue_destroy();
		pthread_exit((void *)-1);
	}
 
	printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id_belt, elements_to_generate);
 
 
	// Remove the queue
	if (queue_destroy() != 0) {
		printf("[ERROR][queue] There was an error while using queue with id: %d\n", id_belt);
		pthread_exit((void *)-1);
	}

    // Signal (alert) the semaphore to indicate that the process_manager has finished
	if (sem_post(&factory_semaphore) != 0) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		pthread_exit((void *)-1);
	}
 
	pthread_exit(NULL); // Exit the thread
}