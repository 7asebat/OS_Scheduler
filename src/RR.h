#ifndef RR_H
#define RR_H

#include "cqueue.h"

/**
 * Inserts a process into the data structure
 * @return -1 on failure, 0 on success
 */
int RR_insertProcess(void *ds, process *p) {
  cqueue *queue = (cqueue *)ds;
  return cqueue_enqueue(queue, p);
}

/**
 * Decides if the scheduler must preempt the current running process. 
 * 
 * The round-robin quanta could be multiple timesteps, so we keep
 * a counter which is decremented every time a timestep has passed,
 * until the quanta is reached.
 * @return true if the process should be preempted
 */
size_t RR_remainingTime = 0;
size_t RR_quanta = 0;
bool RR_mustPreempt(void *ds) {
  cqueue *queue = (cqueue *)ds;

  if (!RR_remainingTime) {
    RR_remainingTime = RR_quanta;
    return true;
  }

  RR_remainingTime--;
  return false;
}

/**
 * Gets the process that should be scheduled next. 
 * 
 * Also dequeues the process that was previously running
 * @return NULL on failure, the address of the process on success
 */
process *RR_getNextProcess(void *ds) {
  cqueue *queue = (cqueue *)ds;

  // Dequeue current process and enqueue next process
  process *p = cqueue_dequeue(queue);
  if (!p) return NULL;
  cqueue_enqueue(queue, p);
  return p;
}

/**
 * Removes a process from the data structure
 * @return -1 on failure, 0 on success
 */
int RR_removeProcess(void *ds, process *p) {
  cqueue *queue = (cqueue *)ds;

  process *dequeued = cqueue_backDequeue(queue);

  return dequeued ? 0 : -1;
}

/**
 * Assigns the Round-robin algorithm to the scheduler.
 * 
 * Also initializes the Round-robin data structure
 * @return -1 on failure, 0 on success
 */
int RR_init(schedulingAlgorithm *runningAlgorithm) {
  cqueue *queue = (cqueue *)malloc(sizeof(cqueue));
  cqueue_create(queue, DS_MAX_SIZE);

  schedulingAlgorithm sa = {
    queue,
    &RR_insertProcess,
    &RR_mustPreempt,
    &RR_getNextProcess,
    &RR_removeProcess,
  };

  *runningAlgorithm = sa;
  return 0;
}

#endif