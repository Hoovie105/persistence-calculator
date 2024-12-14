//Saif Sarhan
//110067646
//HW #1
//1/29/2024
//3300

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
//same function from part 1 with some tweaks intended for child processes.
void childProcess(int childNum, int start, int end, int sortedList[]) {
    int minPersistence = calculatePersistence(sortedList[start]);
    int maxPersistence = minPersistence;
    int minNum = sortedList[start];
    int maxNum = minNum;

    for (int i = start; i < end; i++) {
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
    
    sleep(1);
    printf("I am kid #%d and my id is: %d Min Persistence: %d, Number: %d - Max Persistence: %d, Number: %d\n",
     childNum, getpid(), minPersistence, minNum, maxPersistence, maxNum);
    
    exit(0);
}

int main(int argc, char *argv[]) {
    
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

    // Fork off five child processes
    pid_t childPids[NUM_CHILDREN];
    int step = count / NUM_CHILDREN;
    int start = 0;
    int end;

    for (int i = 0; i < NUM_CHILDREN; i++) {
        end = (i == NUM_CHILDREN - 1) ? count : start + step;
        childPids[i] = fork();

        if (childPids[i] == 0) {
            // Child process
            childProcess(i + 1, start, end, numbers);
        } else if (childPids[i] < 0) {
            perror("Error forking process");
            exit(1);
        }

        start = end;
    }

    // Parent process
    printf("I am the father of the following: ");
    for (int i = 0; i < NUM_CHILDREN -1; i++) {
        printf("%d", childPids[i]);
        if (i < NUM_CHILDREN - 2) {
            printf(", ");
        }
    }
    printf(" And ");
    printf("%d",childPids[4]);
    printf("\n");

    // Wait for all child processes to finish
    sleep(2);
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
