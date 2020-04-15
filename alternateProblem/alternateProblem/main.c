/*=============================================================================
|   Assignment:  Project 2, Problem 1, Variation
|
|       Author:  Charles Owen, Aniqa Rahim
|
|     Language:  C99 Standard
|
|   To Compile:  Run included make file
|
|        Class:  Computer Systems
|
|   Instructor:  Maria Striki
|
|     Due Date:  4/15/20
|
+-----------------------------------------------------------------------------*/
/*
 | This looks similar to the previous solution, that's because it reuses
 | many of the same "helper functions" to solve the problem. But that's where
 | the similarities end. This solution uses threads, and therefore needed to
 | completely reinvent the use and manipulation of data structures to account
 | for the shared memory space.
*/
 
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>

#define INTME 1000                          //Set the number of integers here
#define OUTPUTFILENAME "proj2Prob1.txt"       //Set the projects output file name here
#define NUMTHREADS 4

 
//global variable for left-most value of array
int leftMost = 0;
int rightMost = 0;

//Since pthread_t is not a system portable method of getting and printing
//thread IDs, we will use this counter to establish system agnostic
//IDs for our threads.
int globalThreadCount = 1;

// Counter for found hidden keys
int foundHidden = 0;

/*
 Struct to store all necessary data passed to, or
 required from a thread.
 */
struct partitionVal{
    int left;
    int right;
    int *arrToPass;
    int threadMax;
    int threadID;
};

struct partitionVal arrPart[NUMTHREADS];

void *threadShread(void * vargp);
void createIntFile(int howMany);
void shuffleArray(int arr[]);
void placeKeyValues(int arr[]);
void swap(int *a, int *b);
void writeMyID(void);
void writeTime(double time);
void writeKey(struct partitionVal *, int i);
void partition(struct partitionVal *part);
int* readIntFile(int howMany, int arr[]);
void findKeyValues(struct partitionVal *);

/*
 Once array is partitioned, pass partition bounds and pointer to array
 of ints. Thread finds max and identifies itself.
 */
void *threadShread(void * vargp){
    struct partitionVal *sentData = (struct partitionVal*)vargp;
    sentData->threadID = globalThreadCount++;
    return NULL;
}
/*
 The parent will run this function N children
 number of times to get the array partition
 values.
 */
void partition(struct partitionVal *part){
    //calculate the current rightmost value based on current leftmost
    rightMost = (leftMost + (INTME/NUMTHREADS)-1);
    //set struct values to reflect this iterations values
    part->left = leftMost;
    part->right = rightMost;
    //update the global value of leftMost to reflect this partition
    leftMost = rightMost+1;
}
/*
 Utility function to write the pid and location of
 the key values.
 */
void writeKey(struct partitionVal *threadDataWrite, int i){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Hi I'm Pthread %d and I found the hidden key in position arr[%d]\n", threadDataWrite->threadID, i);
    }
    fclose(idWrite);
}
/*
 Function produces the requested file output for the first section
 of the output file. Set to append.
 */
void writeMyID(){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Hi I'm process %d and my parent is %d\n", getpid(), getppid());
    }
    fclose(idWrite);
}
/*
 Appends the maximum value to the output file.
 */
void writeTime(double time){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Time Elapsed = %f\n", time);
    }
    fclose(idWrite);
}
/*
 This function finds the key values, then writes
 to the pid's and location to the output file
 */
void findKeyValues(struct partitionVal *threadData){
    for(int i = threadData->left; i <= threadData->right; i++){
            if(threadData->arrToPass[i] == -50){
                foundHidden++;
                printf("Found key at : %d\n", i);
                writeKey(threadData, i);
            }
    }
}
/*
Create our list of non-repeating integers from 0 to INTME-1
To change the number of integers created, edit the INTME define above
*/
void createIntFile(int howMany){
    
    int i;
    FILE *fptr = fopen("integerList.txt", "w");
    if(fptr == NULL){
        printf("Error opening file");
        return;
    }
    for(i = 0; i < howMany; i++){
        fprintf(fptr, "%d\n", i);
    }
    fclose(fptr);
    return;
}

/*
Loads the integers back into an array in the program.
Note that we allocate the array in the main function, then
pass it to this function. That way the function persists
throughout execution. The dynamically allocated array is
also tied to the INTME definition above.
*/
int* readIntFile(int howMany, int arr[]){
    
    int i;  //just a couple of iterators
    int k = 0;
    
    FILE *fptr = fopen("integerList.txt", "r");
    if(fptr == NULL){
        printf("Error opening file");
        return 0;
    }
    while(fscanf(fptr, "%d", &i) != EOF){
        arr[k++] = i;
        //printf("%d\n", arr[k-1]); Used for debugging purposes
    }
    fclose(fptr);
    return arr;
}

/*
This is a simple swap utility function,
it's primarily used for the array shuffle function.
*/
void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

/*
This shuffles the array after we read it back into
our array. This was written with reference from geeksforgeeks.com
*/
void shuffleArray(int arr[]){
    srand(time(NULL));
    for(int i = INTME; i > 0; i--){
        int j = rand() % (i+1);
        swap(&arr[i], &arr[j]);
    }
}

/*
This function places our key values as requested
by the project brief. Since the continuity of values in the
integer list isn't important we just overwrite whatever
integer happens to be at that spot in the array.
As a consequence, sometimes the largest value in the array
will be overwritten and value lower than INTME-1 will be max.
*/
void placeKeyValues(int arr[]){
    arr[INTME/4] = -50;
    arr[INTME/2] = -50;
    arr[(INTME/4)*3] = -50;
}

int main(int argc, const char * argv[]) {

//int *ptr = &foundHidden;
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    /*
     This section creates the random integer files
     then reads the integers back into an array.
     You can change the integer values above
     via the INTME define
     We also shuffle the elements and then insert the key, -50, values
     */
    int *arrptr;
    createIntFile(INTME);
    arrptr = (int*)malloc(INTME * sizeof(int));
    readIntFile(INTME, arrptr);
    shuffleArray(arrptr);                         //randomize the position of elements in our array
    placeKeyValues(arrptr);                       //position the key values as requested by brief
    
    /*
     Now the main thread search logic!
     We're going to populate our array of thread data structs, arrParts, with the following:
     The left and right boundary that the thread should search.
     A point to the array common to all threads. This is OK because each thread only searches
       within it's designated memory lane.
     */
    pthread_t thread_id[NUMTHREADS];
    for(int i = 0; i < NUMTHREADS; i++){                               //create the partitions and
        partition(&arrPart[i]);
        arrPart[i].arrToPass = arrptr;
        pthread_create(&thread_id[i], NULL, threadShread, (void*)&arrPart[i]);
    }
      /*
    Now find hidden keys inside the threads!
    We're scanning each member of the thread structure array
    until we find 3 threads.
     */
    for(int i = 0; i < NUMTHREADS; i++){
        if(foundHidden == 3){       //if all hidden keys have already been found
            printf("Terminated key at position: %d\n", i);
            pthread_exit(&thread_id[i]); //terminates threads
        } else{
            findKeyValues(&arrPart[i]);
            printf("foundHidden : %d\n", foundHidden);
            pthread_join(thread_id[i], NULL);
        }
    }
    /*
     Now, we wrap up the clock and send time to
     our writeTime helper function.
     */
    end = clock();
    cpu_time_used = ((double)(end-start)/CLOCKS_PER_SEC);
    writeTime(cpu_time_used);
    return 0;
}

