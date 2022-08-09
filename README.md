# OS_Scheduler
Operating Systems course project, Fall '20.

## Overview
- The project emulates an operating system scheduler, with processes arriving and going through the scheduling lifecycle of running, being pre-empted, idling, etc...
- The project applies several Operating Systems concepts such as: Interprocess Communication, Message Passing, Shared Memory, Memory Management, Round-Robin Scheduling, Highest-Priority-First Scheduling, Shortest-Remaining-Time-Next Scheduling, Process Control Blocks, Signals and Traps.
- The project also uses other programming principles such as: OOP, Priority Queues, Circular Queues, etc...
- The project reads its simulation process data from a text file, begins creating the processes and sending their data to the scheduler, and logging the scheduler's operation

## How to run
```
$ make clean build
$ make generate
$ make run
```