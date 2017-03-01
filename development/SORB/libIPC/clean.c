#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <typewrappers.h>
#include <IPC.h>
#include "ipcqueue.h"

struct ipcQueueStruct {
  pid_t     owner; /* proces id of owner of the queue */
  INT32     semId; /* SysV id for the receiver's semaphore */
  IPCQueue  queue; /* The queue itself   */
} * ipcQueues;

int main() {
	int shmid;
	int result;
	int i;
	struct shmid_ds desc;
	union semun semInfo;

	shmid = shmget (ftok("~/IPC/SharedMemory", 'A'), 0, 0666);
	ipcQueues = shmat (shmid, 0, 0666);
	for (i=0; i<5; i++) /* remove the semaphores */
      		semctl (ipcQueues[i].semId, 0, IPC_RMID, semInfo);
	result = shmctl (shmid, IPC_RMID, &desc);
	printf ("%i\n", result);
	return 0;
}
