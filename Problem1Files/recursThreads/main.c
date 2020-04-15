/*=============================================================================
|   Assignment:  Project 2, Problem 1, Objective 2
|
|       Author:  Charles Owen
|
|     Language:  C99 Standard
|
|   To Compile:  Run included make file
|
|        Class:  Computer Systems
|
|   Instructor:  Maria Striki
|
|     Due Date:  In the recent past
|
+-----------------------------------------------------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>
/*
 User inputs the size of array "chunks" to work on
 system will recursively create threads until size
 constraints are met.
 */
#define ARRAYCHUNKSIZE 500
#define INTME 1000        //Number of integers to process
#define OUTPUTFILENAME "proj2Prob1Obj2.txt"
#define NUMCALLS (INTME/ARRAYCHUNKSIZE)
     
//global variable for left-most value of array
int leftMost = 0;
int rightMost = 0;

/*
 Struct to store all necessary data passed to, or
 required from a thread.
 */
struct partitionVal{
    int left;
    int right;
    int *arrToPass;
    int threadMax;
    pthread_t threadID;
    int keySpot;
};

struct partitionVal threadData[NUMCALLS]; //Array of thread data, stores all the GOODS!

void recursThreads(int numThreads);
void *threadShred(void*);
void createIntFile(int howMany);
void shuffleArray(int arr[]);
void placeKeyValues(int arr[]);
int* readIntFile(int howMany, int arr[]);
void swap(int *a, int *b);
void partition(struct partitionVal *part);
void findMax(struct partitionVal *);
int  findMaxOfMax(void);
void findKeyValues(struct partitionVal *);
void writeKey(struct partitionVal *, int i);
void writeMax(int max, double time);

/*
 Appends the maximum value to the output file.
 */
void writeMax(int max, double time){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Max = %d\n", max);
        fprintf(idWrite, "Time Elapsed = %f\n", time);
    }
    fclose(idWrite);
}

/*
 Utility function to write the pid and location of
 the key values
 */
void writeKey(struct partitionVal *threadDataWrite, int i){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Hi I'm Pthread %d and I found the hidden key in position arr[&d] %d\n", threadDataWrite->threadID, i);
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
            printf("Found key at : %d\n", i);
            writeKey(threadData, i);
        }
    }
}

/*
 This function finds the maximum value in an array
 within the bounds passed as arguments.
 */
void findMax(struct partitionVal *maxData){
    int temp = 0;
    for(int i = maxData->left; i < maxData->right; i++){
        if(maxData->arrToPass[i] > temp)
            temp = maxData->arrToPass[i];
    }
    maxData->threadMax=temp;
}

/*
 The parent will run this function N children
 number of times to get the array partition
 values.
 */
void partition(struct partitionVal *part){
    //calculate the current rightmost value based on current leftmost
    rightMost = (leftMost + (INTME/NUMCALLS)-1);
    //set struct values to reflect this iterations values
    part->left = leftMost;
    part->right = rightMost;
    //update the global value of leftMost to reflect this partition
    leftMost = rightMost+1;
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
This is a simple swap utility function,
it's primarily used for the array shuffle function.
*/
void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
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

/*
 This is the function passed to each new thread. Here, we obtain the thread's ID,
 then we get the array partition this thread should work on, after that we get the Max
 from the array within our thread's bound, finally we see if the key value is within out
 bounds
 */
void *threadShred(void* arg){
    struct partitionVal *sentData = (struct partitionVal*)arg;
    sentData->threadID = pthread_self();
    partition(sentData);
    findMax(sentData);
    findKeyValues(sentData);
    return 0;
}

/*
 This function does the actual recursive creation of threads. The number of
 recursive calls is relative to the the size of "chunks" of data that the user
 selects by altering the definition at the top of the code.
 */
void recursThreads(int numThreads){
    pthread_t thread_id;
    //Recursive base case, no more chunks to create
    if(numThreads != 0){
    //This call moves backwards through the threadData array, but that's OK
    pthread_create(&thread_id, NULL, &threadShred, (void*)&threadData[numThreads-1]);
    /*
     Recursive thread-creation call! The meat and potatoes as they say!
     */
    recursThreads(numThreads-1);
        //Now we wait for threads to finish
        for(int i = 0; i < NUMCALLS; i++){
            pthread_join(thread_id, NULL);
        }
    }
}

int findMaxOfMax(void){
    int maxTemp = 0;
    for(int i = 0; i < NUMCALLS; i++){
        if(threadData[i].threadMax > maxTemp)
            maxTemp = threadData[i].threadMax;
    }
    return maxTemp;
}


int main(int argc, const char * argv[]) {
    
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
    
    for(int i = 0; i < NUMCALLS; i++){
        threadData[i].arrToPass = arrptr;
    }
    
    //Initiates recursive thread creation
    recursThreads(NUMCALLS);
    
    end = clock();
    cpu_time_used = ((double)(end-start)/CLOCKS_PER_SEC);
    
    writeMax(findMaxOfMax(), cpu_time_used);
    
}
