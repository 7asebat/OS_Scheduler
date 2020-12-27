#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

void handler() {
  printf("SIGNAL RECIVED TO CONTINUE");
  sleep(3);
  exit(1);
}

int main(int agrc, char *argv[]) {
  initClk();

  remainingtime = atoi(argv[1]);

  signal(SIGCONT, handler);
  printf("My pid is : %d \n", getpid());

  int currentClk = getClk();
  while (getClk() < currentClk + remainingtime) {
  }

  exit(1);
  destroyClk(false);

  return 0;
}
