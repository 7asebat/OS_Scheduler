#include "scheduler.h"

int main(int argc, char *argv[]) {
  int algorithm = argv[1][0] - '0';
  int msgqId, semid;
  scheduler_init(algorithm, &msgqId, &semid);

  int currentClk, previousClk, msgqENO;
  previousClk = 0;
  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;  // Dummy val

  int currentClk, previousClk = -1;
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

      if (runningProcess == NULL) {
        bool mustPreempt = currentAlgorithm.mustPreempt(currentAlgorithm.algorithmDS);

        if (mustPreempt) {
          preemptProcess(runningProcess);
          process *nextProcess = currentAlgorithm.getNextProcess(currentAlgorithm.algorithmDS);

          resumeProcess(nextProcess);
        }
      }
    }

    currentClk = getClk();
    if (!scheduler_getMessage(msgqId, &msgqBuffer, currentClk))
      scheduler_createProcess(&msgqBuffer, currentClk);

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
      up(semid, 0);
    }
  }

  destroyClk(true);
  return 0;
}
