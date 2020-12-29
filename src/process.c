#include "headers.h"

int remainingtime, runTime, waitingTime = 0, startTime, sleepTime;
FILE* logFile;
void sleepHandler(int SIGNUM) {
  sleepTime = getClk();
  fprintf(logFile, "Now sleeping at time %d\n", sleepTime);
  fflush(logFile);
  raise(SIGSTOP);
  signal(SIGTSTP, sleepHandler);
}

void wakeHandler(int SIGNUM) {
  fprintf(logFile, "Now Waking at time %d lastSleepTime %d lastWaitingTime %d\n", getClk(), sleepTime, waitingTime);
  fflush(logFile);
  waitingTime += getClk() - sleepTime;
  signal(SIGCONT, wakeHandler);
}

int main(int argc, char* argv[]) {
  signal(SIGTSTP, sleepHandler);
  signal(SIGCONT, wakeHandler);

  initClk();
  startTime = getClk();
  runTime = atoi(argv[1]);
  remainingtime = runTime;

  while (remainingtime > 0) {
    remainingtime = runTime - (getClk() - startTime - waitingTime);
  }

  destroyClk(false);
  exit(0);
  return 0;
}
