#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct{
    sem_t h[2];
}shared_memory;

typedef struct{
    pthread_t pid;
    int contatoreP;
    shared_memory* mem;
}pThreads;

void* funzioneContaThread(void* args){
    pThreads* data = (pThreads*) args;

    sem_wait(&data->mem->h[data->contatoreP]);
    printf("Numero di Thread= %d \n", data->contatoreP);
    sleep(1);
    sem_post(&data->mem->h[(data->contatoreP+1)%2]);

    return NULL;
}

shared_memory* init_memory(){
    shared_memory* memory = malloc(sizeof(shared_memory));

    sem_init(&memory->h[0], 0, 1);
    sem_init(&memory->h[1], 0, 0);

    return memory;
}

int main(){
    pThreads p[2];
    shared_memory* mem = init_memory();

    for(int i=0; i<2; i++){
        p[i].contatoreP = i;
        p[i].mem = mem;
        pthread_create(&p[i].pid, NULL, funzioneContaThread, (void*)&p[i]);
    }

    for(int i=0; i<2; i++){
        pthread_join(p[i].pid, NULL);
    }

    for(int i=0; i<2; i++){
        sem_destroy(&mem->h[i]);
    }

    free(mem);
}