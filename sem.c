#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <unistd.h>

int run;
sem_t mutex;

#define MAX_BUFFER_SIZE 10

int buffer[MAX_BUFFER_SIZE];
int write_index = 0;
int read_index = 0;
int buffer_size = 0;

void creator()
{
    while(run)
    {
        int value = rand() % 100;
        sem_wait(&mutex);
        if(buffer_size == MAX_BUFFER_SIZE || !run)
        {
            sem_post(&mutex);
            continue;
        }
        buffer[write_index] = value;
        write_index = (write_index + 1) % MAX_BUFFER_SIZE;
        buffer_size++;
        sem_post(&mutex);
        printf("Wrote %d to buffer\n", value);
        usleep(10000);
    }
}

void consumer()
{
    while(run)
    {
        sem_wait(&mutex);
        if(buffer_size == 0 || !run)
        {
            sem_post(&mutex);
            continue;
        }
        int value = buffer[read_index];
        read_index = (read_index + 1) % MAX_BUFFER_SIZE;
        buffer_size--;
        sem_post(&mutex);
        printf("Read %d from buffer\n", value);
        usleep(100000);
    }
}

int main()
{
    pthread_t *thread_creator, *thread_consumer;
    thread_creator = malloc(sizeof(pthread_t));
    thread_consumer = malloc(sizeof(pthread_t));

    sem_init(&mutex, 0, 1);
    srand(time(NULL));

    run = 1;
    pthread_create(thread_creator, NULL, (void*)creator, NULL);
    pthread_create(thread_consumer, NULL, (void*)consumer, NULL);

    while(1)
    {
        usleep(100);
        if(getchar() == 'q')
        {
            run = 0;
            break;
        }
    }

    pthread_join(*thread_creator, NULL);
    pthread_join(*thread_consumer, NULL);

    free(thread_creator);
    free(thread_consumer);

    return 0;
}