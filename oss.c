/************************************************
 * $Author: o-rooneo $
 *
 * $Date: 2015/04/16 01:44:05 $
 *
 * $Log: oss.c,v $
 * Revision 1.2  2015/04/16 01:44:05  o-rooneo
 * added rcs vars at top
 *
 
  ***********************************************/

#include "definitions.h"

int resourceCount;
int child[MAX_CREATED_PROCESSES];
int semid[3];
char *sem[3];
char *shm[MAX_CREATED_PROCESSES];
int shmid[MAX_CREATED_PROCESSES];

void timeIncrement(LogicalClock *);
int microToMilli(int micro);
int milliToMicro(int milli);
bool createNewProcess(int seconds, int mseconds);

static void signal_handler(int signum){
	int i;
	for(i=0;i<=MAX_CREATED_PROCESSES;i++){
		shmdt(shm[i]);
		shmctl(shmid[i], IPC_RMID, NULL); //delete
	}
	//print message
	exit(1);
}

int main(int argc, char **argv)
{
	resourceCount = 20;

	int i, j, k, l, status, result, createdProcessCount, safe = 0, exec, count;
	char procNum[2], rescNum[2];
	int clockShmID;
	int PPID = getpid();

	//Get key information for shared items
	key_t keyPros, keyRes, keyClock;
	keyPros = ftok("./KeyFile", 1);
	keyRes = ftok("./KeyFile", 2);
	keyClock = ftok("./KeyFile", 3);
	
	signal(SIGINT, signal_handler); //Don't allow program to close without clean up after this point

	struct sembuf waitProc[MAX_CREATED_PROCESSES+1], signalProc[MAX_CREATED_PROCESSES+1];
	for(i = 0; i <= MAX_CREATED_PROCESSES; i++){
		child[i]=0;

		waitProc[i].sem_num = i;
		waitProc[i].sem_op = -1;
		waitProc[i].sem_flg = SEM_UNDO;

		signalProc[i].sem_num = i;
		signalProc[i].sem_op = 1;
		signalProc[i].sem_flg = SEM_UNDO;
	}

	ResourceDescriptor *descriptors[resourceCount];
	struct sembuf waitRes[resourceCount], signalRes[resourceCount];
	
	semid[0] = semget(keyPros,MAX_CREATED_PROCESSES,IPC_CREAT | 0666);
	semid[1] = semget(keyRes,resourceCount,IPC_CREAT | 0666);
	semid[3] = semget(keyClock,1,IPC_CREAT | 0666);
	
		
	for(i= 0; i<=resourceCount; i++){	
		
		shmid[i] = shmget(keyRes+i, sizeof(ResourceDescriptor), 0666);
		if (shmid[i] == -1){ 
			fprintf(stderr, "%s: Producer shm error", argv[0]);
			return 1;
		}
		shm[i] = (char *)(shmat(shmid[i],0,0));
		
		descriptors[i]->shareable = false;
		descriptors[i]->resouceClass = i;
		descriptors[i]->totalResouce = (rand()%10)+1;
		descriptors[i]->available = descriptors[i]->totalResouce;
		descriptors[i]->utilization[MAX_CREATED_PROCESSES];
		
		descriptors[i] = (ResourceDescriptor *) shm[i];
		
		waitRes[i].sem_num = i;
		waitRes[i].sem_op = -1;
		waitRes[i].sem_flg = SEM_UNDO;

		signalRes[i].sem_num = i;
		signalRes[i].sem_op = 1;
		signalRes[i].sem_flg = SEM_UNDO;
	}

	int nonshareableCount = resourceCount - resourceCount*(((rand()%10)+15)/100);
	for(i = resourceCount-1; i > nonshareableCount-1; i--){
		descriptors[i]->shareable = true;
	}

	//initialize clock
	LogicalClock *clock;
	struct sembuf waitClock, signalClock;
	shmid[2] = shmget(keyClock, sizeof(LogicalClock), IPC_CREAT | 0666);
	if(shmid[2] == -1){
		//error
		exit(-1);
	}
	clock = shmat(shmid[2], NULL, 0);
	clock->seconds = 0;
	clock->micro = 0;
	
	do{
		//check if we can and should create a new process
		if(createNewProcess && createdProcessCount <= MAX_CREATED_PROCESSES){
			for(i = 0; i < MAX_CREATED_PROCESSES; i++){
				if(child[i]==0){
					child[i]=fork();
					if(child[i] == -1){	
						fprintf(stderr, "%s: Failed to fork on %i", argv[0], i);
						return -1;
					}
					if(child[i]==0){
						sprintf(procNum, "%d", i);
						printf(rescNum, "%d", resourceCount);
						execl("./userProcess","userProcess", procNum, rescNum, (const char *) 0);
					}
				}
			}
		}
		//clear out any old processes
		do{
			result = waitpid(-1, &status, WNOHANG);
			if(result > 0){ //A child process has ended
				for(i = 0; i < MAX_CREATED_PROCESSES; i++){
					if(result == child[i]){
						child[i]=0;
						for (j = 0; j < resourceCount; j++) {	
							semop(semid[1],&waitRes[j],1);
							descriptors[j]->utilization[i] = 0;
							descriptors[j]->maximum[i] = 0;
							descriptors[j]->request[i] = 0;
							semop(semid[1],&signalRes[j],1);
						}
					}
				}
			}
		}while(result > 0);
		//check for resource requests
		for(i = 0; i < resourceCount; i++){
			for(j = 0; j < MAX_CREATED_PROCESSES; j++){
				semop(semid[1],&waitRes[i],1);
				if(descriptors[i]->request[j] > 0){
					count = MAX_CREATED_PROCESSES;
					while (count != 0) {
						safe = 0;
						for (k = 0; k < MAX_CREATED_PROCESSES; k++) {
							if (child[k]) {
								exec = 1;
								for (l = 0; l < resourceCount; l++) {
									if (descriptors[k]->maximum[l] - descriptors[k]->utilization[l] > descriptors[k]->available) {
										exec = 0;
										break;
									}
								}
								if (exec) {
									printf("\nProcess%d is executing\n", i + 1);
									child[k] = 0;
									count--;
									safe = 1;
				 
									for (l = 0; l < resourceCount; l++) {
										descriptors[k]->available += descriptors[k]->utilization[l];
									}
									break;
								}
							}
						}
						if (!safe) {
							printf("\nThe processes are in unsafe state.\n");
							break;
						} else {
							printf("\nThe process is in safe state");
						}
					}
				}
				semop(semid[1],&signalRes[i],1);
			}
		}
		semop(semid[3],&waitClock,1);	//mutex clock
		timeIncrement(clock);
		semop(semid[3],&signalClock,1);
	}while(true);
}
 
void timeIncrement(LogicalClock *clock)
{
	clock->micro = clock->micro + rand()%1000; //overhead
	while(clock->micro > 1000000){
		clock->seconds = clock->seconds + (int)(clock->micro / 1000000);
		clock->micro = clock->micro % 1000000;
	}
}
 
bool createNewProcess(int seconds, int mseconds)
{
	static int lastSet = 0;
	static int timeToNextProcess = 0; //set to zero to ensure a process is created on first run
	int currentMicroTotal = 1000000*seconds + mseconds;
	if(currentMicroTotal < lastSet + timeToNextProcess){
		lastSet = currentMicroTotal;
		timeToNextProcess = milliToMicro((rand()%500)+1);
		return true;
	}
	lastSet = currentMicroTotal;
	return false;
}

int microToMilli(int micro)
{ 
	return (int)(micro/1000);
}

int milliToMicro(int milli)
{ 
	return (int)(milli*1000);
}

