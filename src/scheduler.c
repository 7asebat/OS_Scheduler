#include "scheduler.h"

int main(int argc, char *argv[]) {
  int algorithm = argv[1][0] - '0';
  int msgqId;
  scheduler_init(algorithm, &msgqId);

  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;  // Dummy val
  int currentClk, previousClk = -1;

  while (1) {
    currentClk = getClk();
    if (scheduler_getMessage(msgqId, &msgqBuffer)) {
      // No process was received
    } else {
      scheduler_createProcess(&msgqBuffer);

      if (currentClk > previousClk) {
        pcb_update();
      }

      scheduler_checkContextSwitch();

      if (currentClk > previousClk) {
        pcb_log(pcbLogFile);
        previousClk = currentClk;
      }
    }

    if (currentClk > previousClk) {
      pcb_update();
      scheduler_checkContextSwitch();
      pcb_log(pcbLogFile);
      previousClk = currentClk;
    }
  }

  destroyClk(true);
  return 0;
}
