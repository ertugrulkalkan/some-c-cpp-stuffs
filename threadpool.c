#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100
#define MAX_THREAD_CNT 4

int numbers_to_process[MAX_BUFFER_SIZE];
int write_index = 0;
int read_index = 0;
int buffer_size = 0;

void write_number(int number) {
    if (buffer_size == MAX_BUFFER_SIZE) {
        printf("Buffer is full!\n");
        return;
    }
    numbers_to_process[write_index++] = number;
    write_index %= MAX_BUFFER_SIZE;
    buffer_size++;
}

int read_number() {
    if (buffer_size == 0) {
        printf("Buffer is empty!\n");
        return 0;
    }
    int number = numbers_to_process[read_index++];
    read_index %= MAX_BUFFER_SIZE;
    buffer_size--;
    return number;
}

void process_number(int number) {
    printf("Processing number: %d, in queue: %d numbers left\n", number, buffer_size);
    sleep(number);
    printf("Done processing number: %d\n", number);
}

pthread_t *threads[MAX_THREAD_CNT];
int run = 1;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void loop()
{
    while (run)
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition, &mutex);
        if(buffer_size == 0)
        {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        int number = read_number();
        pthread_mutex_unlock(&mutex);
        process_number(number);
    }
}

void init_threads()
{
    for (int i = 0; i < MAX_THREAD_CNT; i++)
    {
        pthread_t *thread = malloc(sizeof(pthread_t));
        pthread_create(thread, NULL, (void *)loop, NULL);
        threads[i] = thread;
    }
}

void generate_numbers(int t)
{
    srand(time(NULL));
    for (int i = 0; i < t; i++)
    {
        int number = rand() % 10;
        write_number(number);
    }
}

int main()
{

    // generate 10 numbers to process
    generate_numbers(10);

    // init threads
    init_threads();

    // wait for threads to finish
    while(buffer_size > 0)
    {
        pthread_cond_broadcast(&condition);
        usleep(100);
    }

    // generate 50 more numbers to process
    generate_numbers(50);
    
    // wait for threads to finish
    while(buffer_size > 0)
    {
        pthread_cond_broadcast(&condition);
        usleep(100);
    }

    // stop threads
    run = 0;
    pthread_cond_broadcast(&condition);

    // join threads
    for (int i = 0; i < MAX_THREAD_CNT; i++)
    {
        pthread_join(*threads[i], NULL);
    }
    
    // free threads
    for (int i = 0; i < MAX_THREAD_CNT; i++)
    {
        free(threads[i]);
    }

    // exit
    return 0;
}