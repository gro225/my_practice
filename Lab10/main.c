#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define ARRAY_SIZE 10      
#define NUM_READERS 10     // Количество читающих потоков
#define WRITE_INTERVAL 1   // Интервал записи в секундах

char shared_array[ARRAY_SIZE + 1];  
pthread_mutex_t mutex;              // Мьютекс для синхронизации
pthread_cond_t cond;                
int counter = 0;                    
int current_tid = 0;                

// Функция пишущего потока
void* writer_thread() {
    while (1) {
        pthread_mutex_lock(&mutex);
        snprintf(shared_array, ARRAY_SIZE + 1, "%010d", counter++);

        current_tid = 0;  
        pthread_cond_broadcast(&cond);  

        pthread_mutex_unlock(&mutex);
        sleep(WRITE_INTERVAL); 
    }
    return NULL;
}

// Функция читающих потоков
void* reader_thread(void* arg) {
    int tid = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&mutex);



        while (tid != current_tid) {
            pthread_cond_wait(&cond, &mutex);
        }


        printf("Reader TID %d: [%s]\n", tid, shared_array);

        
        current_tid++;
        pthread_cond_broadcast(&cond);

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    int tids[NUM_READERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&writer, NULL, writer_thread, NULL);

   
    for (int i = 0; i < NUM_READERS; i++) {
        tids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &tids[i]);
    }


    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
