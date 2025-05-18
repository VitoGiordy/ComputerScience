#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../lib-misc.h"

typedef struct{
    sem_t s[3];
    pthread_mutex_t lock;
}defMemory;

typedef struct{
    pthread_t pid;
    int counterThread;
    defMemory* memoryThread;
}defThread;

void* threadFunction(void* args){
    defThread* data = (defThread*) args;

    sem_wait(&data->memoryThread->s[data->counterThread]);
    printf("I am the %dst Thread, I'm in the first semaphore, I'm waiting the mutex\n\n", data->counterThread+1);
    sleep(3);
    pthread_mutex_lock(&data->memoryThread->lock);
    printf("I'm in the critical region\n\n", data->counterThread);
    sleep(3);
    pthread_mutex_unlock(&data->memoryThread->lock);
    printf("I'm outside the critical region\n\n");
    sleep(3);
    sem_post(&data->memoryThread->s[(data->counterThread+1)%3]);

    return NULL;
}

defMemory* initMemory(){
    defMemory* memoInit = malloc(sizeof(defMemory));

    if(memoInit == NULL){                   //
        exit_with_sys_err("Malloc error");  //
    }                                       //

    int r = pthread_mutex_init(&memoInit->lock, NULL);  //
    if(r != 0){                                         //
        exit_with_err("Mutex error",r);                 //
    }                                                   //

    sem_init(&memoInit->s[0], 0, 1);
    for(int i=1;i<3;i++){
        sem_init(&memoInit->s[i], 0, 0);
    }

    return memoInit;
}

int main(){
    defThread t[3];
    defMemory* memoryMain = initMemory();

    for(int i=0;i<3;i++){
        t[i].counterThread = i;
        t[i].memoryThread = memoryMain;
        pthread_create(&t[i].pid, NULL, threadFunction, (void*)&t[i]);
    }

    for(int i=0;i<3;i++){
    pthread_join(t[i].pid, NULL);
    }
    
    for(int i=0;i<3;i++){
        sem_destroy(&memoryMain->s[i]);
    }

    pthread_mutex_destroy(&memoryMain->lock);

    free(memoryMain);

    return 0;
}