#include "SRTN.h"
#include "cqueue.h"
#include "headers.h"
#include "pqueue.h"

void __startq_RR_init(startq* sq) {
  cqueue* cq = (cqueue*)malloc(sizeof(cqueue));
  cqueue_create(cq, DS_MAX_SIZE);

  sq->ds = cq;
  sq->enqueue = (int (*)(void*, process*))(cqueue_enqueue);
  sq->dequeue = (process * (*)(void*))(cqueue_dequeue);
  sq->remove = (process * (*)(void*, process*))(cqueue_remove);
  sq->getElemAt = (process * (*)(void*, int))(cqueue_getElemAt);
  sq->get_size = (size_t(*)(void*))(cqueue_get_size);
}

void __startq_HPF_init(startq* sq) {
  pqueue* pq = (pqueue*)malloc(sizeof(pqueue));
  pqueue_create(pq, DS_MAX_SIZE, pqueue_pcompare);
  sq->ds = pq;
  sq->enqueue = (int (*)(void*, process*))(pqueue_enqueue);
  sq->dequeue = (process * (*)(void*))(pqueue_dequeue);
  sq->remove = (process * (*)(void*, process*))(pqueue_remove);
  sq->getElemAt = (process * (*)(void*, int))(pqueue_getElemAt);
  sq->get_size = (size_t(*)(void*))(pqueue_get_size);
}

void __startq_SRTN_init(startq* sq) {
  pqueue* pq = (pqueue*)malloc(sizeof(pqueue));
  pqueue_create(pq, DS_MAX_SIZE, &SRTN_compare);

  sq->ds = pq;
  sq->enqueue = (int (*)(void*, process*))(pqueue_enqueue);
  sq->dequeue = (process * (*)(void*))(pqueue_dequeue);
  sq->remove = (process * (*)(void*, process*))(pqueue_remove);
  sq->getElemAt = (process * (*)(void*, int))(pqueue_getElemAt);
  sq->get_size = (size_t(*)(void*))(pqueue_get_size);
}

void startq_init(int algorithm, startq* sq) {
  switch (algorithm) {
  case ALGORITHM_RR:
    __startq_RR_init(sq);
    break;
  case ALGORITHM_HPF:
    __startq_HPF_init(sq);
    break;
  case ALGORITHM_SRTN:
    __startq_SRTN_init(sq);
    break;
  default:
    break;
  }
}
