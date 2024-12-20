#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>

#define SHM_KEY 12345
#define LOCK_FILE "/tmp/sender.lock"

typedef struct {
    char message[128];
    pid_t sender_pid;
} SharedData;

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

int main() {
    int lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
    if (lock_fd == -1) {
        perror("Failed to create lock file");
        return 1;
    }

    if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
        fprintf(stderr, "Sender is already running. Exiting.\n");
        return 1;
    }

    int shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Sender process started. PID: %d\n", getpid());
    while (1) {
        char time_str[64];
        get_current_time(time_str, sizeof(time_str));

        snprintf(shared_data->message, sizeof(shared_data->message), "Time: %s, Sender PID: %d", time_str, getpid());
        shared_data->sender_pid = getpid();

        sleep(1);
    }

    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    flock(lock_fd, LOCK_UN);
    close(lock_fd);
    unlink(LOCK_FILE);

    return 0;
}
