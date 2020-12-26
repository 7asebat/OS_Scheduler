#ifndef RR_H
#define RR_H

#include "cqueue.h"

#define RR_STATUS_WAITING 0
#define RR_STATUS_RUNNING 1

/**
 * Updates the circular queue, notifying all processes that one quanta has passed
 * @return -1 on failure, 0 on success
 */
int RR_updateQueue(cqueue *queue, size_t quanta);

/**
 * Pre-empts currently running process and places it at the back of the queue
 * @return -1 on failure, 0 on success
 */
int RR_preemptProcess(cqueue *queue);

/**
 * Schedules a process from the queue if possible
 * @return -1 on failure, 0 on success
 */
int RR_scheduleProcess(cqueue *queue);

int RR_updateQueue(cqueue *queue, size_t quanta) {
  if (!queue) return -1;
  if (!queue->buffer) return -1;
  if (!queue->occupied) return -1;

  // Increase waited time for all processes in the queue
  for (size_t i = queue->front; i != queue->back; i++) {
    if (queue->buffer[i]) {
      queue->buffer[i]->waiting += quanta;
    }
  }
  queue->buffer[queue->back]->waiting += quanta;
  return 0;
}

int RR_preemptProcess(cqueue *queue, size_t quanta) {
  process *running = cqueue_dequeue(queue);
  if (!running) return -1;

  // Reduce remaining time by a quanta
  running->remaining = running->remaining < quanta ? 0 : running->remaining - quanta;

  running->status = RR_STATUS_WAITING;
  cqueue_enqueue(queue, running);
}

int RR_scheduleProcess(cqueue *queue) {
  process *front = cqueue_front(queue);
  if (!front) return -1;

  // See if the process is starting for the first time
  if (!front -> pid) {
    front->remaining = front->runtime;
    // createProcess(front);
  }

  front->status = RR_STATUS_RUNNING;
  // See if the process has finished
}
#endif