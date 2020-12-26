#ifndef HPF_H
#define HPF_H

#include "headers.h"
#include "pqueue.h"

bool HPF_free = false;

int HPF_insertProcess(void *ds, process *p)
{
  pqueue *pq = (pqueue *)(ds);
  return pqueue_enqueue(pq, p);
}

bool HPF_mustPreempt(void *ds)
{
  pqueue *pq = (pqueue *)(ds);
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(void *ds)
{
  pqueue *pq = (pqueue *)(ds);
  return pqueue_front(pq);
}

int HPF_removeProcess(void *ds, process *p)
{
  pqueue *pq = (pqueue *)(ds);
  return pqueue_remove(pq, p);
}

int HPF_init(schedulingAlgorithm *runningAlgorithm)
{
  runningAlgorithm->insertProcess = &HPF_insertProcess;
  runningAlgorithm->mustPreempt = &HPF_mustPreempt;
  runningAlgorithm->getNextProcess = &HPF_getNextProcess;
  runningAlgorithm->removeProcess = &HPF_removeProcess;

  pqueue *pq = (pqueue *)malloc(sizeof(pqueue));
  pqueue_create(pq, 100, pqueue_pcompare);
  runningAlgorithm->algorithmDS = pq;
  return 0;
}

#endif
