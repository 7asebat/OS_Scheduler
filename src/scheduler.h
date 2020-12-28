#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "HPF.h"
#include "RR.h"
#include "SRTN.h"
#include "headers.h"
#include "pcb.h"

schedulingAlgorithm currentAlgorithm;
FILE *pFile;
FILE *pcbLogFile;

void cleanResources(int SIGNUM) {
  fclose(pFile);
  fclose(pcbLogFile);
  signal(SIGINT, cleanResources);
}

void scheduler_preemptProcess(process *p) {
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

void scheduler_resumeProcess(process *p) {
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

  bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

  if (mustPreempt) {
    scheduler_preemptProcess(runningProcess);
    process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

    scheduler_resumeProcess(nextProcess);
  }

  // signal(SIGCHLD, terminatedProcessHandler);
}

/**
 * @return msqId
 */
int scheduler_init(int algorithm, int *msgqId_p, int *semid_p) {
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
  pcb_init(100);

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
  msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  if (msgqId == -1) {
    perror("Error in creating message queue");
    exit(-1);
  }

  *msgqId_p = msgqId;

  int semid = semget(SEMKEY, 1, 0666 | IPC_CREAT);

  if (semid == -1) {
    perror("error in creating semaphore");
    exit(-1);
  }

  *semid_p = semid;

  Semun semun;

  semun.val = 0; /* initial value of the semaphore, Binary semaphore */
  if (semctl(semid, 0, SETVAL, semun) == -1) {
    perror("Error in semctl");
    exit(-1);
  }

  return 0;
}

/**
 * Sees if the scheduler received a new message
 * @return -1 on failure, 0 on success.
 */
int scheduler_getMessage(int msgqId, msgBuf *msgqBuffer, int currentClk) {
  int msgqENO = msgrcv(msgqId, msgqBuffer, sizeof(process), 1, IPC_NOWAIT);
  if (msgqENO < 0) {
    if (errno != ENOMSG) {
      perror("Error in message queue\n");
      exit(-1);
    }
    return -1;
  } else
    return 0;
}

/**
 * Creates a new stopped process and logs its arrival.
 */
void scheduler_createProcess(msgBuf *msgqBuffer, int currentClk) {
  fprintf(pFile, "Process received at clock %d, id is %zu\n", currentClk, msgqBuffer->p.id);
  fflush(pFile);

  int processPid = fork();
  if (processPid == 0) {
    // raise(SIGSTOP);
    char pRemainingTime[10];
    sprintf(pRemainingTime, "%zu", msgqBuffer->p.remaining);
    execl("bin/process.out", "process.out", pRemainingTime, (char *)NULL);
  }

  kill(processPid, SIGSTOP);

  msgqBuffer->p.pid = processPid;

  process *pcbProcessEntry = pcb_insert(&msgqBuffer->p);
  currentAlgorithm.insertProcess(currentAlgorithm.algorithmDS, pcbProcessEntry);
}

#endif