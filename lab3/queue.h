#ifndef HEADER_FILE
#define HEADER_FILE

#include <pthread.h>
#include <semaphore.h>

typedef struct {
  int num_edition;
  int id_belt;
  int last;
} element_t;

// Define the queue structure with synchronization mechanisms
typedef struct {
  element_t *queue;          // Pointer to the queue (array of elements)
  int capacity;               // Maximum size of the queue
  int front;                  // Index of the front element
  int rear;                   // Index of the rear element
  int count;                  // Number of elements in the queue
  pthread_mutex_t mutex;      // Mutex to synchronize access to the queue
  pthread_cond_t not_full;    // Condition variable to signal when the queue is not full
  pthread_cond_t not_empty;   // Condition variable to signal when the queue is not empty
} queue_t;


int queue_init(queue_t *q, int size);          // Initialize the queue
int queue_destroy(queue_t *q);                 // Destroy the queue
int queue_put(queue_t *q, element_t *elem);    // Insert an element into the queue
element_t* queue_get(queue_t *q);              // Remove an element from the queue
int queue_empty(queue_t *q);                   // Check if the queue is empty
int queue_full(queue_t *q);                    // Check if the queue is full

#endif