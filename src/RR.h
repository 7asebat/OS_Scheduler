#ifndef RR_H
#define RR_H

#include "cqueue.h"

/**
 * Inserts a process into the data structure.
 * @return -1 on failure, 0 on success
 */
int RR_insertProcess(void *ds, process *p) {
  cqueue *queue = (cqueue *)ds;
  return cqueue_enqueue(queue, p);
}

size_t RR_quanta = 2;
size_t RR_start;
/**
 * Decides if the scheduler must preempt the current running process. 
 * 
 * The round-robin quanta could be multiple timesteps, so we keep
 * a counter which is decremented every time a timestep has passed, 
 * until the quanta is reached.
 * @return true if the process should be preempted
 */
bool RR_mustPreempt(void *ds) {
  int clk = clk_get();
  if (runningProcess == NULL) {
    RR_start = clk;
    return true;
  }

  cqueue *queue = (cqueue *)ds;
  if (clk - RR_start == RR_quanta) {
    RR_start = clk;

    // Don't preempt the process if it's terminating
    // Don't preempt the process if it's the only one left
    return queue->occupied > 1 && runningProcess->remaining > 0;
  }

  return false;
}

/**
 * Gets the process that should be scheduled next. 
 * 
 * Also dequeues and reenqueues the process that was previously running.
 * @return NULL on failure, the address of the process on success
 */
process *RR_getNextProcess(void *ds) {
  cqueue *queue = (cqueue *)ds;

  if (cqueue_front(queue) == runningProcess) {
    process *p = cqueue_dequeue(queue);
    if (!p) return NULL;

    cqueue_enqueue(queue, p);
  }

  return cqueue_front(queue);
}

/**
 * Removes a process from the data structure.
 * @return -1 on failure, 0 on success
 */
int RR_removeProcess(void *ds, process *p) {
  cqueue *queue = (cqueue *)ds;

  process *dequeued = cqueue_remove(queue, p);
  return dequeued == p ? 0 : -1;
}

/**
 * Frees the resources allocated by the Round-robin algorithm.
 * @return -1 on failure, 0 on success
 */
int RR_free(void *ds) {
  cqueue *queue = (cqueue *)ds;
  return cqueue_free(queue);
}

/**
 * Assigns the Round-robin algorithm to the scheduler.
 * 
 * Also initializes the Round-robin data structure, the
 * `RR_quanta` should be pre-initialized.
 * @return -1 on failure, 0 on success
 */
int RR_init(scalgorithm *runningAlgorithm) {
  if (!runningAlgorithm) return -1;

  cqueue *queue = (cqueue *)malloc(sizeof(cqueue));
  cqueue_create(queue, DS_MAX_SIZE);

  *runningAlgorithm = (scalgorithm){
    queue,
    RR_insertProcess,
    RR_mustPreempt,
    RR_getNextProcess,
    RR_removeProcess,
    RR_free,
  };
  return 0;
}

#endif
