// Saif Sarhan
// 110067646
// 3300
// 2/27/2024
// H4
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#define NUM_CHILDREN 5

// aid function for bubble sort
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

// Function to calculate the persistence of a number
int calculatePersistence(int num) {
    int persistence = 0;

    while (num >= 10) {
        int result = 1;

        // Calculate the product of digits
        while (num > 0) {
            result *= num % 10;
            num /= 10;
        }

        num = result;
        persistence++;
    }

    return persistence;
}

// Function to handle child process
void childProcess(int childNum, int read_fd, int write_fd, int sortedList[], int count) {
    int token;
    int nextToken = childNum + 1;
    read(read_fd, &token, sizeof(token)); // Read token from parent

    if (token == childNum) {
        int start = (childNum - 1) * (count / NUM_CHILDREN);
        int end = childNum * (count / NUM_CHILDREN);
        
        if (childNum == NUM_CHILDREN)
            end = count;

        int minPersistence = calculatePersistence(sortedList[start]);
        int maxPersistence = minPersistence;
        int minNum = sortedList[start];
        int maxNum = minNum;

        for (int i = start + 1; i < end; i++) {
            int persistence = calculatePersistence(sortedList[i]);
            if (persistence < minPersistence) {
                minPersistence = persistence;
                minNum = sortedList[i];
            }
            if (persistence > maxPersistence) {
                maxPersistence = persistence;
                maxNum = sortedList[i];
            }
        }

        printf("I am kid #%d and my id is: %d Min Persistence: %d, Number: %d - Max Persistence: %d, Number: %d\n",
               childNum, getpid(), minPersistence, minNum, maxPersistence, maxNum);
        
        // Pass token to the next child
        write(write_fd, &nextToken, sizeof(nextToken));
    } else {
        write(write_fd, &token, sizeof(token));
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL); // Disable buffering for stdout

    // Record start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    // Read numbers from the file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error");
        exit(1);
    }

    int num;
    int count = 0;
    while (fscanf(file, "%d", &num) == 1) {
        count++;
    }

    fseek(file, 0, SEEK_SET);

    int *numbers = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        fscanf(file, "%d", &numbers[i]);
    }

    fclose(file);

    // Sort the list
    bubbleSort(numbers, count);

    // Create pipes
    int pipes[NUM_CHILDREN][2];
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Error creating pipe");
            exit(1);
        }
    }

    // Fork off five child processes
    pid_t childPids[NUM_CHILDREN];
    int token = 1; // Start with token 1

    // Print parent's children IDs
    printf("I am the father of the following: ");
    for (int i = 0; i < NUM_CHILDREN; i++) {
        childPids[i] = fork();

        if (childPids[i] == 0) {
            // Child process
            close(pipes[i][1]); // Close write end
            close(pipes[(i + 1) % NUM_CHILDREN][0]); // Close read end of next child
            childProcess(i + 1, pipes[i][0], pipes[(i + 1) % NUM_CHILDREN][1], numbers, count);
        } else if (childPids[i] < 0) {
            perror("Error forking process");
            exit(1);
        }
    }
     for (int i = 0; i < NUM_CHILDREN -1; i++) {
        printf("%d", childPids[i]);
        if (i < NUM_CHILDREN - 2) {
            printf(", ");
        }
    }
    printf(" And ");
    printf("%d",childPids[4]);
    printf("\n");

    // Pass token to the first child
    close(pipes[0][0]); // Close read end
    write(pipes[0][1], &token, sizeof(token));

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_CHILDREN; i++) {
        int status;
        waitpid(childPids[i], &status, 0);
        printf("From the main: child, id = %d ended with status %d\n", childPids[i], status);
    }

    printf("Good bye!\n");

    // Record end time
    gettimeofday(&end_time, NULL);

    // Calculate CPU time
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    // Print CPU time and terminate
    printf("Total CPU time: %f seconds\n", elapsed_time);

    free(numbers);
    return 0;
}