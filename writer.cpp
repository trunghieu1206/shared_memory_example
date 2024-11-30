#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>

#define SHM_KEY 1234  // Unique key for shared memory
#define SEM_KEY 5678 // key for semaphore

int shmid; // shared memory id
char *shared_mem; // map to the first address of shared memory with shmmat()
char input[256];

// Function to perform semaphore operations (P or V)
void sem_operation(int semid, int op) {
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = op;   // -1 for P (wait), +1 for V (signal)
    sop.sem_flg = 0;

    // if multiple accesses to perform P(wait) or V(signal) on a semaphore,
    // kernel guarantees that only one process can modify semaphore
    // with semop()
    if (semop(semid, &sop, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

// handle case user press control + C
void sigintHandler(int sig_num) {
    printf("\nSIGINT received, program exiting...\n");

    // Detach from shared memory (so we can clean up later)
    shmdt(shared_mem);

    // Clean up the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);

    exit(0);  // Exit the program
}

int main() {
    // register signal handler for when user press control + C (exit program)
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        printf("Error setting up SIGINT handler\n");
        return 1;
    }

    // Create semaphore
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphore value to 1
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        exit(1);
    }

    // Create shared memory segment (or access if existed before)
    shmid = shmget(SHM_KEY, 1024, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach to the shared memory
    shared_mem = (char *) shmat(shmid, NULL, 0);
    if (shared_mem == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // loop until user press control + C (exit program)
    while(1){
        // Write data to shared memory
        printf("Enter a message to write to shared memory: ");
        fgets(input, 1024, stdin);

        // Remove newline at the end of input
        if(input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

        sem_operation(semid, -1); // P() (wait)
        strcpy(shared_mem, input);
        // fgets(shared_mem, 1024, stdin);

        sem_operation(semid, 1); // V() (signal)

        printf("Message written to shared memory: %s\n", shared_mem);
    }

    return 0;
}
