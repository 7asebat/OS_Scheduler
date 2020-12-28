#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char* argv[]) {
  struct sigaction act;

  act.sa_handler = SIG_DFL;
  act.sa_flags = SA_RESTART;

  if (sigaction(SIGCONT, &act, 0) == -1) {
    perror("sigaction");
    exit(1);
  }

  char logFileName[100];
  sprintf(logFileName, "logs/process_%d.txt", (int)getpid());

  FILE* logFile = fopen(logFileName, "w");
  remainingtime = atoi(argv[1]);

  int semid = semget(SEMKEY, 1, 0666 | IPC_CREAT);

  if (semid == -1) {
    perror("error in creating semaphore");
    exit(-1);
  }

  initClk();

  int previousClk = getClk();
  int currentClk;
  while (remainingtime > 0) {
    down(semid, 0);
    currentClk = getClk();
    if (currentClk > previousClk) {
      remainingtime--;
      previousClk = currentClk;
      fprintf(logFile, "Clk = %d, remainingTime = %d\n", currentClk, remainingtime);
      fflush(logFile);
    }
  }

  destroyClk(false);
  exit(0);
  return 0;
}