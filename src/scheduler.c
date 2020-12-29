#include "scheduler.h"

int main(int argc, char *argv[]) {
  // Initialize the scheduler
  int algorithm = argv[1][0] - '0';
  int msgqId;
  scheduler_init(algorithm, &msgqId);

  // Create the message buffer
  msgBuf msgqBuffer;
  msgqBuffer.mtype = 1;

  int timestep, previousTS = -1;
  while (1) {
    timestep = clk_get();
    // Check if new processes have been received
    if (scheduler_getMessage(msgqId, &msgqBuffer)) {
      // No process was received
    } else {
      // Create process
      scheduler_createProcess(&msgqBuffer);

      // Update PCB with new timestep
      if (timestep > previousTS) {
        pcb_update();
      }

      // Handle context switching
      scheduler_checkContextSwitch();

      // Update timestep
      if (timestep > previousTS) {
        previousTS = timestep;
      }
    }

    if (timestep > previousTS) {
      // Update PCB with new timestep
      pcb_update();

      // Handle context switching
      scheduler_checkContextSwitch();

      // Update timestep
      previousTS = timestep;
    }
  }

  clk_destroy(true);
  return 0;
}
