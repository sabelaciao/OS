#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "queue.h"

static struct element **queue = NULL; // Pointer to the queue
static int capacity = 0; // Maximum size of the queue
static int front = 0; // Index of the first element
static int rear = 0; // Index of the last element
static int count = 0; // Number of elements in the queue

//To create a queue
int queue_init(int size){
	return 0;
}


// To Enqueue an element
int queue_put(struct element* x) {
	return 0;
}


// To Dequeue an element.
struct element* queue_get(void) {
	return NULL;
}


//To check queue state
int queue_empty(void){
	return 0;
}

int queue_full(void){
	return 0;
}

//To destroy the queue and free the resources
int queue_destroy(void){
	return 0;
}
