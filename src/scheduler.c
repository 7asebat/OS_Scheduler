#include <signal.h>

#include "headers.h"

typedef struct
{
  process *array;
  int used;
  int size;
} pcb;

pcb PCB;
process *runningProcess;
schedulingAlgorithm currentAlgorithm;

void initPCB(int initialSize) {
  PCB.array = malloc(initialSize * sizeof(process));
  PCB.used = 0;
  PCB.size = initialSize;
}

process *pcb_getProcessByPID(int pid) {
  process *p = NULL;
  int i;
  for (i = 0; i < PCB.used; i++) {
    if (PCB.array[i].pid == pid) {
      break;
    }
  }
  return p;
}

void pcb_insert(process *element) {
  if (PCB.used == PCB.size) {
    PCB.size *= 2;
    PCB.array = realloc(PCB.array, PCB.size * sizeof(process));
  }
  element->PCB_idx = PCB.used;
  PCB.array[PCB.used++] = *element;
}

void pcb_remove(process element) {
  int pidToRemove = element.pid;

  for (int j = element.PCB_idx; j < PCB.used - 1; j++) {
    PCB.array[j] = PCB.array[j + 1];
  }
  PCB.used -= 1;
}

void pcb_update() {
  for (int i = 0; i < PCB.used; i++) {
    // updating running and remaining times
    if (PCB.array[i].status == STATUS_RUNNING) {
      PCB.array[i].remaining -= 1;
      PCB.array[i].runtime += 1;
    }

    // update waiting time
    if (PCB.array[i].status == STATUS_WAITING) {
      PCB.array[i].waiting += 1;
    }
  }
}

void preemptProcess(process *p) {
  int PCB_idx = p->PCB_idx;
  int pid = PCB.array[PCB_idx].pid;
  PCB.array[PCB_idx].status = STATUS_WAITING;
  kill(pid, SIGSTOP);
  // TODO: save context of process
}

void resumeProcess(process *p) {
  int PCB_idx = p->PCB_idx;
  int pid = PCB.array[PCB_idx].pid;
  PCB.array[PCB_idx].status = STATUS_RUNNING;
  runningProcess = p;
  kill(pid, SIGCONT);
  // TODO: resume context of process
}

void terminatedProcessHandler(int SIGNUM) {
  int process_status;
  int exitedProcessPid = wait(&process_status);
  if (WIFEXITED(process_status)) {
    int exit_code = WEXITSTATUS(process_status);
    printf("process %d: exited with exit code %d\n", exitedProcessPid, exit_code);
  }

  process *p = pcb_getProcessByPID(exitedProcessPid);

  currentAlgorithm.removeProcess(currentAlgorithm.algorithmDS, p);

  pcb_remove(*p);

  signal(SIGCHLD, terminatedProcessHandler);
}

int main(int argc, char *argv[]) {
  signal(SIGCHLD, terminatedProcessHandler);

  int algorithm = ALGORITHM_RR;

  switch (algorithm) {
    case ALGORITHM_RR:
      RR_init(&currentAlgorithm);
      break;
    case ALGORITHM_HPF:
      HPF_init(&currentAlgorithm);
      break;
    case ALGORITHM_SRTN:
      SRTN_init(&currentAlgorithm);
      break;
    default:
      break;
  }

  initClk();

  int clkProcess = fork();
  if (clkProcess == 0) {
    execl("clk.out", NULL);
  }

  int currentClk, previousClk;
  previousClk = getClk();
  while (1) {
    // receive processes from message queue
    // update PCB
    // insert new processes into the datastructure
    // tell the algorithm to insert the process

    currentClk = getClk();
    if (currentClk > previousClk) {
      previousClk = currentClk;

      bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);
      if (mustPreempt) {
        preemptProcess(runningProcess);
        process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);
        resumeProcess(nextProcess);
      }
    }
  }

  destroyClk(true);
}