#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

/* typedef bool short; */
/* #define true 1 */
/* #define false 1 */

#define SHKEY 300

#define STATUS_WAITING 0
#define STATUS_RUNNING 1

///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    while ((int)shmid == -1)
    {
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

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

typedef struct process
{
    size_t PCB_idx;
    size_t arrival;
    size_t runtime;

    size_t remaining;
    size_t waiting;

    size_t priority;
    size_t status;
    size_t pid;
} process;

#define ALGORITHM_RR 0
#define ALGORITHM_HPF 1
#define ALGORITHM_SRTN 2

typedef struct
{
    void *algorithmDS;

    int (*insertProcess)(void *ds, process *p);
    bool (*mustPreempt)(void *ds);
    process *(*getNextProcess)(void *ds);
    int (*removeProcess)(void *ds, process *p);
} schedulingAlgorithm;

#endif