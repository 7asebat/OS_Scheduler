#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char* argv[]) {
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
  exit(0);
  return 0;
}