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
 * Gets the process that should be scheduled next
 * @return NULL on failure, the address of the process on success
 */
process *SRTN_getNextProcess(void *ds) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_front(queue);
}

/**
 * Removes a process from the data structure
 * @return -1 on failure, 0 on success
 */
int SRTN_removeProcess(void *ds, process *p) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_remove(queue, p);
}

/**
 * Compares a, b and returns true if a should come before b. 
 * The comparison criteria is the remaining time
 */
bool SRTN_compare(process *a, process *b) {
  if (a->remaining < b->remaining)  // a < b if a.priority > b.priority
    return true;
  else
    return false;
}

/**
 * Assigns the pre-emptive Shortest Remaining Time Next algorithm
 * to the scheduler. 
 * 
 * Also initializes the SRTN data structure
 * @note SRTN_compare or &SRTN_compare?
 * @return -1 on failure, 0 on success
 */
int SRTN_init(schedulingAlgorithm *runningAlgorithm) {
  pqueue *queue = (pqueue *)malloc(sizeof(pqueue));
  pqueue_create(queue, DS_MAX_SIZE, SRTN_compare);

  schedulingAlgorithm sa = {
    queue,
    &SRTN_insertProcess,
    &SRTN_mustPreempt,
    &SRTN_getNextProcess,
    &SRTN_removeProcess,
  };

  *runningAlgorithm = sa;
  return 0;
}

#endif
