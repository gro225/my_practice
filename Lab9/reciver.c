#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>

#define SHM_KEY 12345
#define SEM_KEY 54321

typedef struct {
    char message[128];
    pid_t sender_pid;
} SharedData;

void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

void sem_op(int sem_id, int sem_num, int op) {
    struct sembuf sb = {sem_num, op, 0};
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    int sem_id = semget(SEM_KEY, 2, 0666);
    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Receiver process started. PID: %d\n", getpid());
    while (1) {
        sem_op(sem_id, 0, -1); // Ждём данных для чтения

        char time_str[64];
        get_current_time(time_str, sizeof(time_str));

        printf("Receiver PID: %d, Current Time: %s\n", getpid(), time_str);
        printf("Received: %s\n\n", shared_data->message);

        sem_op(sem_id, 1, 1); // Сообщаем, что память готова для записи

        sleep(1);
    }

    shmdt(shared_data);
    return 0;
}
