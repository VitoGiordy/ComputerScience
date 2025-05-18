#include "../lib-misc.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define NUM_PARTITE 10

typedef enum{
    CARTA,
    FORBICE,
    SASSO,
}mosse;

const char* nome_mosse[] = {
    "carta",
    "forbice",
    "sasso"
};

typedef struct{
    mosse mossa_p1;
    mosse mossa_p2;
    int vincitore;
    int partita_in_corso;
}Partita;

Partita partita;

typedef struct{
    pthread_mutex_t lock;
    pthread_cond_t cond_mossa_p1;
    pthread_cond_t cond_mossa_p2;
    pthread_cond_t cond_giudice;
    pthread_cond_t cond_tabellone;
}memoryData;

typedef struct{
    pthread_t pid;
    int cThreads;
    memoryData * memoryThreads;
}threadData;

int mossa_p1_fatta = 0;
int mossa_p2_fatta = 0;
int risultato_pronto = 0;
int partite_giocate = 0;
int vittorie_p1 = 0;
int vittorie_p2 = 0;


memoryData* initMemory(){
    memoryData* initMemory = malloc(sizeof(memoryData));

    pthread_mutex_init(&initMemory->lock, NULL);
    
    pthread_cond_init(&initMemory->cond_mossa_p1, NULL);
    pthread_cond_init(&initMemory->cond_mossa_p2, NULL);
    pthread_cond_init(&initMemory->cond_giudice, NULL);
    pthread_cond_init(&initMemory->cond_tabellone, NULL);
    
    return initMemory;
}

int decidiVincitore(mosse m1, mosse m2){
    if(m1 == m2){
        return 0;
    }

    if((m1 == SASSO && m2 == FORBICE) 
    || (m1 == CARTA && m2 == SASSO) 
    || (m1 == FORBICE && m2 == CARTA)){
        return 1;
    }
    return 2;
}

void* thread_p1(void* args){
    threadData* data = (threadData*) args;
    while(1){
        int r = pthread_mutex_lock(&data->memoryThreads->lock);
        if(r!=0){
            exit_with_err("Mutex error", r);
        }

        partita.mossa_p1 = rand() %3;
        mossa_p1_fatta = 1;
        pthread_cond_signal(&data->memoryThreads->cond_mossa_p1);
        pthread_mutex_unlock(&data->memoryThreads->lock);
    }
    return NULL;

}

void* thread_p2(void* args){
    threadData* data = (threadData*) args;

    while(1){
        int r = pthread_mutex_lock(&data->memoryThreads->lock);
        if(r!=0){
            exit_with_err("Mutex error", r);
        }

        partita.mossa_p2 = rand() %3;
        mossa_p2_fatta = 1;
        pthread_cond_signal(&data->memoryThreads->cond_mossa_p2);
        pthread_mutex_unlock(&data->memoryThreads->lock);

    }
    return NULL;
}

void* thread_giudice(void* args){
    threadData* data = (threadData*) args;
    while(1){
        pthread_mutex_lock(&data->memoryThreads->lock);
        while(!mossa_p1_fatta || !mossa_p2_fatta){
            pthread_cond_wait(&data->memoryThreads->cond_mossa_p1, &data->memoryThreads->lock);
            pthread_cond_wait(&data->memoryThreads->cond_mossa_p2, &data->memoryThreads->lock);
        }

        int vinc = decidiVincitore(partita.mossa_p1, partita.mossa_p2);
        partita.vincitore = vinc;

        if(vinc != 0){
            risultato_pronto = 1;
            pthread_cond_signal(&data->memoryThreads->cond_tabellone);
        }
        else{
            mossa_p1_fatta = 0;
            mossa_p2_fatta = 0;
        }
        pthread_mutex_unlock(&data->memoryThreads->lock);

        if(partite_giocate >= NUM_PARTITE){
            break;
        }
    }
    return NULL;
}

void* thread_tabellone(void* args){
    threadData* data = (threadData*) args;

    while(1){
        pthread_mutex_lock(&data->memoryThreads->lock);
        while(!risultato_pronto){
            pthread_cond_wait(&data->memoryThreads->cond_tabellone, &data->memoryThreads->lock);
        }

        printf("Partita %d:\n", partite_giocate + 1);
        printf(" P1: %s\n", nome_mosse[partita.mossa_p1]);
        printf(" P2: %s\n", nome_mosse[partita.mossa_p2]);
        printf(" Vincitore: Giocatore %d\n\n", partita.vincitore);

        if(partita.vincitore == 1){
            vittorie_p1++;
        }
        else if(partita.vincitore == 2){
            vittorie_p2++;
        }

        partite_giocate++;
        mossa_p1_fatta = 0;
        mossa_p2_fatta = 0;
        risultato_pronto = 0;

        pthread_mutex_unlock(&data->memoryThreads->lock);

        if(partite_giocate >= NUM_PARTITE){
            break;
        }

    }
    printf("Classifica finale:\n");
    printf("  Giocatore 1: %d vittorie\n", vittorie_p1);
    printf("  Giocatore 2: %d vittorie\n", vittorie_p2);

    if (vittorie_p1 > vittorie_p2)
        printf(">> Vincitore: Giocatore 1\n");
    else if (vittorie_p2 > vittorie_p1)
        printf(">> Vincitore: Giocatore 2\n");
    else
        printf(">> Pareggio\n");

    return NULL;
}


int main(){
    srand(time(NULL));
    
    threadData p1, p2, giudice, tabellone;

    memoryData* memory = initMemory();

    p1.memoryThreads = memory;
    p2.memoryThreads = memory;
    giudice.memoryThreads = memory;
    tabellone.memoryThreads = memory;


    pthread_create(&p1.pid, NULL, thread_p1, (void*)&p1.pid);
    pthread_create(&p2.pid, NULL, thread_p2, (void*)&p2.pid);
    pthread_create(&giudice.pid, NULL, thread_giudice, (void*)&giudice.pid);
    pthread_create(&tabellone.pid, NULL, thread_tabellone, (void*)&tabellone.pid);

    pthread_join(p1.pid, NULL);
    pthread_join(p2.pid, NULL);
    pthread_join(giudice.pid, NULL);
    pthread_join(tabellone.pid, NULL);

    pthread_mutex_destroy(&memory->lock);
    pthread_cond_destroy(&memory->cond_mossa_p1);
    pthread_cond_destroy(&memory->cond_mossa_p2);
    pthread_cond_destroy(&memory->cond_giudice);
    pthread_cond_destroy(&memory->cond_tabellone);

    free(memory);

}