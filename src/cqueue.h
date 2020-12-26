#ifndef CQUEUE_H
#define CQUEUE_H

#include "headers.h"

typedef struct process {
  size_t arrival;
  size_t runtime;

  size_t remaining;
  size_t waiting;

  size_t priority;
  size_t status;
  size_t pid;
} process;

typedef struct cqueue {
  size_t SIZE;
  size_t occupied;

  size_t front;
  size_t back;

  process **buffer;
} cqueue;

cqueue CIRCULAR_QUEUE_DEFAULT = {0, 0, 0, 0, NULL};

/**
 * Allocates a buffer for incoming processes
 * @return -1 on failure, 0 on success
 */
int cqueue_create(cqueue *queue, size_t size);

/**
 * Frees the buffer allocated by the queue and resets data
 * @return -1 on failure, 0 on success
 */
int cqueue_free(cqueue *queue);

/**
 * Dequeues a pointer to a process from the queue
 * @return NULL on failure, the pointer on success
 */
process* cqueue_dequeue(cqueue *queue);

/**
 * Returns the front of the queue
 * @return NULL on failure, the pointer on success
 */
process* cqueue_front(cqueue *queue);

/**
 * Enqueues a pointer to a process in the queue
 * @return -1 on failure, 0 on success
 */
int cqueue_enqueue(cqueue *queue, process *p);

/**
 * Dequeues the last enqueued pointer to process
 * @return NULL on failure, the pointer on success
 */
process *cqueue_backDequeue(cqueue *queue);

int cqueue_create(cqueue *queue, size_t size) {
  if (!queue || !size) return -1;
  if (queue->buffer) return -1;

  queue->SIZE = size;
  queue->buffer = (process**) calloc(queue->SIZE, sizeof(process*));
  return 0;
}

int cqueue_free(cqueue *queue) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;

  free(queue->buffer);
  return 0;
}

process* cqueue_dequeue(cqueue *queue) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->occupied) return NULL;

  // Remove front element
  process *p = queue->buffer[queue->front];
  queue->front = (queue->front + 1) % queue->SIZE; // Move back circularly
  queue->occupied = queue->occupied ? queue->occupied-1 : 0;

  return p;
}

process* cqueue_front(cqueue *queue) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->occupied) return NULL;

  return queue->buffer[queue->front];
}

int cqueue_enqueue(cqueue *queue, process *p) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;

  // Queue is full
  if (queue->occupied == queue->SIZE) return -1;

  queue->buffer[queue->back] = p;
  queue->back = (queue->back + 1) % queue->SIZE; // Move back circularly
  queue->occupied = queue->occupied < queue->SIZE ? queue->occupied+1 : queue->SIZE;

  return 0;
}

process* cqueue_backDequeue(cqueue *queue) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->occupied) return NULL;

  // Remove back element
  process *p = queue->buffer[queue->back];
  queue->back = (queue->back + queue->SIZE-1) % queue->SIZE; // Move forward circularly
  queue->occupied = queue->occupied ? queue->occupied-1 : 0;

  return p;
}
#endif