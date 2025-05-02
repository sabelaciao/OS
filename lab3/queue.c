#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "queue.h"

static struct element **queue = NULL; // Pointer to the queue
static int capacity = 0; // Maximum size of the queue
static int front = 0; // Index of the first element
static int rear = 0; // Index of the last element
static int count = 0; // Number of elements in the queue

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronizing access to the queue (ensures only one thread can access the queue at a time)
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER; // Condition variable (ensures that threads wait when the queue is empty) -> Used by consumers
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER; // Condition variable (ensures that threads wait when the queue is full) -> Used by producers

//To create a queue
int queue_init(int size){
	queue = malloc(sizeof(struct element*)*size); // size is the maximum size of the queue (given by the user)
	if (queue == NULL) {
		return -1; // Memory allocation failed
	}

	capacity = size; // Set the capacity of the queue
	front = 0; // Initialize front index
	rear = 0; // Initialize rear index
	count = 0; // Initialize count of elements

	if (thread_mutex_init(&mutex, NULL)){ // Initialize the mutex
		return -1;
	}
	if (pthread_cond_init(&not_empty, NULL)){ // Initialize the condition variable for not_empty
		pthread_mutex_destroy(&mutex); // Destroy the mutex if condition variable initialization fails
		return -1;
	}
	if (pthread_cond_init(&not_full, NULL)){ // Initialize the condition variable for not_full
		pthread_cond_destroy(&not_empty); // Destroy the not_empty condition variable
		pthread_mutex_destroy(&mutex); // Destroy the mutex
		return -1;
	}

	return 0; // Success
}


// To Enqueue an element
int queue_put(struct element* x) {

	return 0; // Success
}


// To Dequeue an element.
struct element* queue_get(void) {
	return NULL;
}


//To check queue state
int queue_empty(void){
	int is_empty;

	pthread_mutex_lock(&mutex); // Lock the mutex to ensure exclusive access to the queue

	if (count  == 0) {
		is_empty = 1; // Queue is empty
	} else {
		is_empty = 0; // Queue is not empty
	}	

    pthread_mutex_unlock(&mutex); // Unlock the mutex

	// Notify the condition variable that the queue is not empty
	return is_empty;
}

int queue_full(void){
	int is_full;

	pthread_mutex_lock(&mutex); // Lock the mutex to ensure exclusive access to the queue

	if (count  == capacity) {
		is_full = 1; // Queue is full
	} else {
		is_full = 0; // Queue is not full
	}	

    pthread_mutex_unlock(&mutex); // Unlock the mutex

	// Notify the condition variable that the queue is not full
	return is_full;
}

//To destroy the queue and free the resources
int queue_destroy(void){
	return 0;
}
