#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data_file> <child_index>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Sleep(5000); // Пауза в 5 секунд

    const char* inputFile = argv[1];
    int childIndex = atoi(argv[2]);

    char resultFilename[260];
    sprintf(resultFilename, "%s_result", inputFile); // Убираем расширение .txt из имени файла результата

    FILE *file = fopen(inputFile, "r");
    if (!file) {
        fprintf(stderr, "Error opening input file: %s\n", inputFile);
        exit(EXIT_FAILURE);
    }

    long sum = 0;
    int number;
    while (fscanf(file, "%d", &number) == 1) {
        sum += number * number;
    }
    fclose(file);

    file = fopen(resultFilename, "w");
    if (!file) {
        fprintf(stderr, "Error creating result file: %s\n", resultFilename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%ld", sum);
    fclose(file);

    return 0;
}
