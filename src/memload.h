#ifndef MEMLOAD_H
#define MEMLOAD_H
#include "buddy.h"
#include "cqueue.h"
#include "headers.h"
typedef struct memload {
  cqueue* waitingQueue;
  process* (*can_load)(cqueue*);

} memload;

process* __memload_RR_canLoad(cqueue* waitingQueue) {
  process* bestFound = NULL;
  for (int i = 0; i < waitingQueue->size; i++) {
    if (buddy_tryAllocate(waitingQueue->buffer[i]->memsize) > -1) {
      bestFound = waitingQueue->buffer[i];
      break;
    }
  }

  return bestFound;
}

process* __memload_SRTN_canLoad(cqueue* waitingQueue) {
  process* bestFound = NULL;

  for (int i = 0; i < waitingQueue->size; i++) {
    if ((bestFound == NULL || (waitingQueue->buffer[i]->remaining < bestFound->remaining)) && buddy_tryAllocate(waitingQueue->buffer[i]->memsize) > -1)
      bestFound = waitingQueue->buffer[i];
  }

  if (runningProcess == NULL || bestFound != NULL && bestFound->remaining < runningProcess->remaining) {
    return bestFound;
  }
  else {
    return NULL;
  }
}

process* __memload_HPF_canLoad(cqueue* waitingQueue) {
  process* bestFound = NULL;

  for (int i = 0; i < waitingQueue->size; i++) {
    if ((bestFound == NULL || (waitingQueue->buffer[i]->priority < bestFound->priority)) && buddy_tryAllocate(waitingQueue->buffer[i]->memsize) > -1)
      bestFound = waitingQueue->buffer[i];
  }

  if (runningProcess == NULL) {
    return bestFound;
  }
  else {
    return NULL;
  }
}

void memload_init(memload* mload, scalgorithm* currentAlgorithm) {
  mload->waitingQueue = (cqueue*)malloc(sizeof(cqueue));
  cqueue_create(mload->waitingQueue, WAITING_QUEUE_SIZE);

  switch (currentAlgorithm->type) {
  case (ALGORITHM_RR):
    mload->can_load = __memload_RR_canLoad;
    break;
  case (ALGORITHM_SRTN):
    mload->can_load = __memload_SRTN_canLoad;
    break;
  case (ALGORITHM_HPF):
    mload->can_load = __memload_HPF_canLoad;
    break;
  default:
    break;
  }
}

process* memload_remove(memload* mload, process* p) {
  return cqueue_remove(mload->waitingQueue, p);
}

int memload_insert(memload* mload, process* p) {
  return cqueue_enqueue(mload->waitingQueue, p);
}

process* memload_tryLoad(memload* mload) {
  process* processToLoad = mload->can_load(mload->waitingQueue);

  if (processToLoad == NULL)
    return NULL;

  cqueue_remove(mload->waitingQueue, processToLoad);

  int memindex = buddy_allocate(processToLoad->memsize);
  processToLoad->memindex = memindex;

  return processToLoad;
}

int memload_free(memload* mload) {
  return cqueue_free(mload->waitingQueue);
}

#endif