#include "headers.h"
#include "priority_queue.h"

bool HPF_free = false;

int HPF_insertProcess(priorityQueue *pqueue, process *p)
{
  return enqueue(pqueue, p);
}

bool HPF_mustPreempt(priorityQueue *pqueue)
{
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(priorityQueue *pqueue)
{
  return top(pqueue);
}

int HPF_removeProcess(priorityQueue *pqueue, process *p)
{
  return remove(pqueue, p);
}