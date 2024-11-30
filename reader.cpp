#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>

#define SHM_KEY 1234  // Same key as used in writer.c
#define SEM_KEY 5678 // same semaphore key as writer

int shmid;
char *shared_mem;
char input[10];

void sem_operation(int semid, int op) {
    struct sembuf sop;
    sop.sem_num = 0; // Semaphore number within the set (typically 0 for a single semaphore)
    sop.sem_op = op; // -1 for P (wait), +1 for V (signal)
    sop.sem_flg = 0; // Operation flags: typically 0, but could include IPC_NOWAIT or SEM_UNDO

    // if multiple accesses to perform P(wait) or V(signal) on a semaphore,
    // kernel guarantees that only one process can modify semaphore
    // with semop()
    if (semop(semid, &sop, 1) == -1) {
        perror("semop");
        exit(1);
    }

    // explain semop():
    // - `semid`: the semaphore to operate on
    // - invoke kernel to perform semaphore operation defined in `sop`
    // - 1: indicates that operation applies to a single semaphore 
}

// handle case user press control + C
void sigintHandler(int sig_num) {
    printf("\nSIGINT received, program exiting...\n");

    // Detach from shared memory (so we can clean up later)
    shmdt(shared_mem);

    exit(0);  // Exit the program
}

int main() {
    // register signal handler for when user press control + C (exit program)
    // if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    //     printf("Error setting up SIGINT handler\n");
    //     return 1;
    // }

    // Access semaphore
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    while(1){
        memset(input, 0, sizeof(input));
        // prompt if user want to get data from shared memory
        printf("Press 1 to read from shared memory\n");
        fgets(input, 10, stdin);

        // remove newline character
        if(input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

        if(strcmp(input, "1") != 0){
            continue;
        }

        sem_operation(semid, -1); // P(wait)

        // Access the shared memory segment 
        shmid = shmget(SHM_KEY, 1024, 0666);
        if (shmid == -1) {
            perror("shmget failed");
            exit(1);
        }

        // Attach to the shared memory
        shared_mem = (char *) shmat(shmid, NULL, 0);
        
        if (shared_mem == (char *) -1) {
            perror("shmat failed");
            exit(1);
        }

        // Read and display data from shared memory
        printf("Message read from shared memory: %s\n", shared_mem);

        // Detach from shared memory (so we can clean up later)
        shmdt(shared_mem);

        sem_operation(semid, 1); // V(signal)
    }

    return 0;
}
