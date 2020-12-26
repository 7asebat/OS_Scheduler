#include <signal.h>
#include "headers.h"

int numberOfProcesses = 2;
char *processesInfo[] = {"5", "1"};

int main(int argc, char *argv[])
{
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
