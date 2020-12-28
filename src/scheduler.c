#include "scheduler.h"

int main(int argc, char *argv[]) {
  int algorithm = argv[1][0] - '0';
  int msgqId = scheduler_init(algorithm);

  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;  // Dummy val

  int currentClk, previousClk = -1;
  while (1) {
    currentClk = getClk();
    if (!scheduler_getMessage(msgqId, &msgqBuffer, currentClk))
      scheduler_createProcess(&msgqBuffer, currentClk);

    if (currentClk > previousClk) {
      bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

      if (mustPreempt) {
        scheduler_preemptProcess(runningProcess);
        process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

        scheduler_resumeProcess(nextProcess);
      }

      previousClk = currentClk;
      pcb_update();
      pcb_log(pcbLogFile);
    }
  }

  destroyClk(true);
  return 0;
}
