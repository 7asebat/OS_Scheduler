#include <signal.h>

#include "headers.h"
#include "HPF.h"
#include "RR.h"
#include "SRTN.h"

typedef struct
{
  process **array;
  int used;
  int size;
} pcb;

pcb PCB;

schedulingAlgorithm currentAlgorithm;
FILE *pFile;

void initPCB(int initialSize) {
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

void preemptProcess(process *p) {
  if (p == NULL)
    return;

  int pid = p->pid;
  p->status = STATUS_WAITING;
  kill(pid, SIGSTOP);
  fprintf(pFile, "At time %d process %zu stopped arr %zu total %zu remain %zu wait %zu\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(pFile);

  // TODO: store context of process
}

void resumeProcess(process *p) {
  runningProcess = p;
  if (p == NULL) return;

  int pid = p->pid;
  p->status = STATUS_RUNNING;

  kill(pid, SIGCONT);

  bool started = (p->remaining == p->runtime);

  fprintf(pFile, "At time %d process %zu %s arr %zu total %zu remain %zu wait %zu\n",
          getClk(),
          p->id,
          (started) ? "started" : "resumed",
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(pFile);

  // TODO: resume context of process
}

/** Creates a stopped process **/
int createProcess(process *p) {
  int processPid = fork();

  if (processPid == 0) {
    raise(SIGSTOP);
    char pRemainingTime[10];
    sprintf(pRemainingTime, "%zu", p->remaining);
    execl("bin/process.out", "process.out", pRemainingTime, (char *)NULL);
  }

  return processPid;
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

  fprintf(pFile, "At time %d process %zu finished arr %zu total %zu remain %zu wait %zu TA %zu WTA %zu\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting,
          (size_t)0,
          (size_t)0);
  fflush(pFile);
  runningProcess = NULL;

  pcb_remove(p);
  runningProcess = NULL;

  //  signal(SIGCHLD, terminatedProcessHandler);
}

FILE *pcbLogFile;

void cleanResources(int SIGNUM) {
  fclose(pFile);
  fclose(pcbLogFile);
  signal(SIGINT, cleanResources);
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

int main(int argc, char *argv[]) {
  pcbLogFile = fopen("logs/pcb_log.txt", "w");
  pFile = fopen("logs/scheduler_log.txt", "w");
  fprintf(pFile, "Scheduler loaded\n");
  fflush(pFile);

  // signal(SIGCHLD, terminatedProcessHandler);
  struct sigaction act;

  act.sa_handler = terminatedProcessHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &act, 0) == -1) {
    perror("sigaction");
    exit(1);
  }

  signal(SIGINT, cleanResources);

  initClk();
  initPCB(100);

  int algorithm = argv[1][0] - '0';

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

  int msgqId;
  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;  // Dummy val
  msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  if (msgqId == -1) {
    perror("Error in creating message queue");
    exit(-1);
  }

  int currentClk, previousClk, msgqENO;
  previousClk = getClk();

  while (1) {
    msgqENO = msgrcv(msgqId, &msgqBuffer, sizeof(process), 1, IPC_NOWAIT);
    if (msgqENO < 0) {
      if (errno != ENOMSG) {
        perror("Error in message queue\n");
        exit(-1);
      }
    } else {
      fprintf(pFile, "Process received at clock %d, id is %d\n", currentClk, (int)msgqBuffer.p.id);
      fflush(pFile);
      int processPid = createProcess(&msgqBuffer.p);
      msgqBuffer.p.pid = processPid;

      process *pcbProcessEntry = pcb_insert(&msgqBuffer.p);
      currentAlgorithm.insertProcess(currentAlgorithm.algorithmDS, pcbProcessEntry);
    }

    currentClk = getClk();

    if (currentClk > previousClk) {
      pcb_log(pcbLogFile);

      previousClk = currentClk;

      pcb_update();

      bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

      if (mustPreempt) {
        preemptProcess(runningProcess);
        process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

        resumeProcess(nextProcess);
      }
    }
  }

  destroyClk(true);
  return 0;
}
