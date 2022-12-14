#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
// take from Lab 3 Part 2
// Parent and Child Processes

// Implement semaphore instead of while loop
void ClientProcess(int []);
void DadProcess(int [], sem_t *);
void ChildProcess(int [], sem_t *, int);
void MomProcess(int [], sem_t *);

//define ShmID; ShmPTR; pid; status; mutex
int  main(int  argc, char *argv[])
{
     int    ShmID, i;
     int    *ShmPTR;
     pid_t  pid;
     int    status;
     sem_t *mutex;
// ShmID sets IPC_PRIVATE
// if statement whether its greater than 0

     ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }

    if ((mutex = sem_open("examplesemaphore5", O_CREAT, 0644, 1)) == SEM_FAILED) {
          perror("semaphore initilization");
         exit(1);
     }
     printf("Server has attached the shared memory...\n");

     printf("Server is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
          for(i = 0; i < 25; i++)
               ChildProcess(ShmPTR, mutex, getpid());
          exit(0);
     }

     DadProcess(ShmPTR, mutex);

     wait(&status);
     printf("Server has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);
}

// ChildProcess like in Lab 3
// Uses semaphore

void ChildProcess(int sharedMem[], sem_t* mutex, int id){
  int localBalance, amount, need;
  srand(getpid());
  sleep(rand()%6);
  amount = rand()%101;
  printf("Poor Student: Attempting to Check Balance\n");
  sem_wait(mutex);
  if (amount %2 == 0){
    localBalance = sharedMem[0];

      need = rand()%51;
      printf("Poor Student needs $%d\n", need);
      if (need <= localBalance){
        localBalance -= need;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
      }
      else{
        printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
      }
      sharedMem[0] = localBalance;
  }
  else{
    printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
  }
  sem_post(mutex);
}

//no two processes can increment the shared counter variable   counterptr


void DadProcess(int sharedMem[], sem_t* mutex) {
  int localBalance, amount;
  srand(getpid());

  for (int i=0; i<25; i++) {
    sleep(rand()%6);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    sem_wait(mutex);
    localBalance = sharedMem[0];
    amount = rand()%101;
    if (localBalance < 100){
      if (amount % 2 == 0){
        localBalance += amount;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
        sharedMem[0] = localBalance;
      }
      else{
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
    }
    else{
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
    }
    sem_post(mutex);
  }
}

//modified code of Lab3 part 2 
