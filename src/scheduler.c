#include "scheduler.h"

int main(int argc, char *argv[]) {
  int algorithm = argv[1][0] - '0';
  int msgqId;
  scheduler_init(algorithm, &msgqId);

  int msgqENO;
  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;  // Dummy val
  int currentClk, previousClk = -1;

  FILE *pqueueLogFile = fopen("logs/pqueue_log.txt", "w");
  while (1) {
    currentClk = getClk();
    if (scheduler_getMessage(msgqId, &msgqBuffer, currentClk)) {
    } else {
      scheduler_createProcess(&msgqBuffer, currentClk);

      bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

      if (mustPreempt) {
        scheduler_preemptProcess(runningProcess);
        process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

        scheduler_resumeProcess(nextProcess);
      }
    }

    if (currentClk > previousClk) {
      previousClk = currentClk;
      pcb_update();
      bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

      if (mustPreempt) {
        scheduler_preemptProcess(runningProcess);
        process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

        scheduler_resumeProcess(nextProcess);
      }
      pcb_log(pcbLogFile);
    }
  }

  destroyClk(true);
  return 0;
}
