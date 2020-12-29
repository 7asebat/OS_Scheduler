#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "HPF.h"
#include "RR.h"
#include "SRTN.h"
#include "headers.h"
#include "pcb.h"

scalgorithm currentAlgorithm;
FILE *log_scheduler;

void scheduler_checkContextSwitch();

/**
 * Cleans up the resources allocated by the scheduler, called prior to program exit.
 */
void scheduler_cleanup(int SIGNUM) {
  fclose(log_scheduler);
  pcb_free();

  int msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  msgctl(msgqId, IPC_RMID, (struct msqid_ds *)0);

  clk_destroy(false);

  signal(SIGINT, scheduler_cleanup);

  currentAlgorithm.free(currentAlgorithm.ds);
}

/**
 * Preempts the current running process.
 */
void scheduler_preemptProcess(process *p) {
  if (p == NULL) return;

  p->status = STATUS_WAITING;
  kill(p->pid, SIGTSTP);
  fprintf(log_scheduler, "At time %d process %zu stopped arr %zu total %zu remain %zu wait %zu\n",
          clk_get(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(log_scheduler);
}

/**
 * Resumes a given process
 */
void scheduler_resumeProcess(process *p) {
  runningProcess = p;
  if (p == NULL) return;

  int pid = p->pid;
  p->status = STATUS_RUNNING;

  kill(pid, SIGCONT);

  bool started = (p->remaining == p->runtime);

  fprintf(log_scheduler, "At time %d process %zu %s arr %zu total %zu remain %zu wait %zu\n",
          clk_get(),
          p->id,
          (started) ? "started" : "resumed",
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(log_scheduler);
}

/**
 * Is called whenever a process terminates and sends a signal to the scheduler. 
 * Removes the process from the data structure and the process control block, 
 * frees the memory allocated by it, and checks if context switching is necessary.
 */
void scheduler_processTerminationHandler(int SIGNUM) {
  int process_status;

  int exitedProcessPid = wait(&process_status);
  if (WIFEXITED(process_status)) {
    int exit_code = WEXITSTATUS(process_status);
    printf("process %d: exited with exit code %d\n", exitedProcessPid, exit_code);
  }
  process *p = pcb_getProcessByPID(exitedProcessPid);

  currentAlgorithm.removeProcess(currentAlgorithm.ds, p);

  size_t TA = clk_get() - p->arrival;
  double WTA = TA / (double)p->runtime;

  fprintf(log_scheduler, "At time %d process %zu finished arr %zu total %zu remain %zu wait %zu TA %zu WTA %.2f\n",
          clk_get(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting,
          TA,
          WTA);
  fflush(log_scheduler);

  runningProcess = NULL;
  pcb_remove(p);
  scheduler_checkContextSwitch();
}

/**
 * Performs context switching if necessary, preempting the current process, 
 * and resuming the next process in the ready queue, which is determined by 
 * the scheduling algorithm.
 */
void scheduler_checkContextSwitch() {
  bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.ds);

  if (mustPreempt) {
    scheduler_preemptProcess(runningProcess);
    process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.ds);

    scheduler_resumeProcess(nextProcess);
  }
}

/**
 * Initializes the scheduler and all its components
 * @return msqId
 */
int scheduler_init(int algorithm, int *msgqId_p) {
  log_scheduler = fopen("logs/scheduler.log", "w");

  struct sigaction act;
  act.sa_handler = scheduler_processTerminationHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &act, 0) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Latch the cleanup handler
  signal(SIGINT, scheduler_cleanup);

  clk_init();
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
  fprintf(log_scheduler, "Process received at clock %d, id is %zu\n", clk_get(), msgqBuffer->p.id);
  fflush(log_scheduler);

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
  currentAlgorithm.insertProcess(currentAlgorithm.ds, pcbProcessEntry);
}

#endif
