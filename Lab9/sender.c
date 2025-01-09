#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
      fprintf(stderr, "Sender is already running. Exiting.\n");
        return 1;
    }

    // Инициализация семафоров
    semctl(sem_id, 0, SETVAL, 0); // Семафор для чтения 
    semctl(sem_id, 1, SETVAL, 1); // Семафор для записи

    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Sender process started. PID: %d\n", getpid());
    while (1) {
        sem_op(sem_id, 1, -1); // Ждём пока память будет готова для записи

        char time_str[64];
        get_current_time(time_str, sizeof(time_str));

        snprintf(shared_data->message, sizeof(shared_data->message), "Time: %s, Sender PID: %d", time_str, getpid());
        shared_data->sender_pid = getpid();

        sem_op(sem_id, 0, 1); 

        sleep(1);
    }

   
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    semctl(sem_id, 1, IPC_RMID);

    return 0;
}
