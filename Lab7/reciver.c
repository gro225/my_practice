#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <time.h>
#include <sys/types.h>

#define SHM_KEY 12345

typedef struct {
    char message[128];
    pid_t sender_pid;
} SharedData;

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Receiver process started. PID: %d\n", getpid());
    while (1) {
        char time_str[64];
        get_current_time(time_str, sizeof(time_str));

        printf("Receiver PID: %d, Current Time: %s\n", getpid(), time_str);
        printf("Received: %s\n\n", shared_data->message);

        sleep(1);
    }

    shmdt(shared_data);

    return 0;
}
