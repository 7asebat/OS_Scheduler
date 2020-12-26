#include <signal.h>
#include "headers.h"

typedef struct {
  process *array;
  int used;
  int size;
} pcb;

pcb PCB;

void initPCB(int initialSize){
    (&PCB)->array = malloc(initialSize * sizeof(process));
    (&PCB)->used = 0;
    (&PCB)->size = initialSize;
}

void pcb_insert(process element) {
  if ((&PCB)->used == (&PCB)->size) {
    (&PCB)->size *= 2;
    (&PCB)->array = realloc((&PCB)->array, (&PCB)->size * sizeof(process));
  }
  (&PCB)->array[(&PCB)->used++] = element;
}

void pcb_remove(process element){
    int pidToRemove = element.pid;

    for(int i=0; i<PCB.used; i++){
        if( PCB.array[i].pid == pidToRemove ){

            for(int j=i; j<PCB.used-1; j++){
                PCB.array[j] = PCB.array[j+1];
            }
            PCB.used -= 1;
            break;
        }
    }

}

int numberOfProcesses = 2;
char *processesInfo[] = {"5", "1"};

int main(int argc, char *argv[])
{
    initPCB(10);

    process p1 = {1, 10};
    process p2 = {2, 50};
    process p3 = {3, 80};

    pcb_insert(p1);
    pcb_insert(p2);
    pcb_insert(p3);    
    printf("The number of used items : %d\n", PCB.used); 

    pcb_remove(p2);
    printf("The number of used items : %d\n", PCB.used); 

    initClk();

    int clkProcess = fork();
    if(clkProcess == 0){
        //child ( the clock process itself )
        execl("clk.out", NULL);
    }

    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    for (int i = 0; i < numberOfProcesses; i++)
    {
        int processPid = fork();
        if(processPid == 0){
            //child ( the process itself )
            execl("process.out", "process.out", processesInfo[i], NULL);
        }
        // send cont signal to process
        // kill(processPid, SIGCONT);

        int process_status;
        int exitedProcessPid = wait(&process_status);
        if(WIFEXITED(process_status)){
            int exit_code = WEXITSTATUS(process_status);
            printf("process %d: exited with exit code %d\n",exitedProcessPid, exit_code);
        }

    }

    destroyClk(true);
}
