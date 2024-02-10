#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <data_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int number;
    long sum = 0;
    while (fscanf(file, "%d", &number) == 1) {
        sum += number * number;
    }
    fclose(file);

    // Adding a sleep to ensure visibility of child processes
    sleep(5);

    char resultFilename[256];
    sprintf(resultFilename, "%s_result", argv[1]);

    file = fopen(resultFilename, "w");
    if (!file) {
        perror("Error opening result file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%ld", sum);
    fclose(file);

    return EXIT_SUCCESS;
}
