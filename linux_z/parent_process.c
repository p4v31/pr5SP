#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number_of_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    int nChildren = atoi(argv[2]);

    if (nChildren < 1) {
        fprintf(stderr, "Number of children must be at least 1.\n");
        exit(EXIT_FAILURE);
    }

    // Reading numbers from file
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int number, count = 0;
    while (fscanf(file, "%d", &number) == 1) {
        count++;
    }
    rewind(file);

    if (nChildren > count) {
        nChildren = count;
        printf("Adjusting number of children to %d due to count of numbers.\n", nChildren);
    }

    int numbersPerChild = count / nChildren;
    int extraNumbers = count % nChildren;

    pid_t pids[nChildren];
    char childFilename[BUFFER_SIZE];

    for (int i = 0; i < nChildren; ++i) {
        snprintf(childFilename, sizeof(childFilename), "%s_%d", filename, i);

        FILE *childFile = fopen(childFilename, "w");
        if (!childFile) {
            perror("Error creating child file");
            exit(EXIT_FAILURE);
        }

        int limit = numbersPerChild + (i < extraNumbers ? 1 : 0);
        for (int j = 0; j < limit; ++j) {
            if (fscanf(file, "%d", &number) != 1) break;
            fprintf(childFile, "%d\n", number);
        }
        fclose(childFile);

        pids[i] = fork();
        if (pids[i] == 0) {
            execl("./child_process", "child_process", childFilename, NULL);
            perror("Error in execl");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    // Добавьте задержку перед тем как идти дальше
    sleep(30); // Задержка для наблюдения зомби-процессов

    // Закомментировано для создания зомби-процессов
    // long totalSum = 0;
    // int status;
    // for (int i = 0; i < nChildren; ++i) {
    //     waitpid(pids[i], &status, 0);
    //     snprintf(childFilename, sizeof(childFilename), "%s_%d_result", filename, i);

    //     FILE *resultFile = fopen(childFilename, "r");
    //     if (!resultFile) {
    //         perror("Error opening result file");
    //         continue;
    //     }

    //     long sum;
    //     if (fscanf(resultFile, "%ld", &sum) == 1) {
    //         totalSum += sum;
    //     }
    //     fclose(resultFile);
    //     remove(childFilename); // Clean up
    // }

    // printf("
    // printf("Total sum of squares: %ld\n", totalSum);

    // Задержка перед завершением, чтобы дать наблюдателю время увидеть зомби-процессы
    sleep(10);

    return EXIT_SUCCESS;
}