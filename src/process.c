#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char* argv[]) {
  FILE* pFile = fopen("process_log.txt", "w");
  fprintf(pFile, "process with pid = %d entered\n", getpid());
  fflush(pFile);

  remainingtime = atoi(argv[1]);

  initClk();

  int previousClk = getClk();
  int currentClk;
  while (remainingtime > 0) {
    currentClk = getClk();
    if (currentClk > previousClk) {
      remainingtime--;
      previousClk = currentClk;
    }
  }

  destroyClk(false);

  return 0;
}