#ifndef RR_H
#define RR_H

#include "cqueue.h"

/**
 * Inserts a process into the data structure
 * @return -1 on failure, 0 on success
 */
int RR_insertProcess(cqueue *queue, process *p);

/**
 * Decides if the scheduler must preempt the current running process
 * 
 * The round-robin quanta could be multiple timesteps,
 * keep a counter which is decremented every time a timestep has passed,
 * until the quanta is reached.
 * @return true if the process should be preempted
 */
bool RR_mustPreempt(cqueue *queue, size_t *remainingTime, size_t quanta);

/**
 * Gets the process that should be scheduled next
 * 
 * Also dequeues the process that was previously running
 * @return NULL on failure, the address of the process on success
 */
process *RR_getNextProcess(cqueue *queue);

/**
 * Removes a process from the data structure
 * @return -1 on failure, 0 on success
 */
int RR_removeProcess(cqueue *queue, process *p);

// ==========================================================================================
int RR_insertProcess(cqueue *queue, process *p) {
  return cqueue_enqueue(queue, p);
}

bool RR_mustPreempt(cqueue *queue, size_t *remainingTime, size_t quanta) {
  if (!(*remainingTime)) {
    *remainingTime = quanta;
    return true;
  }

  (*remainingTime)--;
  return false;
}

process *RR_getNextProcess(cqueue *queue) { 
  // Dequeue current process and enqueue next process
  process *p = cqueue_dequeue(queue);
  if (!p) {
    return NULL;
  }
  cqueue_enqueue(queue, p);
  return p;
}

int RR_removeProcess(cqueue *queue, process *p) {
  process *dequeued = cqueue_backDequeue(queue);

  return dequeued ? 0 : -1;
}

#endif