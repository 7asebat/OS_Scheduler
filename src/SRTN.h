#ifndef SRTN_H
#define SRTN_H

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
bool SRTN_mustPreempt(pqueue *queue, process *running);

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

int SRTN_init(schedulingAlgorithm *runningAlgorithm);

// ==========================================================================================
int SRTN_insertProcess(pqueue *queue, process *p) {
  return pqueue_enqueue(queue, p);
}

bool SRTN_mustPreempt(pqueue *queue, process *running) {
  return pqueue_front(queue) == running;
}

process *SRTN_getNextProcess(pqueue *queue) {
  // Dequeue current process and enqueue next process
  return pqueue_front(queue);
}

int SRTN_removeProcess(pqueue *queue, process *p) {
  return pqueue_remove(queue, p);
}

int SRTN_init(schedulingAlgorithm *runningAlgorithm) {
  pqueue *queue = (pqueue *)malloc(sizeof(pqueue));
  pqueue_create(queue, 100, pqueue_pcompare);

  schedulingAlgorithm sa = {
    queue,
    &HPF_insertProcess,
    &HPF_mustPreempt,
    &HPF_getNextProcess,
    &HPF_removeProcess,
  };

  *runningAlgorithm = sa;
  return 0;
}

#endif
