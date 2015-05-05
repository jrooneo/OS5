/************************************************
 * $Author: o-rooneo $
 *
 * $Date: 2015/04/16 01:44:05 $
 *
 * $Log: userProcess.c,v $
 * Revision 1.2  2015/04/16 01:44:05  o-rooneo
 * added rcs vars at top
 *
 
  ***********************************************/

#include "definitions.h"

#include <math.h>

bool shouldDie(LogicalClock *clock);

int semid[3];
char *sem[3];
char *shm[MAX_CREATED_PROCESSES];
int shmid[MAX_CREATED_PROCESSES];

int secondStart, microStart;

int main(int argc, char **argv)
{
	//setup rand
	time_t t;
	srand((unsigned) time(&t));
	int quantum, resourceCount, i;
	int secondStart, microStart;
	
	key_t keyPros, keyRes, keyClock;
	keyPros = ftok("./KeyFile", 1);
	keyRes = ftok("./KeyFile", 2);
	keyClock = ftok("./KeyFile", 3);

	//Get key information for shared items
	int processNumber;
	if(argc > 3){
		processNumber = atoi(argv[1]);
		resourceCount = atoi(argv[2]);
	} else {
		//error 
		exit(-1);
	}
	for(i=0;i< resourceCount;i++){	 //Buffers start at index 1 and go to BUFFERCOUNT
		
	}
	
	ResourceDescriptor *descriptors[resourceCount];
	struct sembuf waitRes[resourceCount], signalRes[resourceCount];
	for(i= 0; i<=resourceCount; i++){	
	
		shmid[i] = shmget(keyRes+i, sizeof(ResourceDescriptor), 0666);
		if (shmid[i] == -1){ 
			fprintf(stderr, "%s: Producer shm error", argv[0]);
			return 1;
		}
		shm[i] = (char *)(shmat(shmid[i],0,0));
		descriptors[i] = (ResourceDescriptor *) shm[i];
	
		waitRes[i].sem_num = i;
		waitRes[i].sem_op = -1;
		waitRes[i].sem_flg = SEM_UNDO;

		signalRes[i].sem_num = i;
		signalRes[i].sem_op = 1;
		signalRes[i].sem_flg = SEM_UNDO;
	}
	semid[0] = semget(keyPros,MAX_CREATED_PROCESSES,IPC_CREAT | 0666);
	semid[1] = semget(keyRes,resourceCount,IPC_CREAT | 0666);
	semid[3] = semget(keyClock,1,IPC_CREAT | 0666);
	
	//initialize clock
	LogicalClock *clock;
	struct sembuf waitClock, signalClock;
	shmid[0] = shmget(keyClock, sizeof(LogicalClock), IPC_CREAT | 0666);
	if(shmid[0] == -1){
		//error
		exit(-1);
	}
	clock = shmat(shmid[0], NULL, 0);
	secondStart = clock->seconds;
	microStart = clock->micro;
	
	for(i=0; i<3; i++){
		if(shmid[i] <=0){
			fprintf(stderr, "Failed to initialize semaphore");
			exit(-1);
		}
	}

	//declare max claims
	for(i = 0; i < resourceCount; i++){
		semop(shmid[1], &waitRes[i], 1);
		descriptors[i]->maximum[processNumber] = rand()%(descriptors[i]->totalResouce/2);
		if(descriptors[i]->maximum[processNumber] 
		semop(shmid[1], &signalRes[i], 1);
	}
	
	do{
		if(shouldDie(&clock) && rand%2){
			for(i=0;i<=MAX_CREATED_PROCESSES;i++){
				shmdt(shm[i]);
				shmctl(shmid[i], IPC_RMID, NULL); //delete
			}
			return 0;
		}
		for(i = 0; i < resourceCount; i++){
			if(rand%3){
				descriptors[i].request[processNum] = rand()%descriptors[i].maximum[processNum];
			}
		}
	}while(true);

}

bool shouldDie(*clock){
	int beginingMicro = microStart;
	beginingMicro += secondStart*1000000;
	if(clock->seconds*1000000+clock->micro > beginingMicro){
		return true;
	}
	return false;
}