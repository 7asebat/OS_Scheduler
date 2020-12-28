#ifndef HEADERS_H
#define HEADERS_H

#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>  //if you don't use scanf/printf change this include
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* typedef bool short; */
/* #define true 1 */
/* #define false 1 */

#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define SHKEY 300

#define STATUS_WAITING 0
#define STATUS_RUNNING 1

#define MSGQKEY 200

#define SEMKEY 400

#define DS_MAX_SIZE 100
///==============================
//don't mess with this variable//
int *shmaddr;  //
//===============================

int getClk() {
  return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk() {
  int shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
  while ((int)shmid == -1) {
    //Make sure that the clock exists
    printf("Wait! The clock not initialized yet!\n");
    sleep(1);
    shmid = shmget(SHKEY, 4, 0444);
  }
  shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll) {
  shmdt(shmaddr);
  if (terminateAll) {
    killpg(getpgrp(), SIGINT);
  }
}

typedef union {
  int val;               /* value for SETVAL */
  struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
  ushort *array;         /* array for GETALL & SETALL */
  struct seminfo *__buf; /* buffer for IPC_INFO */
  void *__pad;
} Semun;

void down(int sem, int id) {
  struct sembuf p_op;

  p_op.sem_num = id;
  p_op.sem_op = -1;
  p_op.sem_flg = !IPC_NOWAIT;

  if (semop(sem, &p_op, 1) == -1) {
    int x = errno;
    char errorbuf[255];
    sprintf(errorbuf, "Error in down(), %d", errno);
    perror(errorbuf);
    exit(-1);
  }
}



void up(int sem, int id) {
  struct sembuf v_op;

  v_op.sem_num = id;
  v_op.sem_op = 1;
  v_op.sem_flg = !IPC_NOWAIT;

  if (semop(sem, &v_op, 1) == -1) {
    perror("Error in up()");
    exit(-1);
  }
}

typedef struct process {
  size_t arrival;
  size_t runtime;

  size_t remaining;
  size_t waiting;

  size_t priority;
  size_t status;
  size_t id;
  size_t pid;
} process;

process *runningProcess = NULL;

#define ALGORITHM_HPF 1
#define ALGORITHM_SRTN 2
#define ALGORITHM_RR 3

typedef struct schedulingAlgorithm {
  void *algorithmDS;

  int (*insertProcess)(void *ds, process *p);
  bool (*mustPreempt)(void *ds);
  process *(*getNextProcess)(void *ds);
  int (*removeProcess)(void *ds, process *p);
} schedulingAlgorithm;

typedef struct {
  long mtype;
  process p;
} msgBuf;

#endif