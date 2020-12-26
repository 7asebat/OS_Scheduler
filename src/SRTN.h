#ifndef RR_H
#define RR_H

#include "pqueue.h"

/**
 * Inserts a process into the data structure
 * @return -1 on failure, 0 on success
 */
int SRTN_insertProcess(pqueue *queue, process *p);

/**
 * Decides if the scheduler must preempt the current running process
 * 
 * If the top of the queue does not match the current running process
 * @return true if the process should be preempted
 */
bool SRTN_mustPreempt(pqueue *queue);

/**
 * Gets the process that should be scheduled next
 * 
 * Also dequeues the process that was previously running
 * @return NULL on failure, the address of the process on success
 */
process *SRTN_getNextProcess(pqueue *queue);

/**
 * Removes a process from the data structure
 * @return -1 on failure, 0 on success
 */
int SRTN_removeProcess(pqueue *queue, process *p);

// ==========================================================================================
int SRTN_insertProcess(pqueue *queue, process *p) {
  return pqueue_enqueue(queue, p);
}

bool SRTN_mustPreempt(pqueue *queue, process *running) {
  return pqueue_front(queue) == running;
}

process *SRTN_getNextProcess(pqueue *queue) { 
  // Dequeue current process and enqueue next process
  process *p = pqueue_dequeue(queue);
  if (!p) {
    return NULL;
  }
  pqueue_enqueue(queue, p);
  return p;
}

int SRTN_removeProcess(pqueue *queue, process *p) {
  return pqueue_remove(queue, p);
}

#endif
