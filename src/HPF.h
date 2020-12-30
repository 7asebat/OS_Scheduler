#ifndef HPF_H
#define HPF_H
#include "pqueue.h"

int HPF_insertProcess(void *ds, process *p) {
  pqueue *pq = (pqueue *)(ds);
  return pqueue_enqueue(pq, p);
}

bool HPF_mustPreempt(void *ds) {
  pqueue *pq = (pqueue *)(ds);
  return (runningProcess == NULL) || (runningProcess->status == STATUS_WAITING);
}

process *HPF_getNextProcess(void *ds) {
  pqueue *pq = (pqueue *)(ds);
  return pqueue_front(pq);
}

int HPF_removeProcess(void *ds, process *p) {
  pqueue *pq = (pqueue *)(ds);
  process *dequeued = pqueue_remove(pq, p);
  return dequeued == p ? 0 : -1;
}

/**
 * Frees the resources allocated by the HPF algorithm.
 * @return -1 on failure, 0 on success
 */
int HPF_free(void *ds) {
  pqueue *queue = (pqueue *)ds;
  return pqueue_free(queue);
}

int HPF_init(scalgorithm *runningAlgorithm) {
  if (!runningAlgorithm) return -1;

  pqueue *queue = (pqueue *)malloc(sizeof(pqueue));
  pqueue_create(queue, DS_MAX_SIZE, pqueue_pcompare);

  *runningAlgorithm = (scalgorithm){
    queue,
    HPF_insertProcess,
    HPF_mustPreempt,
    HPF_getNextProcess,
    HPF_removeProcess,
    HPF_free,
  };

  return 0;
}
#endif
