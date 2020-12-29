#include "headers.h"

/* Modify this file as needed*/
int remainingtime, runTime, waitingTime = 0, startTime,sleepTime;
FILE* logFile;
void sleepHandler(int SIGNUM){
  sleepTime = getClk();
  fprintf(logFile, "Now sleeping at time %d\n",sleepTime);
  fflush(logFile);
  raise(SIGSTOP);
  signal(SIGTSTP,sleepHandler);
}

void wakeHandler(int SIGNUM){
  fprintf(logFile, "Now Waking at time %d last sleep time %d lastWaitingTime %d\n",getClk(),sleepTime,waitingTime);
  fflush(logFile);
  waitingTime += getClk()-sleepTime;
  signal(SIGCONT,wakeHandler);
}

int main(int argc, char* argv[]) {
  
  signal(SIGTSTP,sleepHandler);
  signal(SIGCONT,wakeHandler);

  char logFileName[100];
  sprintf(logFileName, "logs/process_%d.txt", (int)getpid());

  logFile = fopen(logFileName, "w");

 
  initClk();
  startTime = getClk();
  runTime = atoi(argv[1]);
  remainingtime = runTime;
  
  while (remainingtime > 0) 
  {
    remainingtime = runTime - (getClk()-startTime-waitingTime);
  }

  destroyClk(false);
  exit(0);
  return 0;
}