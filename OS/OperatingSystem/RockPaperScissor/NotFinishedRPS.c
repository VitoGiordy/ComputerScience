#include "../lib-misc.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PARTITE 6


typedef enum{
    carta,
    forbice,
    sasso,
}mossa;

char* nome_mosse[] = {
    "carta",
    "forbice",
    "sasso",
};

typedef struct{
    mossa mossaP1;
    mossa mossaP2;
    int vincitore;
}Partita;

typedef struct{
    Partita partita;
    int partite_giocate;
    int vittorie_p1;
    int vittorie_p2;
    bool fine_torneo;

    sem_t sem_p1;
    sem_t sem_p2;
    sem_t sem_giudice;
    sem_t sem_tabellone;

}def_Memory;

Partita partita;

int mossa_P1_fatta;
int mossa_P2_fatta;

typedef struct{
    pthread_t pid;
    def_Memory* threadMemory;
}init_Thread;

def_Memory* initMemory(){
    def_Memory* functionMemory = malloc(sizeof(def_Memory));

    sem_init(&functionMemory->sem_p1, 0, 1);
    sem_init(&functionMemory->sem_p2, 0, 1);
    sem_init(&functionMemory->sem_giudice, 0, 0);
    sem_init(&functionMemory->sem_tabellone, 0, 0);

    return functionMemory;
}

int vittoria(int m1, int m2){
    if(m1 == m2){
        return 0;
    }
    if((m1 == sasso && m2 == forbice) 
    || (m1 == carta && m2 == sasso) 
    || (m1 == forbice && m2 == carta)){
        return 1;
    }
    else{
        return 2;
    }
}

void* thread_P1(void* args){
    init_Thread* data = (init_Thread*) args;

    while(1){

        sem_wait(&data->threadMemory->sem_p1);

        if(data->threadMemory->fine_torneo == 1){
            break;
        }

        partita.mossaP1 = rand() %3;
        printf("[P1] Gioca: %s\n", nome_mosse[partita.mossaP1]);
        mossa_P1_fatta = 1;

        sem_post(&data->threadMemory->sem_giudice);

    }

    return NULL;
}

void* thread_P2(void* args){
    init_Thread* data = (init_Thread*) args;

    while(1){

        sem_wait(&data->threadMemory->sem_p2);

        if(data->threadMemory->fine_torneo == 1){
            break;
        }

            partita.mossaP2 = rand() %3;
            printf( "[P2] Gioca: %s\n", nome_mosse[partita.mossaP2]);
            mossa_P2_fatta = 1;

        sem_post(&data->threadMemory->sem_giudice);

    }

    return NULL;
}

void* thread_Giudice(void* args){
    init_Thread* data = (init_Thread*) args;
    int count = 0;

    while(1){

    sem_wait(&data->threadMemory->sem_giudice);
    sem_wait(&data->threadMemory->sem_giudice);

        if(data->threadMemory->fine_torneo == 1){
            break;
        }

        count++;
        int vincitore = vittoria(partita.mossaP1, partita.mossaP2);

        if(vincitore == 1){
            data->threadMemory->vittorie_p1++;  
            printf(" [P1] vince la battaglia\n");

        }

        else if(vincitore == 2){
            data->threadMemory->vittorie_p2++;
            printf(" [P2] vince la battaglia\n");

        }

        else if(vincitore == 0){
            printf("La battaglia ha avuto un pareggio\n");

        }

        if(count >= MAX_PARTITE){
            sem_post(&data->threadMemory->sem_tabellone); 
            
        }

    sem_post(&data->threadMemory->sem_p1);
    sem_post(&data->threadMemory->sem_p2);

    }
    return NULL;
}

void* thread_Tabellone(void* args){
    init_Thread* data = (init_Thread*) args;

    while(1){

    sem_wait(&data->threadMemory->sem_tabellone);

            data->threadMemory->fine_torneo = 1;

        if(data->threadMemory->vittorie_p1 > data->threadMemory->vittorie_p2){
            printf(" \n[P1] ha vinto il gioco\n");
        }
        else if(data->threadMemory->vittorie_p1 < data->threadMemory->vittorie_p2){
            printf(" \n[P2] ha vinto il gioco\n]");
        }
        else if(data->threadMemory->vittorie_p1 == data->threadMemory->vittorie_p2){
            printf(" \nIl gioco finisce con un pareggio");
        }


        sem_post(&data->threadMemory->sem_giudice);

    }

    return NULL;
}


int main(){
    init_Thread P1;
    init_Thread P2;
    init_Thread Giudice;
    init_Thread Tabellone;

    def_Memory* memoryMain = initMemory();
    
    P1.threadMemory = memoryMain;
    P2.threadMemory = memoryMain;
    Giudice.threadMemory = memoryMain;
    Tabellone.threadMemory = memoryMain;

    pthread_create(&P1.pid, NULL, thread_P1, (void*)&P1);
    pthread_create(&P2.pid, NULL, thread_P2, (void*)&P2);
    pthread_create(&Giudice.pid, NULL, thread_Giudice, (void*)&Giudice);
    pthread_create(&Tabellone.pid, NULL, thread_Tabellone, (void*)&Tabellone);

    pthread_join(P1.pid, NULL);
    pthread_join(P2.pid, NULL);
    pthread_join(Giudice.pid, NULL);
    pthread_join(Tabellone.pid, NULL);

    sem_destroy(&memoryMain->sem_p1);
    sem_destroy(&memoryMain->sem_p2);
    sem_destroy(&memoryMain->sem_giudice);
    sem_destroy(&memoryMain->sem_tabellone);

    free(memoryMain);
}
