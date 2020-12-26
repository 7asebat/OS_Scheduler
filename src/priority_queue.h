#ifndef HEADERS_H
#include "headers.h"
#endif

#define L_CHILD(i) ((2 * i) + 1)
#define R_CHILD(i) ((2 * i) + 2)
#define PAR(i) ((i - 1) / 2)

typedef struct
{
  int pid;
  int priority;
} process;

typedef struct
{
  size_t capacity;
  size_t size;

  char (*compare)(process *, process *);
  process **buffer;
} priorityQueue;

priorityQueue PRIORITY_QUEUE_DEFAULT = {0, 0, NULL};

/**
 * Swaps two processes passed in by pointer
 * @param l first process
 * @param r second process
 * @return void
 */
void swap(process **l, process **r)
{
  process *temp = *l;
  *l = *r;
  *r = temp;
}

/**
 * Returns comparison of two processes a and b according to priority
 * Used in priority queue to inclusively sort
 * @param a First process for comparison
 * @param b Second process for comparison
 * @return{char} returns comparison result, 0 if a < b, 1 elsewise
 */
char priorityCompare(process *a, process *b)
{
  if (a->priority < b->priority)
    return 1;
  else
    return 0;
}

/**
 * Allocates a buffer in the priority queue
 * @return -1 on failure, 0 on success
 */
int createPQueue(priorityQueue *pqueue, size_t capacity, char (*compare)(process *, process *))
{
  if (!pqueue || !capacity)
    return -1;
  if (pqueue->buffer)
    return -1;

  if (compare != NULL)
    pqueue->compare = compare;
  else
    pqueue->compare = &priorityCompare;

  pqueue->size = 0;
  pqueue->capacity = capacity;
  pqueue->buffer = (process **)calloc(pqueue->capacity, sizeof(process *));
  return 0;
}

/**
 * Returns top of the queue
 * @param{PriorityQueue*} Priority queue object
 * @return{process*} The process at the top of the queue
 */
process *top(priorityQueue *pqueue)
{
  return pqueue->buffer[0];
}

/**
 * Enqueues an element inside the queue
 * @param{priorityQueue*} Priority queue object
 * @param{process*} The process to be inserted
 * @return 0 on success, -1 on failure
 */
int enqueue(priorityQueue *pqueue, process *p)
{
  if (pqueue->capacity == pqueue->size)
    return -1;
  size_t *size = &pqueue->size;
  pqueue->buffer[*size] = p;
  (*size)++;

  // reheap up

  int i = *size - 1;
  while (i != 0 && pqueue->compare(pqueue->buffer[PAR(i)], pqueue->buffer[i]))
  {
    swap(&pqueue->buffer[i], &pqueue->buffer[PAR(i)]);
    i = PAR(i);
  }
  return 0;
}

/**
 * Dequeues an element from the top of the queue
 * @param{priorityQueue*} Priority queue object
 * @return 0 on success, -1 if the queue is empty
 */
int dequeue(priorityQueue *pqueue)
{
  if (pqueue->size == 0)
    return -1;

  size_t *size = &pqueue->size;

  (*size)--;
  pqueue->buffer[0] = pqueue->buffer[*size];

  // reheap down

  int i = 0;

  while (i < *size)
  {
    if (L_CHILD(i) < *size && R_CHILD(i) < *size)
    {
      if (pqueue->compare(pqueue->buffer[L_CHILD(i)], pqueue->buffer[R_CHILD(i)]) && pqueue->compare(pqueue->buffer[L_CHILD(i)], pqueue->buffer[i]))
      // if l_child < r_child , swap i with l_child
      {
        swap(&pqueue->buffer[i], &pqueue->buffer[L_CHILD(i)]);
        i = L_CHILD(i);
      }
      else if (pqueue->compare(pqueue->buffer[R_CHILD(i)], pqueue->buffer[i]))
      {
        swap(&pqueue->buffer[i], &pqueue->buffer[R_CHILD(i)]);
        i = R_CHILD(i);
      }
    }
    else if (L_CHILD(i) < *size && pqueue->compare(pqueue->buffer[L_CHILD(i)], pqueue->buffer[i]))
    {
      swap(&pqueue->buffer[i], &pqueue->buffer[L_CHILD(i)]);
    }
    else
    {
      break;
    }
  }

  return 0;
}