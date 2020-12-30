#ifndef SRTN_H
#define SRTN_H

#include "pqueue.h"

/**
 * Inserts a process into the data structure
 * @return -1 on failure, 0 on success
 */
int SRTN_insertProcess(void *ds, process *p) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_enqueue(queue, p);
}

/**
 * Decides if the scheduler must preempt the current running process, 
 * 
 * if the top of the queue does not match the current running process
 * @return true if the process should be preempted
 */
bool SRTN_mustPreempt(void *ds) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_front(queue) != runningProcess;
}

/**
 * Gets the process that should be scheduled next.
 * @return NULL on failure, the address of the process on success.
 */
process *SRTN_getNextProcess(void *ds) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_front(queue);
}

/**
 * Removes a process from the data structure.
 * @return -1 on failure, 0 on success.
 */
int SRTN_removeProcess(void *ds, process *p) {
  pqueue *queue = (pqueue *)ds;
  process *dequeued = pqueue_remove(queue, p);
  return dequeued == p ? 0 : -1;
}

/**
 * Priority queue comparator function for the SRTN algorithm.
 * The comparison criteria is the remaining time.
 * @return true if a should come before b.
 */
bool SRTN_compare(process *a, process *b) {
  return a->remaining < b->remaining;  // a < b if a.priority > b.priority
}

/**
 * Frees the resources allocated by the SRTN algorithm.
 * @return -1 on failure, 0 on success
 */
int SRTN_free(void *ds) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_free(queue);
}

/**
 * Assigns the pre-emptive Shortest Remaining Time Next algorithm
 * to the scheduler. 
 * 
 * Also initializes the SRTN data structure.
 * @return -1 on failure, 0 on success
 */
int SRTN_init(scalgorithm *runningAlgorithm) {
  if (!runningAlgorithm) return -1;

  pqueue *queue = (pqueue *)malloc(sizeof(pqueue));
  pqueue_create(queue, DS_MAX_SIZE, &SRTN_compare);

  *runningAlgorithm = (scalgorithm){
    queue,
    SRTN_insertProcess,
    SRTN_mustPreempt,
    SRTN_getNextProcess,
    SRTN_removeProcess,
    SRTN_free,
  };

  return 0;
}
#endif
