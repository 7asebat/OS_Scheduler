#ifndef PQUEUE_H
#define PQUEUE_H

#include "headers.h"

#define L_CHILD(i) ((2 * i) + 1)
#define R_CHILD(i) ((2 * i) + 2)
#define PAR(i) ((i - 1) / 2)

typedef struct pqueue {
  size_t capacity;
  size_t size;

  bool (*compare)(process *, process *);
  process **buffer;
} pqueue;

pqueue PRIORITY_QUEUE_DEFAULT = {0, 0, NULL, NULL};

void pqueue_print(pqueue *pq) {
  for (int i = 0; i < pq->size; i++) {
    printf("%zu ", pq->buffer[i]->remaining);
  }
  printf("\n");
}

/**
 * Swaps two processes passed in by pointer
 * @param l first process
 * @param r second process
 * @return void
 */
void __pqueue_swap(process **l, process **r) {
  process *temp = *l;
  *l = *r;
  *r = temp;
}

/**
 * Returns comparison of two processes a and b according to priority
 * Used in priority queue to inclusively sort
 * @param a First process for comparison
 * @param b Second process for comparison
 * @return{char} returns comparison result, 1 if a < b, 0 elsewise
 */
bool pqueue_pcompare(process *a, process *b) {
  if (a->priority < b->priority)  // a < b if a.priority > b.priority
    return true;
  else
    return false;
}

/**
 * Allocates a buffer in the priority queue
 * @return -1 on failure, 0 on success
 */
int pqueue_create(pqueue *queue, size_t capacity, bool (*compare)(process *, process *)) {
  if (!queue || !capacity) return -1;
  if (queue->buffer) return -1;

  if (compare != NULL)
    queue->compare = compare;
  else
    queue->compare = &pqueue_pcompare;

  queue->size = 0;
  queue->capacity = capacity;
  queue->buffer = (process **)calloc(queue->capacity, sizeof(process *));
  return 0;
}

/**
 * Returns top of the queue
 * @param{PriorityQueue*} Priority queue object
 * @return{process*} The process at the top of the queue
 */
process *pqueue_front(pqueue *queue) {
  if (queue->size == 0)
    return NULL;
  return queue->buffer[0];
}

void __pqueue_reheapUp(pqueue *queue) {
  size_t *size = &queue->size;

  int i = *size - 1;
  while (i != 0 && queue->compare(queue->buffer[i], queue->buffer[PAR(i)])) {
    __pqueue_swap(&queue->buffer[i], &queue->buffer[PAR(i)]);
    i = PAR(i);
  }
}

void __pqueue_reheapDown(pqueue *queue, int i) {
  size_t *size = &queue->size;

  bool leftChildCan = L_CHILD(i) < *size && queue->compare(queue->buffer[L_CHILD(i)], queue->buffer[i]);
  bool rightChildCan = R_CHILD(i) < *size && queue->compare(queue->buffer[R_CHILD(i)], queue->buffer[i]);

  while (i < *size && (leftChildCan || rightChildCan)) {
    if (leftChildCan && rightChildCan) {
      if (queue->compare(queue->buffer[L_CHILD(i)], queue->buffer[R_CHILD(i)])) {
        __pqueue_swap(&queue->buffer[i], &queue->buffer[L_CHILD(i)]);
        i = L_CHILD(i);
      } else {
        __pqueue_swap(&queue->buffer[i], &queue->buffer[R_CHILD(i)]);
        i = R_CHILD(i);
      }
    } else if (leftChildCan) {
      __pqueue_swap(&queue->buffer[i], &queue->buffer[L_CHILD(i)]);
      i = L_CHILD(i);
    } else {
      __pqueue_swap(&queue->buffer[i], &queue->buffer[R_CHILD(i)]);
      i = R_CHILD(i);
    }

    leftChildCan = L_CHILD(i) < *size && queue->compare(queue->buffer[L_CHILD(i)], queue->buffer[i]);
    rightChildCan = R_CHILD(i) < *size && queue->compare(queue->buffer[R_CHILD(i)], queue->buffer[i]);
  }
}

/**
 * Enqueues an element inside the queue
 * @param{pqueue*} Priority queue object
 * @param{process*} The process to be inserted
 * @return 0 on success, -1 on failure
 */
int pqueue_enqueue(pqueue *queue, process *p) {
  if (queue->capacity == queue->size)
    return -1;
  size_t *size = &queue->size;

  queue->buffer[*size] = p;
  (*size)++;

  __pqueue_reheapUp(queue);

  return 0;
}

/**
 * Dequeues an element from the top of the queue
 * @param{pqueue*} Priority queue object
 * @return 0 on success, -1 if the queue is empty
 */
process *pqueue_dequeue(pqueue *queue) {
  if (!queue) return NULL;
  if (queue->size == 0) return NULL;

  size_t *size = &queue->size;

  (*size)--;
  queue->buffer[0] = queue->buffer[*size];

  // reheap down
  __pqueue_reheapDown(queue, 0);

  return queue->buffer[queue->size];
}

int pqueue_remove(pqueue *queue, process *process) {
  int i;
  size_t *size = &queue->size;

  for (i = 0; i < *size; i++) {
    if (queue->buffer[i] == process) {
      __pqueue_swap(&queue->buffer[i], &queue->buffer[*size - 1]);
      (*size)--;
      __pqueue_reheapDown(queue, i);
      break;
      return 0;
    }
  }

  return -1;
}

/**
 * Frees the buffer allocated by the queue and resets data.
 * @return -1 on failure, 0 on success.
 */
int pqueue_free(pqueue *queue) {
  if (!queue)
    return -1;
  if (!queue->buffer)
    return -1;

  free(queue->buffer);
  return 0;
}

void pqueue_log(pqueue *queue, FILE *pFile) {
  fprintf(pFile, "---------------------------------\n");
  fprintf(pFile, "clk = %d\n", getClk());
  for (int i = 0; i < queue->size; i++) {
    fprintf(pFile, "{id= %zu, remaining= %zu, priority= %zu} ", queue->buffer[i]->id, queue->buffer[i]->remaining, queue->buffer[i]->priority);
    fprintf(pFile, "\n");
  }
  fprintf(pFile, "---------------------------------");
  fflush(pFile);
}
#endif
