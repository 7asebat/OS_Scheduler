/* Minheap implementation */
#include <stdio.h>
#define N 1000
#define L_CHILD(i) ((2 * i) + 1)
#define R_CHILD(i) ((2 * i) + 2)
#define PAR(i) ((i - 1) / 2)

typedef struct process
{
  int pid;
  int priority;
} process;

process *pqueue[N];
int size = 0;

void swap(process **l, process **r)
{
  process *temp = *l;
  *l = *r;
  *r = temp;
}

/* Return 1 if a < b or 0 otherwise */
char compare(process *a, process *b)
{
  if (a->priority < b->priority)
    return 1;
  else
    return 0;
}

process *top(process **pqueue)
{
  return pqueue[0];
}

void enqueue(process **pqueue, int *size, process *p)
{
  pqueue[*size] = p;
  (*size)++;

  // reheap up

  int i = *size - 1;
  while (i != 0 && compare(pqueue[PAR(i)], pqueue[i]))
  {
    swap(&pqueue[i], &pqueue[PAR(i)]);
    i = PAR(i);
  }
}

void dequeue(process **pqueue, int *size)
{
  (*size)--;
  pqueue[0] = pqueue[*size];

  // reheap down

  int i = 0;

  while (i < *size)
  {
    if (L_CHILD(i) < *size && R_CHILD(i) < *size)
    {
      if (compare(pqueue[L_CHILD(i)], pqueue[R_CHILD(i)]) && compare(pqueue[L_CHILD(i)], pqueue[i]))
      // if l_child < r_child , swap i with l_child
      {
        swap(&pqueue[i], &pqueue[L_CHILD(i)]);
        i = L_CHILD(i);
      }
      else if (compare(pqueue[R_CHILD(i)], pqueue[i]))
      {
        swap(&pqueue[i], &pqueue[R_CHILD(i)]);
        i = R_CHILD(i);
      }
    }
    else if (L_CHILD(i) < *size && compare(pqueue[L_CHILD(i)], pqueue[i]))
    {
      swap(&pqueue[i], &pqueue[L_CHILD(i)]);
    }
    else
    {
      break;
    }
  }
}

int main()
{
}