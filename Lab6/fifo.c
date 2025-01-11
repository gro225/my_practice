#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%H:%M:%S", localtime(&now));
}

int main() {
    pid_t pid;
    char buffer[1024];
    const char *filename = "/tmp/my_fifo";

    // Создаем FIFO
    if (mkfifo(filename, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    // Родитель
    if (pid > 0) {
        int fd = open(filename, O_WRONLY);
        if (fd == -1) {
            perror("open");
            return 1;
        }

        char time_str[8];
        get_current_time(time_str, sizeof(time_str));

        snprintf(buffer, sizeof(buffer), "Time: %s, Parent PID: %d\n", time_str, getpid());
        write(fd, buffer, strlen(buffer));
        close(fd);

        printf("Parent sent: %s", buffer);
        wait(NULL);

        unlink(filename);
    } 
    // Ребенок
    else {  
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            return 1;
        }

        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
        }
        sleep(5);
        char time_str[8];
        get_current_time(time_str, sizeof(time_str));
        printf("Child received: %sChild current time: %s\n", buffer, time_str);

        close(fd);
    }

    return 0;
}
