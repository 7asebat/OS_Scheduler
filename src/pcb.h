#ifndef PCB_H
#define PCB_H

#include "headers.h"

struct
{
  int used;
  int size;
  process **array;
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

/**
 * This function also frees the memory allocated for the process.
 */
void pcb_remove(process *p) {
  int pidToRemove = p->pid;

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

void pcb_free() {
  for (int i = 0; i < PCB.used; i++)
    free(PCB.array[i]);

  free(PCB.array);
}

void pcb_log(FILE *logFile) {
  // Previous clock has finished
  fprintf(logFile, "------------------------------------------------------------------\n[%d]\n", clk_get()-1);
  for (int i = 0; i < PCB.used; i++) {
    process *p = PCB.array[i];
    fprintf(logFile, "ID\t%zu\t", p->id);
    fprintf(logFile, "PID\t%zu\t", p->pid);
    fprintf(logFile, "RUN\t%zu\t", p->status);
    fprintf(logFile, "PRIO\t%zu\t", p->priority);
    fprintf(logFile, "ARR\t%zu\t", p->arrival);
    fprintf(logFile, "BURST\t%zu\t", p->runtime);
    fprintf(logFile, "REM\t%zu\t", p->remaining);
    fprintf(logFile, "WAIT\t%zu\t\n", p->waiting);
    fflush(logFile);
  }
  fprintf(logFile, "\n------------------------------------------------------------------");
  fflush(logFile);
}

#endif
