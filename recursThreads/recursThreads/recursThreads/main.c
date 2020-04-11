//  Created by charles owen on 4/10/20.
//  main.c
//  recursThreads
//
//
//
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>

#define NUMTHREADS 4
#define ARRAYDIVISIONS ??  //User inputs the number of times they'd like the array
                           //divided as criteria to create threads

void recursThreads(int numThreads);
void *threadShred(void*);

void *threadShred(void* arg){
    printf("From thread\n");
    return 0;
}

void recursThreads(int numThreads){
    pthread_t thread_id;
    if(numThreads != 0){  //So, here we have criteria that allows us to recurs, how can we apply it further...??
    pthread_create(&thread_id, NULL, &threadShred, NULL);
    /*
     Recursive thread creation call
     */
    recursThreads(numThreads-1);
        
        for(int i = 0; i < NUMTHREADS; i++){
            pthread_join(thread_id, NULL);
        }
    }
}

int main(int argc, const char * argv[]) {
    
    recursThreads(NUMTHREADS);
    
    
}
