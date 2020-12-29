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
FILE *schLog;  // DEBUG

void scheduler_checkContextSwitch();

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
  kill(pid, SIGTSTP);
  fprintf(pFile, "At time %d process %zu stopped arr %zu total %zu remain %zu wait %zu\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(pFile);

  fprintf(schLog, "[%d]\t%zu STOP  \tREM (%zu)\tBURST (%zu)\n",
          getClk(), p->id, p->remaining, p->runtime);
  fflush(schLog);

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

  fprintf(schLog, "[%d]\t%zu %s\tREM (%zu)\tBURST (%zu)\n",
          getClk(), p->id, started ? "START " : "RESUME", p->remaining, p->runtime);
  fflush(schLog);

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

  size_t TA = getClk() - p->arrival;
  double WTA = TA / (double)p->runtime;

  fprintf(pFile, "At time %d process %zu finished arr %zu total %zu remain %zu wait %zu TA %zu WTA %.2f\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting,
          TA,
          WTA);
  fflush(pFile);

  // DEBUG
  fprintf(schLog, "[%d]\t%zu FINISH\tREM (%zu)\tBURST (%zu)\n",
          getClk(), p->id, p->remaining, p->runtime);
  fflush(schLog);

  runningProcess = NULL;

  pcb_remove(p);

  scheduler_checkContextSwitch();
}

void scheduler_checkContextSwitch() {
  bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

  if (mustPreempt) {
    scheduler_preemptProcess(runningProcess);
    process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

    scheduler_resumeProcess(nextProcess);
  }
}

/**
 * @return msqId
 */
int scheduler_init(int algorithm, int *msgqId_p) {
  pcbLogFile = fopen("logs/pcb.log", "w");
  pFile = fopen("logs/scheduler.log", "w");
  fprintf(pFile, "Scheduler loaded\n");
  fflush(pFile);

  // DEBUG
  schLog = fopen("./logs/sch.log", "w");
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

  return 0;
}

/**
 * Sees if the scheduler received a new message.
 * @return -1 on failure, 0 on success.
 */
int scheduler_getMessage(int msgqId, msgBuf *msgqBuffer) {
  int msgqENO = msgrcv(msgqId, msgqBuffer, sizeof(process), 1, IPC_NOWAIT);
  if (msgqENO < 0) {
    if (errno != ENOMSG) {
      perror("Error in message queue\n");
      exit(-1);
    }
    return -1;
  } else {
    return 0;
  }
}

/**
 * Creates a new stopped process and logs its arrival.
 */
void scheduler_createProcess(msgBuf *msgqBuffer) {
  fprintf(pFile, "Process received at clock %d, id is %zu\n", getClk(), msgqBuffer->p.id);
  fflush(pFile);

  fprintf(schLog, "[%d]\t%zu ARRIVE\n",
          getClk(), msgqBuffer->p.id);
  fflush(schLog);

  int processPid = fork();
  if (processPid == 0) {
    // raise(SIGSTOP);
    char pRemainingTime[10];
    sprintf(pRemainingTime, "%zu", msgqBuffer->p.remaining);
    execl("bin/process.out", "process.out", pRemainingTime, (char *)NULL);
  }

  kill(processPid, SIGTSTP);

  msgqBuffer->p.pid = processPid;

  process *pcbProcessEntry = pcb_insert(&msgqBuffer->p);
  currentAlgorithm.insertProcess(currentAlgorithm.algorithmDS, pcbProcessEntry);
}

#endif
