#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "HPF.h"
#include "RR.h"
#include "SRTN.h"
#include "buddy.h"
#include "headers.h"
#include "memload.h"
#include "pcb.h"

scalgorithm currentAlgorithm;
memload mload;
FILE *log_scheduler;
FILE *log_memory;

size_t numberOfProccesses = INT_MAX;

size_t stat_lastUtilizationTimestep;
size_t stat_idle = 0;

double stat_nvarWTA = 0;
double stat_meanWTA = 0;
size_t stat_waited = 0;
size_t stat_n = 0;

void scheduler_checkContextSwitch();

/**
 * Cleans up the resources allocated by the scheduler, called prior to program exit.
 */
void scheduler_cleanup(int SIGNUM) {
  fclose(log_scheduler);
  fclose(log_memory);

  // Log stats
  log_scheduler = fopen("logs/scheduler.perf", "w");
  double stat_utilization = 1 - ((double)stat_idle / clk_get());
  double stat_avgWaited = (double)stat_waited / stat_n;
  double stat_stdWTA = sqrt(stat_nvarWTA / stat_n);
  fprintf(log_scheduler, "CPU utilization = %.2f%%\n", stat_utilization * 100);
  fprintf(log_scheduler, "Avg WTA = %.2f\n", stat_meanWTA);
  fprintf(log_scheduler, "Avg Waiting = %.2f\n", stat_avgWaited);
  fprintf(log_scheduler, "Std WTA = %.2f\n", stat_stdWTA);
  fflush(log_scheduler);
  fclose(log_scheduler);

  pcb_free();

  currentAlgorithm.free(currentAlgorithm.ds);
  memload_free(&mload);

  int msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  msgctl(msgqId, IPC_RMID, (struct msqid_ds *)0);

  clk_destroy(false);
  signal(SIGINT, scheduler_cleanup);
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

  stat_lastUtilizationTimestep = clk_get();
}

/**
 * Resumes a given process
 */
void scheduler_resumeProcess(process *p) {
  runningProcess = p;
  if (p == NULL) return;

  int currentClk = clk_get();

  p->status = STATUS_RUNNING;
  kill(p->pid, SIGCONT);

  bool started = (p->remaining == p->runtime);
  fprintf(log_scheduler, "At time %d process %zu %s arr %zu total %zu remain %zu wait %zu\n",
          currentClk,
          p->id,
          (started) ? "started" : "resumed",
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(log_scheduler);

  stat_idle += currentClk - stat_lastUtilizationTimestep;
  stat_lastUtilizationTimestep = currentClk;
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
    int exitCode = WEXITSTATUS(process_status);
    if (exitCode) {
      // Handle error
    }
  }

  int currentClk = clk_get();

  process *p = pcb_getProcessByPID(exitedProcessPid);
  currentAlgorithm.removeProcess(currentAlgorithm.ds, p);

  stat_n++;
  size_t TA = currentClk - p->arrival;
  double WTA = TA / (double)p->runtime;

  double meanWTA = stat_meanWTA;
  stat_meanWTA = (WTA + (stat_n - 1) * meanWTA) / stat_n;

  double nvarWTA = stat_nvarWTA;
  stat_nvarWTA = nvarWTA + (WTA - meanWTA) * (WTA - stat_meanWTA);

  stat_waited += p->waiting;
  stat_lastUtilizationTimestep = currentClk;

  fprintf(log_scheduler, "At time %d process %zu finished arr %zu total %zu remain %zu wait %zu TA %zu WTA %.2f\n",
          currentClk,
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting,
          TA,
          WTA);
  fflush(log_scheduler);

  buddy_free(p->memindex, p->memsize);
  int memUpperbound = buddy_upperbound(p->memsize);
  fprintf(log_memory, "At time %d freed %d bytes from process %zu from %zu to %zu\n",
          clk_get(),
          memUpperbound,
          p->id,
          p->memindex,
          p->memindex + memUpperbound);
  fflush(log_memory);

  pcb_remove(p);
  runningProcess = NULL;

  numberOfProccesses--;
  if (numberOfProccesses == 0)
    raise(SIGINT);

  process *loaded = memload_tryLoad(&mload);

  if (loaded != NULL) {
    size_t memindex = loaded->memindex;
    size_t memUpperbound = buddy_upperbound(loaded->memsize);
    fprintf(log_memory, "At time %d allocated %zu bytes for process %zu from %zu to %zu\n",
            clk_get(),
            memUpperbound,
            loaded->id,
            memindex,
            memindex + memUpperbound);
    fflush(log_memory);

    currentAlgorithm.insertProcess(currentAlgorithm.ds, loaded);
  }

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
  signal(SIGINT, scheduler_cleanup);
  log_scheduler = fopen("logs/scheduler.log", "w");
  log_memory = fopen("logs/memory.log", "w");

  fputs("#At time x process y state arr w total z remain y wait k\n", log_scheduler);
  fflush(log_scheduler);

  struct sigaction act;
  act.sa_handler = scheduler_processTerminationHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &act, 0) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Latch the cleanup handler

  clk_init();
  stat_lastUtilizationTimestep = 1;
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

  currentAlgorithm.type = algorithm;

  memload_init(&mload, &currentAlgorithm);

  int msgqId;
  msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  if (msgqId == -1) {
    perror("Error in creating message queue");
    exit(-1);
  }

  *msgqId_p = msgqId;

  // Initialize the memory allocation system
  buddy_init();

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
  }
  else {
    return 0;
  }
}

/**
 * Creates a new stopped process and logs its arrival.
 */
void scheduler_createProcess(msgBuf *msgqBuffer) {
  int processPid = fork();
  if (processPid == 0) {
    char pRemainingTime[10];
    sprintf(pRemainingTime, "%zu", msgqBuffer->p.remaining);
    execl(BIN_DIRECTORY"/process.out", "process.out", pRemainingTime, (char *)NULL);
  }

  kill(processPid, SIGTSTP);
  msgqBuffer->p.pid = processPid;
  process *pcbProcessEntry = pcb_insert(&msgqBuffer->p);

  memload_insert(&mload, pcbProcessEntry);

  process *loaded = memload_tryLoad(&mload);

  if (loaded != NULL) {
    size_t memindex = loaded->memindex;
    size_t memUpperbound = buddy_upperbound(loaded->memsize);

    fprintf(log_memory, "At time %d allocated %zu bytes for process %zu from %zu to %zu\n",
            clk_get(),
            memUpperbound,
            loaded->id,
            memindex,
            memindex + memUpperbound);
    fflush(log_memory);
    currentAlgorithm.insertProcess(currentAlgorithm.ds, loaded);
  }
}

#endif
