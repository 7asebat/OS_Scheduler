#ifndef CQUEUE_H
#define CQUEUE_H

#include "headers.h"

typedef struct cqueue {
  size_t SIZE;
  size_t occupied;

  size_t front;
  size_t back;

  process **buffer;
} cqueue;

cqueue CIRCULAR_QUEUE_DEFAULT = {0, 0, 0, 0, NULL};

/**
 * Returns the next index, moving towards the front.
 * @return index.
 */
size_t __cqueue_forward(cqueue *queue, size_t i) {
  return (i + queue->SIZE - 1) % queue->SIZE;
}

/**
 * Returns the next index, moving towards the back.
 * @return index.
 */
size_t __cqueue_backward(cqueue *queue, size_t i) {
  return (i + 1) % queue->SIZE;
}

/**
 * Allocates a buffer for incoming processes.
 * @return -1 on failure, 0 on success.
 */
int cqueue_create(cqueue *queue, size_t size) {
  if (!queue || !size) return -1;
  if (queue->buffer) return -1;

  queue->SIZE = size;
  queue->buffer = (process **)calloc(queue->SIZE, sizeof(process *));
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
  if (!queue->occupied)
    return NULL;

  // Remove front element
  process *p = queue->buffer[queue->front];
  queue->front = __cqueue_backward(queue, queue->front);  // Move back circularly
  queue->occupied = queue->occupied ? queue->occupied - 1 : 0;

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
  if (!queue->occupied) return NULL;

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
  if (queue->occupied == queue->SIZE) return -1;

  queue->buffer[queue->back] = p;
  queue->back = __cqueue_backward(queue, queue->back);  // Move back circularly
  queue->occupied = queue->occupied < queue->SIZE ? queue->occupied + 1 : queue->SIZE;

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
  if (!queue->occupied) return NULL;

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
  queue->occupied = queue->occupied ? queue->occupied - 1 : 0;
  return p;
}

size_t cqueue_get_size(cqueue *queue) {
  return queue->occupied;
}

process *cqueue_getElemAt(cqueue *queue, int idx) {
  if (idx >= queue->occupied)
    return NULL;

  return queue->buffer[idx];
}

#endif