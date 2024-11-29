#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define SHM_KEY 1234  // Unique key for shared memory

int shmid; // shared memory id
char *shared_mem; // map to the first address of shared memory with shmmat()

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
        fgets(shared_mem, 1024, stdin);

        // Remove newline at the end of input
        if(shared_mem[strlen(shared_mem) - 1] == '\n') shared_mem[strlen(shared_mem) - 1] = '\0';

        printf("Message written to shared memory: %s\n", shared_mem);
    }

    return 0;
}
