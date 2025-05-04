#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "queue.h"

typedef struct {
    struct element* buffer;
    int capacity;
    int count;
    int head;
    int tail;
    pthread_mutex_t mutex;
	pthread_cond_t not_empty;
	pthread_cond_t not_full;

} queue_t;

static queue_t queue; // Only one global queue

//To create a queue
int queue_init(int size){
	queue.buffer = (struct element*) malloc(sizeof(struct element) * size); // Allocate memory for the queue
	if (queue.buffer == NULL) {
		return -1; // Memory allocation failed
	}

	queue.capacity = size; // Set the capacity of the queue
	queue.head = 0; // Initialize front index
	queue.tail = 0; // Initialize rear index
	queue.count = 0; // Initialize count of elements

	if (pthread_mutex_init(&queue.mutex, NULL)){ // Initialize the mutex
		perror("Failed to initialize the mutex");
		return -1;
	}
	if (pthread_cond_init(&queue.not_empty, NULL)){ // Initialize the condition variable for not_empty
		perror("Failed to initialize the 'not_empty' condition variable");
		pthread_mutex_destroy(&queue.mutex); // Destroy the mutex if condition variable initialization fails
		return -1;
	}
	if (pthread_cond_init(&queue.not_full, NULL)){ // Initialize the condition variable for not_full
		perror("Failed to initialize the 'not_full' condition variable");
		pthread_cond_destroy(&queue.not_empty); // Destroy the not_empty condition variable
		pthread_mutex_destroy(&queue.mutex); // Destroy the mutex
		return -1;
	}

	return 0;
}


// To Enqueue an element
int queue_put(struct element* x) {
	// Lock the mutex to ensure exclusive access to the queue
	if (pthread_mutex_lock(&queue.mutex) != 0){
		perror("Failed to lock the mutex");
		return -1; // Failed to lock the mutex
	}

	// Wait until the queue is not full
	while (queue_full()) { // Check if the queue is full
		if (pthread_cond_wait(&queue.not_full, &queue.mutex) != 0) { // The producer thread waits until the queue is not full (not_full is signaled)
			perror("Failed to wait on the 'not_full' condition variable");
			pthread_mutex_unlock(&queue.mutex); // Unlock the mutex before returning
			return -1; // Failed to wait on the condition variable
		}
	}

	// Add the new element to the queue
	queue.buffer[queue.tail] = *x; // Add the element to the rear (end) of the queue
	queue.tail = (queue.tail + 1) % queue.capacity; // Update the rear index in a circular manner
	queue.count++; // Increment the count of elements in the queue

	printf("[OK][queue] Introduced element with id %d in belt %d.\n", x->num_edition, x->id_belt);

	if (pthread_cond_signal(&queue.not_empty) != 0) { // Notify the condition variable that the queue is not empty
		perror("Failed to signal the 'not_empty' condition variable");
		pthread_mutex_unlock(&queue.mutex); // Unlock the mutex before returning
		return -1; // Failed to signal the condition variable
	}

	if (pthread_mutex_unlock(&queue.mutex) != 0) { // Unlock the mutex
		perror("Failed to unlock the mutex");
		return -1; // Failed to unlock the mutex
	}

	return 0;
}


// To Dequeue an element.
struct element* queue_get(void) {
	if (pthread_mutex_lock(&queue.mutex) != 0) { // Lock the mutex to ensure exclusive access to the queue
		perror("Failed to lock the mutex");
		return NULL; // Failed to lock the mutex
	}

	while (queue_empty()){ // Wait until the queue is not empty
		if (pthread_cond_wait(&queue.not_empty, &queue.mutex) != 0) { // The consumer thread waits until the queue is not empty (not_empty is signaled)
			perror("Failed to wait on the 'not_empty' condition variable");
			pthread_mutex_unlock(&queue.mutex); // Unlock the mutex before returning
			return NULL; // Failed to wait on the condition variable
		}
	}

	struct element* out = malloc(sizeof(struct element));
	*out = queue.buffer[queue.head]; // Get the element from the front (head) of the queue

	queue.head = (queue.head + 1) % queue.capacity; // Update the front index in a circular manner
	queue.count--; // Decrement the count of elements in the queue

	printf("[OK][queue] Obtained element with id %d in belt %d.\n", out->num_edition, out->id_belt);

	if (pthread_cond_signal(&queue.not_full) != 0) { // Notify the condition variable that the queue is not full
		perror("Failed to signal the 'not_full' condition variable");
		pthread_mutex_unlock(&queue.mutex); // Unlock the mutex before returning
		return NULL; // Failed to signal the condition variable
	}
	
	if (pthread_mutex_unlock(&queue.mutex) != 0) { // Unlock the mutex
		perror("Failed to unlock the mutex");
		return NULL; // Failed to unlock the mutex
	}

	return out;
}


//To check queue state
int queue_empty(void){
	int is_empty;

	if (queue.count  == 0) {
		is_empty = 1; // Queue is empty
	} else {
		is_empty = 0; // Queue is not empty
	}	

	// Notify the condition variable that the queue is not empty
	return is_empty;
}

int queue_full(void){
	int is_full;

	if (queue.count  == queue.capacity) {
		is_full = 1; // Queue is full
	} else {
		is_full = 0; // Queue is not full
	}	

	// Notify the condition variable that the queue is not full
	return is_full;
}

//To destroy the queue and free the resources
int queue_destroy(void){
	free(queue.buffer);

	// Destroy the mutex and the condition variables
	if (pthread_mutex_destroy(&queue.mutex) != 0) {
		perror("Failed to destroy the mutex");
		return -1; // Failed to destroy the mutex
	}

	if (pthread_cond_destroy(&queue.not_empty) != 0) {
		perror("Failed to destroy the 'not_empty' condition variable");
		return -1; // Failed to destroy the not_empty condition variable
	}

	if (pthread_cond_destroy(&queue.not_full) != 0) {
		perror("Failed to destroy the 'not_full' condition variable");
		return -1; // Failed to destroy the not_full condition variable
	}

	return 0;
}