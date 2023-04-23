#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int semid;
struct sembuf sops[1];
void signalAllButMe(int i);
void cleanup();
extern int errno;

#ifdef _SEM_SEMUN_UNDEFINED
/* In some earlier versions of glibc, the semun union was defined in <sys/sem.h>,
 * but POSIX.1 requires that the caller define this union. On those glibc versions 
 * macro _SEM_SEMUN_UNDEFINED is defined in <sys/sem.h>.
*/
union semun
{
   int val;
   struct semid_ds * buff;
   unsigned short *array;
   struct seminfo *_buf;
};
#endif
union semun semarg;

#define MAX_NUM (100)		//Maximal number to print
#define NUM_OF_TASKS (5)	//Number of concurrent processes

void main()
{
    int i, j, status;
    pid_t pid[NUM_OF_TASKS],fork_res;

    semid=semget(IPC_PRIVATE, NUM_OF_TASKS, 0600);

    for(i = 1; i < NUM_OF_TASKS + 1; i++)
        signal(i, &cleanup);

    for(i = 0; i < NUM_OF_TASKS; i++)
    {
        semarg.val = (NUM_OF_TASKS - 1) - i;
        semctl(semid, i, SETVAL, semarg);
	pid[i] = 0;

    }

    sops->sem_num = 0;
    sops->sem_flg = 0;

    for(i = 0; i < NUM_OF_TASKS; i++)
    {
	fork_res = fork();
	if (fork_res == 0) 
        {
            for(j = i + 1; j < MAX_NUM + 1; j += NUM_OF_TASKS)
            {
    		sops->sem_num = i;
    		sops->sem_op = -(NUM_OF_TASKS - 1);
            	semop ( semid , sops , 1 );
		printf("%d, task#%d\n", j, i);
                signalAllButMe(i);
            }
	    exit(0);
        }
	else if (fork_res > 0) 
		pid[i] = fork_res;	
	else 
		printf("Fork failed %s\n", strerror(errno));
    }
    do ; while (wait(&status) > 0);
    for (i=0; i< NUM_OF_TASKS; i++)
    {
	    printf ("Task#%d had PID %d\n", i, pid[i]);
    }

    cleanup();
}

void cleanup()
{
    semctl ( semid , 0 , IPC_RMID , semarg );
    exit(1);
}

void signalAllButMe(int i)
{
    int k;
    for(k = 0; k < NUM_OF_TASKS; k++)
    {
        if(k != i)
        {
            sops->sem_num = k;
            sops->sem_op = 1;
            semop ( semid , sops , 1 );
        }
    }
}