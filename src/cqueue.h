#ifndef CQUEUE_H
#define CQUEUE_H

#include "headers.h"

typedef struct cqueue {
  size_t CAPACITY;
  size_t size;

  size_t front;
  size_t back;

  process **buffer;
} cqueue;

/**
 * Returns the next index, moving towards the front.
 * @return index.
 */
size_t __cqueue_forward(cqueue *queue, size_t i) {
  return (i + queue->CAPACITY - 1) % queue->CAPACITY;
}

/**
 * Returns the next index, moving towards the back.
 * @return index.
 */
size_t __cqueue_backward(cqueue *queue, size_t i) {
  return (i + 1) % queue->CAPACITY;
}

/**
 * Allocates a buffer for incoming processes.
 * @return -1 on failure, 0 on success.
 */
int cqueue_create(cqueue *queue, size_t capacity) {
  if (!queue || !capacity) return -1;
  if (queue->buffer) return -1;

  queue->CAPACITY = capacity;
  queue->buffer = (process **)calloc(queue->CAPACITY, sizeof(process *));
  return 0;
}

/**
 * Frees the buffer allocated by the queue and resets data.
 * @return -1 on failure, 0 on success.
 */
int cqueue_free(cqueue *queue) {
  if (!queue)
    return -1;
  if (!queue->buffer)
    return -1;

  free(queue->buffer);
  return 0;
}

/**
 * Dequeues a pointer to a process from the queue.
 * @return NULL on failure, the pointer on success.
 */
process *cqueue_dequeue(cqueue *queue) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->size)
    return NULL;

  // Remove front element
  process *p = queue->buffer[queue->front];
  queue->front = __cqueue_backward(queue, queue->front);  // Move back circularly
  queue->size = queue->size ? queue->size - 1 : 0;

  return p;
}

/**
 * Returns the front of the queue.
 * @return NULL on failure, the pointer on success.
 */
process *cqueue_front(cqueue *queue) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->size) return NULL;

  return queue->buffer[queue->front];
}

/**
 * Enqueues a pointer to a process in the queue.
 * @return -1 on failure, 0 on success.
 */
int cqueue_enqueue(cqueue *queue, process *p) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;

  // Queue is full
  if (queue->size == queue->CAPACITY) return -1;

  queue->buffer[queue->back] = p;
  queue->back = __cqueue_backward(queue, queue->back);  // Move back circularly
  queue->size = queue->size < queue->CAPACITY ? queue->size + 1 : queue->CAPACITY;

  return 0;
}

/**
 * Removes a pointer to a process from the queue. 
 * Searches starting from the end.
 * @return NULL on failure, the pointer on success.
 */
process *cqueue_remove(cqueue *queue, process *p) {
  if (!queue) return NULL;
  if (!queue->buffer) return NULL;

  // Queue is empty
  if (!queue->size) return NULL;

  // Look for p starting at one step ahead of queue->back
  size_t i = __cqueue_forward(queue, queue->back);
  for (; i != queue->back; i = __cqueue_forward(queue, i)) {
    if (queue->buffer[i] == p) break;
  }
  if (i == queue->back) return NULL;

  for (; i != queue->back; i = __cqueue_backward(queue, i)) {
    queue->buffer[i] = queue->buffer[__cqueue_backward(queue, i)];
  }

  queue->back = __cqueue_forward(queue, queue->back);  // Move forward circularly
  queue->size = queue->size ? queue->size - 1 : 0;
  return p;
}

#endif