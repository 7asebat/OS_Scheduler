#include "headers.h"
#include "priority_queue.h"

int HPF_insertProcess(void *ds, process *p)
{
  priorityQueue *pqueue = (priorityQueue *)(ds);
  return enqueue(pqueue, p);
}

bool HPF_mustPreempt(void *ds)
{
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(void *ds)
{
  priorityQueue *pqueue = (priorityQueue *)(ds);
  return top(pqueue);
}

int HPF_removeProcess(void *ds, process *p)
{
  priorityQueue *pqueue = (priorityQueue *)(ds);
  return remove(pqueue, p);
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