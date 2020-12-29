#include "headers.h"

int time_remaining, time_runtime, time_waited = 0, time_start, time_sleep;
void sleepHandler(int SIGNUM) {
  time_sleep = clk_get();
  raise(SIGSTOP);
  signal(SIGTSTP, sleepHandler);
}

void wakeHandler(int SIGNUM) {
  time_waited += clk_get() - time_sleep;
  signal(SIGCONT, wakeHandler);
}

int main(int argc, char* argv[]) {
  signal(SIGTSTP, sleepHandler);
  signal(SIGCONT, wakeHandler);

  clk_init();
  time_start = clk_get();
  time_runtime = atoi(argv[1]);
  time_remaining = time_runtime;

  while (time_remaining > 0) {
    time_remaining = time_runtime - (clk_get() - time_start - time_waited);
  }

  clk_destroy(false);
  exit(0);
  return 0;
}
