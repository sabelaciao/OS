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

void *producer(void *arg){

	// Get the thread data
	thread_data_t *data = (thread_data_t *)arg;
	if (data == NULL) {
		printf("[ERROR][process_manager] Arguments not valid.\n");
		pthread_exit((void *)-1);
	}

	int id_belt = data->id_belt;
	int items_to_produce = data->items_to_produce;

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

		// Wait if queue is full
		while (queue_full()) {
			usleep(100); // Sleep for a while to avoid busy waiting
		}

		if (queue_put(e) != 0) {
			printf("“[ERROR][queue] There was an error while using queue with id: %d.\n", id_belt);
			free(e);
			pthread_exit((void *)-1);
		}
		printf("[OK][queue] Introduced element with id %d in belt %d.\n", e->num_edition, e->id_belt);
	}

	pthread_exit(NULL);
}

void *consumer(void *arg){
	// Get the thread data
	thread_data_t *data = (thread_data_t *)arg;
	if (data == NULL) {
		printf("[ERROR][process_manager] Arguments not valid.\n");
		pthread_exit((void *)-1);
	}

	int finished = 0;

	while (!finished) {
		while (queue_empty()) {
			usleep(100); // Sleep for a while to avoid busy waiting
		}

		// Dequeue an element
		struct element *e = queue_get();

		if (e == NULL) {
			printf("[ERROR][queue] There was an error while using queue with id: %d.\n", data->id_belt);
			pthread_exit((void *)-1);
		}

		printf("[OK][queue] Obtained element with id %d in belt %d.\n", e->num_edition, e->id_belt);
		if (e->last == 1) {
			finished = 1; // Mark as finished if it's the last item
		}
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

	int id_belt = element->id_belt;
    int belt_size = element->belt_size;
    int items_to_produce = element->elements_to_generate;

	printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id_belt, items_to_produce);

	// Wait until the semaphore is signaled
	extern sem_t *sem_processes;
	if (sem_wait(&sem_processes[id_belt]) == -1) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		pthread_exit((void *)-1); // Exit with error
	}


	// Create the queue (conveyor belt)
	if (queue_init(belt_size) != 0) {
		printf("[ERROR][queue] There was an error while using queue with id: %d\n", id_belt);
		pthread_exit((void *)-1); // Exit with error
	}
	printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id_belt, belt_size);


	// Create the producer and consumer threads
	pthread_t producer_thread;
	pthread_t consumer_thread;

	thread_data_t *process_data = malloc(sizeof(thread_data_t)); // Allocate memory for the thread data
	process_data->id_belt = id_belt;
	process_data->items_to_produce = items_to_produce;

	if (pthread_create(&producer_thread, NULL, producer, (void *)process_data) != 0 || pthread_create(&consumer_thread, NULL, consumer, (void *)process_data) != 0) {
        printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
        queue_destroy();
        free(process_data);
        pthread_exit((void *)-1);
    }
	
	// Wait for the producer and consumer threads to finish
	if (pthread_join(producer_thread, NULL) != 0 || pthread_join(consumer_thread, NULL) != 0) {
		printf("[ERROR][process_manager] There was an error executing process_manager with id %d.\n", id_belt);
		queue_destroy();
		free(process_data);
		pthread_exit((void *)-1);
	}

	printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id_belt, items_to_produce);


	// Remove the queue
	if (queue_destroy() != 0) {
		printf("[ERROR][queue] There was an error while using queue with id: %d\n", id_belt);
		pthread_exit((void *)-1);
	}

	// Free the allocated memory for thread data
	free(process_data); 

	pthread_exit(NULL); // Exit the thread
}