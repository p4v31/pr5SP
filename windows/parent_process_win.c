#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUFFER_SIZE 1024

void DistributeDataAmongChildren(const char* filename, int nChildren) {
    FILE* inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int totalNumbers = 0, number;
    while (fscanf(inFile, "%d", &number) == 1) totalNumbers++;
    fseek(inFile, 0, SEEK_SET);

    int numbersPerChild = totalNumbers / nChildren;
    int extraNumbers = totalNumbers % nChildren;

    for (int i = 0; i < nChildren; i++) {
        char childFilename[BUFFER_SIZE];
        sprintf(childFilename, "%s_%d", filename, i);

        FILE* childFile = fopen(childFilename, "w");
        if (!childFile) {
            fprintf(stderr, "Error creating file for child process: %s\n", childFilename);
            fclose(inFile);
            exit(EXIT_FAILURE);
        }

        int limit = numbersPerChild + (i < extraNumbers ? 1 : 0);
        for (int j = 0; j < limit; j++) {
            if (fscanf(inFile, "%d", &number) != 1) break;
            fprintf(childFile, "%d\n", number);
        }
        fclose(childFile);
    }
    fclose(inFile);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number_of_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    int nChildren = atoi(argv[2]);

    DistributeDataAmongChildren(filename, nChildren);

    STARTUPINFO si[nChildren];
    PROCESS_INFORMATION pi[nChildren];
    char cmdLine[BUFFER_SIZE];

    for (int i = 0; i < nChildren; i++) {
        ZeroMemory(&si[i], sizeof(si[i]));
        si[i].cb = sizeof(si[i]);
        ZeroMemory(&pi[i], sizeof(pi[i]));

        snprintf(cmdLine, BUFFER_SIZE, "child_process_win.exe \"%s_%d\" %d", filename, i, i);

        if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si[i], &pi[i])) {
            fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            exit(EXIT_FAILURE);
        }
    }

    long totalSum = 0;
    for (int i = 0; i < nChildren; i++) {
        WaitForSingleObject(pi[i].hProcess, INFINITE);
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);

        char resultFilename[BUFFER_SIZE];
        sprintf(resultFilename, "%s_%d_result", filename, i);

        FILE* resultFile = fopen(resultFilename, "r");
        if (!resultFile) {
            fprintf(stderr, "Error opening result file: %s\n", resultFilename);
            continue;
        }

        long sum = 0;
        fscanf(resultFile, "%ld", &sum);
        totalSum += sum;
        fclose(resultFile);
        remove(resultFilename); // Удаляем файл с результатами после чтения

        char childFilename[BUFFER_SIZE];
        sprintf(childFilename, "%s_%d", filename, i);
        remove(childFilename); // Удаляем файл, созданный для дочернего процесса
    }

    printf("Total sum of squares: %ld\n", totalSum);

    return EXIT_SUCCESS;
}
