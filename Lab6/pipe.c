#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%H:%M:%S", localtime(&now));
}

int main() {
    int fd[2];
    pid_t pid;
    char buffer[1024];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    // Родитель
    if (pid > 0) {
        close(fd[0]);  
        char time_str[8];
        get_current_time(time_str, sizeof(time_str));

        snprintf(buffer, sizeof(buffer), "Time: %s, Parent PID: %d\n", time_str, getpid());
        write(fd[1], buffer, strlen(buffer));
        close(fd[1]);

        printf("Parent sent: %s", buffer);
        wait(NULL);
    }

    //Ребенок
    else { 
        sleep(5);  
        close(fd[1]); 
     
        read(fd[0], buffer, sizeof(buffer) - 1);
        buffer[strlen(buffer)] = '\0';

        char time_str[8];
        get_current_time(time_str, sizeof(time_str));
        printf("Child received: %sChild current time: %s\n", buffer, time_str);

        close(fd[0]);
    }

    return 0;
}
