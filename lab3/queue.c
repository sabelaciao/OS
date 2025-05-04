#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "queue.h"

// To create a queue
int queue_init(queue_t *q, int size) {
    // Allocate memory for the queue
    q->queue = malloc(sizeof(element_t*) * size);
    if (q->queue == NULL) {
        return -1; // Memory allocation failed
    }

    q->capacity = size; // Set the capacity of the queue
    q->front = 0; // Initialize front index
    q->rear = 0; // Initialize rear index
    q->count = 0; // Initialize count of elements

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&q->mutex, NULL)) {
        perror("Failed to initialize the mutex");
        free(q->queue);
        return -1;
    }
    if (pthread_cond_init(&q->not_empty, NULL)) {
        perror("Failed to initialize the 'not_empty' condition variable");
        pthread_mutex_destroy(&q->mutex);
        free(q->queue);
        return -1;
    }
    if (pthread_cond_init(&q->not_full, NULL)) {
        perror("Failed to initialize the 'not_full' condition variable");
        pthread_cond_destroy(&q->not_empty);
        pthread_mutex_destroy(&q->mutex);
        free(q->queue);
        return -1;
    }

    return 0;
}


// To Enqueue an element
int queue_put(queue_t *q, element_t *x) {


    // Wait until the queue is not full
    while (queue_full(q)) {
        if (pthread_cond_wait(&q->not_full, &q->mutex) != 0) {
            perror("Failed to wait on the 'not_full' condition variable");
            pthread_mutex_unlock(&q->mutex);
            return -1;
        }
    }

    // Add the new element to the queue
    q->queue[q->rear] = *x; // Add the element to the rear of the queue
    q->rear = (q->rear + 1) % q->capacity; // Update the rear index in a circular manner
    q->count++; // Increment the count of elements in the queue

    if (pthread_cond_signal(&q->not_empty) != 0) {
        perror("Failed to signal the 'not_empty' condition variable");
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }

    return 0;
}


// To Dequeue an element
element_t* queue_get(queue_t *q) {

    // Wait until the queue is not empty
    while (q->count == 0) {
        if (pthread_cond_wait(&q->not_empty, &q->mutex) != 0) {
            perror("Failed to wait on the 'not_empty' condition variable");
            pthread_mutex_unlock(&q->mutex);
            return NULL;
        }
    }

    // Dynamically allocate memory for the item
    element_t* item = malloc(sizeof(element_t));
    if (item == NULL) {
        perror("Failed to allocate memory for the item");
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }

    // Copy the data from the queue element to the allocated memory
    *item = q->queue[q->front]; // Copy the element into the allocated memory

    q->front = (q->front + 1) % q->capacity; // Update the front index in a circular manner
    q->count--; // Decrement the count of elements in the queue

    if (pthread_cond_signal(&q->not_full) != 0) {
        perror("Failed to signal the 'not_full' condition variable");
        pthread_mutex_unlock(&q->mutex);
        free(item);  // Free allocated memory on error
        return NULL;
    }

    return item;
}



//To check queue state
int queue_empty(queue_t *q) {
    int is_empty;

    if (q->count == 0) {
        is_empty = 1; // Queue is empty
    } else {
        is_empty = 0; // Queue is not empty
    }
    return is_empty;
}

// To check if the queue is full
int queue_full(queue_t *q) {
	if (!q) {
        printf("[DEBUG][queue_full] queue pointer is NULL!\n");
        return 1; // Avoid segfault
    }

    printf("[DEBUG][queue_full] count = %d, capacity = %d\n", q->count, q->capacity);
    int is_full;

    if (q->count == q->capacity) {
        is_full = 1; // Queue is full
    } else {
        is_full = 0; // Queue is not full
    }
	printf("[DEBUG][queue_full] is_full = %d\n", is_full);
    return is_full;
}

//To destroy the queue and free the resources
int queue_destroy(queue_t *q) {
	printf("[DEBUG][queue_destroy] Destroying queue...\n");
	if (pthread_mutex_lock(&q->mutex) != 0) {
        perror("Failed to lock mutex during destruction");
        return -1;
    }
    free(q->queue);
    q->queue = NULL;
    q->capacity = 0;
    q->front = 0;
    q->rear = 0;
    q->count = 0;
	printf("[DEBUG][queue_destroy] Queue destroyed.\n");
	
	if (pthread_mutex_unlock(&q->mutex) != 0) {
        perror("Failed to unlock mutex during destruction");
        // Continue with destruction anyway
    }
    // Destroy the mutex and the condition variables
    if (pthread_mutex_destroy(&q->mutex) != 0) {
        perror("Failed to destroy the mutex");
        return -1;
    }
	printf("[DEBUG][queue_destroy] Queue destroyed2.\n");
    if (pthread_cond_destroy(&q->not_empty) != 0) {
        perror("Failed to destroy the 'not_empty' condition variable");
        return -1;
    }
	printf("[DEBUG][queue_destroy] Queue destroyed3.\n");
    if (pthread_cond_destroy(&q->not_full) != 0) {
        perror("Failed to destroy the 'not_full' condition variable");
        return -1;
    }

    return 0;
}