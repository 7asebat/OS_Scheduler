#ifndef PCB_H
#define PCB_H

#include "headers.h"

struct
{
  process **array;
  int used;
  int size;
} PCB;

void pcb_init(int initialSize) {
  initialSize = max(1, initialSize);

  PCB.array = malloc(initialSize * sizeof(process));
  PCB.used = 0;
  PCB.size = initialSize;
}

process *pcb_getProcessByPID(int pid) {
  process *p = NULL;
  int i;
  for (i = 0; i < PCB.used; i++) {
    if (PCB.array[i]->pid == pid) {
      p = PCB.array[i];
      break;
    }
  }
  return p;
}

process *pcb_insert(process *element) {
  if (PCB.used == PCB.size) {
    PCB.size *= 2;
    PCB.array = realloc(PCB.array, PCB.size * sizeof(process *));
  }

  process *newElement = (process *)malloc(sizeof(process));
  *newElement = *element;

  PCB.array[PCB.used++] = newElement;
  return PCB.array[PCB.used - 1];
}

void pcb_remove(process *element) {
  int pidToRemove = element->pid;

  process *toBeRemoved;
  for (int i = 0; i < PCB.used; i++) {
    if (PCB.array[i]->pid == pidToRemove) {
      toBeRemoved = PCB.array[i];
      for (int j = i; j < PCB.used - 1; j++) {
        PCB.array[j] = PCB.array[j + 1];
      }
      PCB.used -= 1;
      break;
    }
  }

  free(toBeRemoved);
}

void pcb_update() {
  for (int i = 0; i < PCB.used; i++) {
    // updating running and remaining times
    if (PCB.array[i]->status == STATUS_RUNNING) {
      PCB.array[i]->remaining -= 1;
    }

    // update waiting time
    if (PCB.array[i]->status == STATUS_WAITING) {
      PCB.array[i]->waiting += 1;
    }
  }
}

void pcb_log(FILE *logFile) {
  fprintf(logFile, "---------------------------------\n");
  fprintf(logFile, "clk = %d\n", getClk());
  for (int i = 0; i < PCB.used; i++) {
    process *p = PCB.array[i];
    fprintf(logFile, "Process id: %zu \t ", p->id);
    fprintf(logFile, "Process pid: %zu \t ", p->pid);
    fprintf(logFile, "Status: %zu \t ", p->status);
    fprintf(logFile, "Priority: %zu \t ", p->priority);
    fprintf(logFile, "Arrival time: %zu \t ", p->arrival);
    fprintf(logFile, "Runtime: %zu \t ", p->runtime);
    fprintf(logFile, "Remaining time: %zu \t ", p->remaining);
    fprintf(logFile, "Waiting time: %zu \t \n", p->waiting);
    fflush(logFile);
  }
  fprintf(logFile, "---------------------------------");
  fflush(logFile);
}

#endif