#ifndef HEADERS_H
#include "headers.h"
#endif

typedef struct {
  u_int32_t arrival;
  u_int32_t elapsed;
  u_int32_t runtime;
} process;

typedef struct
{
  size_t SIZE;
  size_t occupied;

  size_t front;
  size_t back;

  process **buffer;
} circularQueue;

circularQueue CIRCULAR_QUEUE_DEFAULT = { 0, 0, 0, 0, NULL };

/**
 * Allocates a buffer for incoming processes
 * @return -1 on failure, 0 on success
 */
int createQueue(circularQueue *queue, size_t size) {
  if (!queue || !size) return -1;
  if (queue->buffer) return -1;

  queue->SIZE = size;
  queue->buffer = (process**) calloc(queue->SIZE, sizeof(process*));
  return 0;
}

/**
 * Frees the buffer allocated by the queue and resets data
 * @return -1 on failure, 0 on success
 */
int freeQueue(circularQueue *queue) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;

  free(queue->buffer);
  return 0;
}

/**
 * Dequeue a pointer to a process from the queue.
 * @return NULL on failure, the pointer on success
 */
process* dequeueFromQueue(circularQueue *queue) {
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

/**
 * Enqueue a pointer to a process in the queue.
 * @return -1 on failure, 0 on success
 */
int enqueueInQueue(circularQueue *queue, process *p) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;

  // Queue is full
  if (queue->occupied == queue->SIZE) return -1;

  queue->buffer[queue->back] = p;
  queue->back = (queue->back + 1) % queue->SIZE; // Move back circularly
  queue->occupied = queue->occupied < queue->SIZE ? queue->occupied+1 : queue->SIZE;

  return 0;
}
