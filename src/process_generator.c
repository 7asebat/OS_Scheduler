#include <string.h>

#include "cqueue.h"
#include "headers.h"

void clearResources(int);

cqueue processQueue;  //Processes queue
int selectedAlgorithm;
pid_t clkPid, schedulerPid;

void parseInput(char *fn) {
  FILE *iFile = fopen("input/processes.txt", "r");
  if (iFile == NULL) {
    perror("Error opening the input file");
    exit(1);
  }
  char chunk[128];
  while (fgets(chunk, sizeof(chunk), iFile) != NULL) {
    if (chunk[0] == '#')
      continue;
    char *token = strtok(chunk, "\t");
    int data[4], index = 0;
    // loop through the string to extract all other tokens
    do {
      data[index] = atoi(token);
      token = strtok(NULL, "\t");
      index++;
    } while (token != NULL);
    process *pTemp = (process *)malloc(sizeof(process));
    pTemp->id = data[0], pTemp->arrival = data[1], pTemp->remaining = data[2], pTemp->runtime = data[2], pTemp->priority = data[3];
    cqueue_enqueue(&processQueue, pTemp);
  }
}

void promptUserForAlgorithm() {
  printf("Enter the required scheduling algorithm (1) for HPF, (2) for SRTN, (3) for RR\n");
  do {
    scanf("%d", &selectedAlgorithm);
    if (selectedAlgorithm < 1 || selectedAlgorithm > 3)
      printf("Invalid input, Enter the required scheduling algorithm (1) for HPF, (2) for SRTN, (3) for RR\n");
  } while (selectedAlgorithm < 1 || selectedAlgorithm > 3);
}

void startProcesses() {
  clkPid = fork();
  if (clkPid < 0) {
    perror("Error while forking");
    exit(1);
  }
  if (clkPid == 0) {
    execl("bin/clk.out", "clk.out", (char *)NULL);
  }
  else {
    schedulerPid = fork();
    if (schedulerPid < 0) {
      perror("Error while forking");
      exit(1);
    }
    if (schedulerPid == 0) {
      char selectedAlgorithmChar[5];
      sprintf(selectedAlgorithmChar, "%d", selectedAlgorithm);
      printf("forking scheduler.out now\n");
      execl("bin/scheduler.out", "scheduler.out", selectedAlgorithmChar, (char *)NULL);
    }
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
  cqueue_create(&processQueue, 100);

  char *fileName = "../input/processes.txt";
  parseInput(fileName);

  promptUserForAlgorithm();

  int msgqId, sendVal;
  msgBuf message;
  message.mtype = 1;  // Dummy val
  msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  if (msgqId == -1) {
    perror("Error in creating message queue");
    exit(-1);
  }

  startProcesses();

  clk_init();

  while (processQueue.occupied != 0) {
    int currClk = clk_get();
    process *temp = cqueue_front(&processQueue);
    if (temp->arrival <= currClk) {
      temp = cqueue_dequeue(&processQueue);
      message.p = *temp;
      sendVal = msgsnd(msgqId, &message, sizeof(message.p), !IPC_NOWAIT);
      if (sendVal == -1) {
        perror("Error in sending message");
      }
      free(temp);
    }
  }
  printf("Waiting for scheduler to finish\n");
  int statLoc;
  pid_t cPid = wait(&statLoc);

  clk_destroy(true);
}

void clearResources(int signum) {
  clk_destroy(true);
  int msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
  msgctl(msgqId, IPC_RMID, (struct msqid_ds *)0);
  exit(0);
}
