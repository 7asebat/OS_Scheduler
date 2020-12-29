#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "HPF.h"
#include "RR.h"
#include "SRTN.h"
#include "buddy.h"
#include "headers.h"
#include "pcb.h"

schedulingAlgorithm currentAlgorithm;
FILE *log_scheduler;
FILE *log_memory;
FILE *log_pcb;
FILE *log_sch;

void scheduler_checkContextSwitch();

void scheduler_cleanup(int SIGNUM) {
  fclose(log_scheduler);
  fclose(log_pcb);
  fclose(log_memory);
  fclose(log_sch);

  pcb_free();

  int msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  msgctl(msgqId, IPC_RMID, (struct msqid_ds *)0);

  destroyClk(false);

  signal(SIGINT, scheduler_cleanup);
}

void scheduler_preemptProcess(process *p) {
  if (p == NULL)
    return;

  int pid = p->pid;
  p->status = STATUS_WAITING;
  kill(pid, SIGTSTP);
  fprintf(log_scheduler, "At time %d process %zu stopped arr %zu total %zu remain %zu wait %zu\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(log_scheduler);

  fprintf(log_sch, "[%d]\t%zu STOP  \tREM (%zu)\n",
          getClk(), p->id, p->remaining);
  fflush(log_sch);

  // TODO: store context of process
}

void scheduler_resumeProcess(process *p) {
  runningProcess = p;
  if (p == NULL) return;

  int pid = p->pid;
  p->status = STATUS_RUNNING;

  kill(pid, SIGCONT);

  bool started = (p->remaining == p->runtime);

  fprintf(log_scheduler, "At time %d process %zu %s arr %zu total %zu remain %zu wait %zu\n",
          getClk(),
          p->id,
          (started) ? "started" : "resumed",
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting);
  fflush(log_scheduler);

  fprintf(log_sch, "[%d]\t%zu %s\tREM (%zu)\n",
          getClk(), p->id, started ? "START " : "RESUME", p->remaining);
  fflush(log_sch);
}

void scheduler_processTerminationHandler(int SIGNUM) {
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

  fprintf(log_scheduler, "At time %d process %zu finished arr %zu total %zu remain %zu wait %zu TA %zu WTA %.2f\n",
          getClk(),
          p->id,
          p->arrival,
          p->runtime,
          p->remaining,
          p->waiting,
          TA,
          WTA);
  fflush(log_scheduler);

  // DEBUG
  fprintf(log_sch, "[%d]\t%zu FINISH\tREM (%zu)\n",
          getClk(), p->id, p->remaining);
  fflush(log_sch);

  runningProcess = NULL;

  buddy_free(p->memindex, p->memsize);
  int memUpperbound = buddy_upperbound(p->memsize);
  fprintf(log_memory, "At time %d freed %d bytes from process %zu from %d to %d\n",
          getClk(),
          memUpperbound,
          p->id,
          p->memindex,
          p->memindex + memUpperbound);
  fflush(log_memory);

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
  log_pcb = fopen("logs/pcb.log", "w");
  log_memory = fopen("logs/memory.log", "w");
  log_scheduler = fopen("logs/scheduler.log", "w");
  fprintf(log_scheduler, "Scheduler loaded\n");
  fflush(log_scheduler);

  // DEBUG
  log_sch = fopen("./logs/sch.log", "w");
  struct sigaction act;

  act.sa_handler = scheduler_processTerminationHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &act, 0) == -1) {
    perror("sigaction");
    exit(1);
  }

  signal(SIGINT, scheduler_cleanup);

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
  fprintf(log_scheduler, "Process received at clock %d, id is %zu\n", getClk(), msgqBuffer->p.id);
  fflush(log_scheduler);

  fprintf(log_sch, "[%d]\t%zu ARRIVE\tBURST (%zu)\n",
          getClk(), msgqBuffer->p.id, msgqBuffer->p.runtime);
  fflush(log_sch);

  int processPid = fork();
  if (processPid == 0) {
    // raise(SIGSTOP);
    char pRemainingTime[10];
    sprintf(pRemainingTime, "%zu", msgqBuffer->p.remaining);
    execl("bin/process.out", "process.out", pRemainingTime, (char *)NULL);
  }

  kill(processPid, SIGTSTP);

  msgqBuffer->p.pid = processPid;

  int memindex = buddy_allocate(msgqBuffer->p.memsize);
  if (memindex == -1) {
    // handle error
  }
  msgqBuffer->p.memindex = memindex;
  int memUpperbound = buddy_upperbound(msgqBuffer->p.memsize);
  fprintf(log_memory, "At time %d allocated %d bytes for process %zu from %d to %d\n",
          getClk(),
          memUpperbound,
          msgqBuffer->p.id,
          memindex,
          memindex + memUpperbound);
  fflush(log_memory);

  process *pcbProcessEntry = pcb_insert(&msgqBuffer->p);
  currentAlgorithm.insertProcess(currentAlgorithm.algorithmDS, pcbProcessEntry);
}

#endif
