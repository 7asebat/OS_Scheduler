#ifndef HPF_H
#define HPF_H

#include "headers.h"
#include "pqueue.h"

bool HPF_free = false;

int HPF_insertProcess(pqueue *pqueue, process *p) {
  return pqueue_enqueue(pqueue, p);
}

bool HPF_mustPreempt(pqueue *pqueue) {
  return false;
  //return runningProcess->status == WAITING;
}

process *HPF_getNextProcess(pqueue *pqueue) {
  return pqueue_front(pqueue);
}

int HPF_removeProcess(pqueue *pqueue, process *p) {
  return pqueue_remove(pqueue, p);
}

#endif