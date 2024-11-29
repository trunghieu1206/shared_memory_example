#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

#define SHM_KEY 1234  // Same key as used in writer.c

int shmid;
char *shared_mem;
char input[10];

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
    }

    return 0;
}
