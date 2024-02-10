package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

func main() {
	if len(os.Args) < 3 {
		fmt.Println("Использование: <программа> <имя файла> <количество процессов>")
		os.Exit(1)
	}

	// Режим работы дочернего процесса
	if len(os.Args) == 4 && os.Args[3] == "child" {
		processChild()
		return
	}

	fileName := os.Args[1]
	N, err := strconv.Atoi(os.Args[2])
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка: неверный формат количества процессов: %s\n", err)
		os.Exit(1)
	}

	bytes, err := ioutil.ReadFile(fileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка при чтении файла: %s\n", err)
		os.Exit(1)
	}

	numbersStr := strings.Fields(string(bytes))
	M := len(numbersStr)
	if M < 2 {
		fmt.Println("Файл должен содержать как минимум 2 числа.")
		os.Exit(1)
	}

	if N > M/2 {
		N = M / 2
		fmt.Println("Предупреждение: количество процессов было уменьшено до:", N)
	}

	numbers := make([]int, M)
	for i, s := range numbersStr {
		numbers[i], err = strconv.Atoi(s)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Ошибка при преобразовании числа: %s\n", err)
			os.Exit(1)
		}
	}

	execPath, err := os.Executable()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка получения пути исполняемого файла: %s\n", err)
		os.Exit(1)
	}
	execPath, err = filepath.Abs(execPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка получения абсолютного пути исполняемого файла: %s\n", err)
		os.Exit(1)
	}

	var wg sync.WaitGroup
	sum := 0
	mu := sync.Mutex{}

	for i := 0; i < N; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			start := i * (M / N)
			end := (i + 1) * (M / N)
			if i == N-1 {
				end = M
			}

			tempInputFile, err := ioutil.TempFile("", "input")
			if err != nil {
				fmt.Fprintf(os.Stderr, "Ошибка при создании временного файла: %s\n", err)
				os.Exit(1)
			}
			defer os.Remove(tempInputFile.Name())

			for _, num := range numbers[start:end] {
				tempInputFile.WriteString(fmt.Sprintf("%d\n", num))
			}
			tempInputFile.Close()

			cmd := exec.Command(execPath, tempInputFile.Name(), "1", "child")
			output, err := cmd.CombinedOutput()
			if err != nil {
				fmt.Fprintf(os.Stderr, "Ошибка при выполнении дочернего процесса: %s\n", err)
				os.Exit(1)
			}

			var partSum int
			fmt.Sscanf(string(output), "%d", &partSum)

			mu.Lock()
			sum += partSum
			mu.Unlock()
		}(i)
	}

	wg.Wait()

	fmt.Printf("Итоговая сумма квадратов: %d\n", sum)
}

func processChild() {
	fileName := os.Args[1]

	bytes, err := ioutil.ReadFile(fileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка при чтении файла: %s\n", err)
		os.Exit(1)
	}

	scanner := bufio.NewScanner(strings.NewReader(string(bytes)))
	sum := 0
	for scanner.Scan() {
		num, err := strconv.Atoi(scanner.Text())
		if err != nil {
			fmt.Fprintf(os.Stderr, "Ошибка при преобразовании числа: %s\n", err)
			os.Exit(1)
		}
		sum += num * num
	}

	// Приостанавливаем выполнение на 5 секунд
	time.Sleep(5 * time.Second)

	fmt.Println(sum)
}
