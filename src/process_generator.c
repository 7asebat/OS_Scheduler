#include <string.h>

#include "cqueue.h"
#include "headers.h"

void clearResources(int);

cqueue pQueue;  //Processes queue

void parseInput(char* fn) {
  FILE* iFile = fopen("processes.txt", "r");
  if (iFile == NULL) {
    perror("Error opening the input file");
    exit(1);
  }
  char chunk[128];
  while (fgets(chunk, sizeof(chunk), iFile) != NULL) {
    if (chunk[0] == '#')
      continue;
    char* token = strtok(chunk, "\t");
    int data[4], index = 0;
    // loop through the string to extract all other tokens
    do {
      data[index] = atoi(token);
      token = strtok(NULL, "\t");
      index++;
    } while (token != NULL);
    process* pTemp = (process*)malloc(sizeof(process));
    pTemp->pid = data[0], pTemp->arrival = data[1], pTemp->runtime = data[2], pTemp->priority = data[3];
    cqueue_enqueue(&pQueue, pTemp);
  }
}

int main(int argc, char* argv[]) {
  signal(SIGINT, clearResources);
  // TODO Initialization
  cqueue_create(&pQueue, 100);
  // 1. Read the input files.
  char* fileName = "processes.txt";
  parseInput(fileName);
  // 3. Initiate and create the scheduler and clock processes.
  // 4. Use this function after creating the clock process to initialize clock
  initClk();
  // To get time use this
  int x = getClk();
  printf("current time is %d\n", x);
  // TODO Generation Main Loop
  // 5. Create a data structure for processes and provide it with its parameters.
  // 6. Send the information to the scheduler at the appropriate time.
  // 7. Clear clock resources
  destroyClk(true);
}

void clearResources(int signum) {
  //TODO Clears all resources in case of interruption
  exit(0);
}
