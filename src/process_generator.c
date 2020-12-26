#include "headers.h"
#include<string.h>
#include "cqueue.h"

void clearResources(int);

cqueue processQueue; //Processes queue
int selectedAlgorithm;
pid_t clkPid,schedulerPid;

typedef struct msgBuf{
    long mtype;
    process p;
}msgBuf;

void parseInput(char* fn){
    FILE* iFile = fopen("processes.txt","r");
    if(iFile == NULL){
        perror("Error opening the input file");
        exit(1);
    }
    char chunk[128];
    while(fgets(chunk,sizeof(chunk),iFile)!=NULL){
        if(chunk[0]=='#')
            continue;
        char * token = strtok(chunk, "\t");
        int data[4],index=0;
        // loop through the string to extract all other tokens
        do{
            data[index]=atoi(token);
            token = strtok(NULL,"\t");
            index++;
        }while(token!=NULL);
        process *pTemp = (process*) malloc(sizeof(process));
        pTemp->pid=data[0],pTemp->arrival=data[1],pTemp->runtime=data[2],pTemp->priority=data[3];
        cqueue_enqueue(&processQueue,pTemp);
    }
}

void promptUserForAlgorithm(){
    printf("Enter the required scheduling algorithm (1) for HPF, (2) for SRTN, (3) for RR\n");
    do{
        scanf("%d",&selectedAlgorithm);
        if(selectedAlgorithm < 1 || selectedAlgorithm > 3)
            printf("Invalid input, Enter the required scheduling algorithm (1) for HPF, (2) for SRTN, (3) for RR\n");
    }while(selectedAlgorithm < 1 || selectedAlgorithm > 3);
}

void startProcesses(){
    clkPid = fork();
    if(clkPid < 0){
        perror("Error while forking");
        exit(1);
    }
    if(clkPid == 0){
        execl("clk.out",NULL);
    }else{
        schedulerPid = fork();
        if(schedulerPid < 0){
            perror("Error while forking");
            exit(1);
        }
        if(schedulerPid == 0){
            printf("CLK process %d PPID: %d",schedulerPid,getppid());
            execl("scheduler.out",NULL);
        }
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    cqueue_create(&processQueue,100);
    // 1. Read the input files.
    char* fileName = "processes.txt";
    parseInput(fileName);
    // 2. Propmt user for the scheduling algorithm
    promptUserForAlgorithm();
    
    //Create a message queue for information passing between scheduler and process_generator
    int msgqId, sendVal;
    msgBuf message;
    message.mtype=1; //Dummy val
    msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
    if(msgqId == -1){
        perror("Error in creating message queue");
        exit(-1);
    }

    // 3. Initiate and create the scheduler and clock processes.
    startProcesses();
    // printf("Clk process initialized with pid : %d \nScheduler process initialized with pid %d\n",clkPid,schedulerPid);
    
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);

    // TODO Generation Main Loop
    while(processQueue.occupied != 0){
        // 5. Create a data structure for processes and provide it with its parameters.
        int currClk=getClk();
        process *temp = cqueue_front(&processQueue);
        // 6. Send the information to the scheduler at the appropriate time.
        if(temp->arrival <= currClk){
            temp = cqueue_dequeue(&processQueue);
            message.p = *temp;
            sendVal = msgsnd(msgqId, &message, sizeof(message.p), !IPC_NOWAIT);
            if(sendVal == -1){
                perror("Error in sending message");
            }
        }
    }
    printf("Waiting for scheduler to finish\n");
    int statLoc;
    pid_t cPid = wait(&statLoc);

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //Clearing resources
    destroyClk(true);
    int msgqId = msgget(MSGQKEY, 0666 | IPC_CREAT);
    msgctl(msgqId, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
