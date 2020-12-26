#ifndef HPF_H
#define HPF_H

#include "headers.h"
#include "pqueue.h"

bool HPF_free = false;

int HPF_insertProcess(pqueue *pqueue, process *p)
{
  return pqueue_enqueue(pqueue, p);
}

bool HPF_mustPreempt(pqueue *pqueue)
{
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(pqueue *pqueue)
{
  return pqueue_front(pqueue);
}

int HPF_removeProcess(pqueue *pqueue, process *p)
{
  return pqueue_remove(pqueue, p);
}

int HPF_init(schedulingAlgorithm *runningAlgorithm)
{
  runningAlgorithm->insertProcess = &HPF_insertProcess;
  runningAlgorithm->mustPreempt = &HPF_mustPreempt;
  runningAlgorithm->getNextProcess = &HPF_getNextProcess;
  runningAlgorithm->removeProcess = &HPF_removeProcess;

  priorityQueue *pqueue = (priorityQueue *)malloc(sizeof(priorityQueue));
  createPQueue(pqueue, 100, priorityCompare);
  runningAlgorithm->algorithmDS = pqueue;
  return 0;
}

#endif
