#ifndef HPF_H
#define HPF_H

#include "pqueue.h"
int HPF_insertProcess(void *ds, process *p) {
  pqueue *pq = (pqueue *)(ds);
  return pqueue_enqueue(pq, p);
}

bool HPF_mustPreempt(void *ds) {
  pqueue *pq = (pqueue *)(ds);
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(void *ds) {
  pqueue *pq = (pqueue *)(ds);
  return pqueue_front(pq);
}

int HPF_removeProcess(void *ds, process *p) {
  pqueue *pq = (pqueue *)(ds);
  return pqueue_remove(pq, p);
}

int HPF_init(schedulingAlgorithm *runningAlgorithm) {
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
