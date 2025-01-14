#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define ARRAY_SIZE 10      
#define NUM_READERS 10     // Количество читающих потоков
#define WRITE_INTERVAL 1   // Интервал записи в секундах

char shared_array[ARRAY_SIZE + 1];  
pthread_rwlock_t rwlock;            // Блокировка чтения-записи
int counter = 0;                    
int current_tid = 0;                

// Функция пишущего потока
void* writer_thread() {
    while (1) {
        pthread_rwlock_wrlock(&rwlock); 

        snprintf(shared_array, ARRAY_SIZE + 1, "%010d", counter++);

     
        current_tid = 0;

        pthread_rwlock_unlock(&rwlock);  
        sleep(WRITE_INTERVAL); 
    }
    return NULL;
}

// Функция читающих потоков
void* reader_thread(void* arg) {
    int tid = *(int*)arg;

    while (1) {
        while (1) {
            if (tid == current_tid) {
                pthread_rwlock_rdlock(&rwlock);  
                printf("Reader TID %d: [%s]\n", tid, shared_array);
                pthread_rwlock_unlock(&rwlock);  
                
                current_tid++;
        
                break;  
            }
            usleep(1000);  
        }
    }
    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    int tids[NUM_READERS];

    pthread_rwlock_init(&rwlock, NULL);  

    pthread_create(&writer, NULL, writer_thread, NULL);

    for (int i = 0; i < NUM_READERS; i++) {
        tids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &tids[i]);
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    //Очистка
    pthread_rwlock_destroy(&rwlock);

    return 0;
}
