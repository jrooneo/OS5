#ifndef DEFS
#define DEFS

typedef int bool;
enum { false, true };


//Standard set of includes needed in each program
#include <errno.h>
#include <time.h>
#include <semaphore.h>      
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MAX_CREATED_PROCESSES 18
const char *path = "./keyFile";

typedef struct{
	int seconds;
	int micro;
}LogicalClock;

typedef struct{
	int resouceClass;
	int totalResouce;
	int available;
	bool shareable;
	int utilization[MAX_CREATED_PROCESSES];
	int request[MAX_CREATED_PROCESSES];
	int maximum[MAX_CREATED_PROCESSES];
}ResourceDescriptor;
//uti and request can be treated as a 2D array.
//resource[i].(uti/request)[j]
//It's really no different
#endif
