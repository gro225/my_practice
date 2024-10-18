#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void exit_handler() {
    printf("Программа завершена.\n");
}

void signal_handler(int signum) {
    printf("Получен сигнал %s\n", strsignal(signum));
    exit(signum);
}

int main() {
    if (atexit(exit_handler) != 0) {
        perror("Ошибка atexit");
        return 1;
    }

    if (signal(2, signal_handler) == (void (*)(int))-1) {
        perror("Не удалось обработать SIGINT");
        exit(1);
    }

    struct sigaction sa ;
    sa.sa_handler = signal_handler;
    
    if (sigaction(15, &sa, 0) == -1) {
        perror("Не удалось обработать SIGTERM");
        exit(1);
    }
    

    pid_t pid = fork();

    if (pid < 0){
        perror("Ошибка вызова fork");
        return 1;
    } 
    else if (pid == 0) {
        printf("Это дочерний процесс с PID: %d\n", getpid());
    } 
    else {
        printf("Это родительский процесс с PID: %d, дочерний PID: %d\n", getpid(), pid);
    }

    // Цикл для ожидания сигналов
    while (1) {
        // int sys_pause(void);  
        // current_process->state = TASK_INTERRUPTIBLE;
        // schedule();  
        pause();
    }

    return 0;
}

