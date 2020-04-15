//
//  main.c
//  projProbTwo
//
//  Created by charles owen on 3/1/20.
//  Copyright © 2020 charles owen. All rights reserved.
// THIS IS THE PURE C VERSION!!

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define INTME 1000                                 //Set the number of intergers here, plz
#define OUTPUTFILENAME "projProbTwoOutput.txt"     //Set the projects output file name here
#define NUMCHILDREN 4                              //Set desired number of children

//global variable for left-most value of array
int leftMost = 0;
int rightMost = 0;
int flag = 1;      //Flag int for signaling children

//Since pthread_t is not a system portable method of getting and printing
//thread IDs, we will use this counter to establish system agnostic
//IDs for our threads.
int globalThreadCount = 1;

//globally defined struct
struct partitionVal{
    int left;
    int right;
};

void createIntFile(int howMany);
void shuffleArray(int arr[]);
void placeKeyValues(int arr[]);
void swap(int *a, int *b);
void writeMyID(void);
void writeMax(int max, double time);
void writeKey(int position);
void partition(struct partitionVal *part);
int* readIntFile(int howMany, int arr[]);
int findMax(int arr[], int left, int right);
void findKeyValues(int arr[], int left, int right);
int compare(int a, int b);

/*
 The parent will run this function N children
 number of times to get the array partition
 values.
 */
void partition(struct partitionVal *part){
    //calculate the current rightmost value based on current leftmost
    rightMost = (leftMost + (INTME/NUMCHILDREN)-1);
    //set struct values to reflect this iterations values
    part->left = leftMost;
    part->right = rightMost;
    //update the global value of leftMost to reflect this partition
    leftMost = rightMost+1;
}
/*
 Utility function to write the pid and location of
 the key values
 */
void writeKey(int position){
    FILE *idWrite = fopen(OUTPUTFILENAME, "a");
    if(idWrite == NULL){
        printf("Error opening file");
    }else{
        fprintf(idWrite, "Hi I'm process %d and I found the hidden key in position arr[&d] %d\n", getpid(), position);
    }
    fclose(idWrite);
}
/*
 Funciton produces the requested file output for the first section
 of the output file. Set to append, will not
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
 Simple utility function to compare two arguments
 and return the greater of the two.
 */
int compare(int a, int b){
    return (a > b) ? a : b;
}

/*
 This function finds the maximum value in an array
 within the bounds passed as arguments.
 */
int findMax(int arr[], int left, int right){
    int temp = 0;
    for(int i = left; i < right; i++){
        if(arr[i] > temp)
            temp = arr[i];
    }
    return temp;
}

/*
 This function finds the key values, then writes
 to the pid's and location to the output file
 */
void findKeyValues(int arr[], int left, int right){
    for(int i = left; i <= right; i++){
        if(arr[i] == -50){
            writeKey(i);
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
     Print out list to demonstrate randomization and key insertion
     */
    /*
    for(int i = 0; i < INTME-1; i++){
        printf("%d\n", arrptr[i]);
    }
     */
    
    //Going to try and use 2 processes and divide the work between them
    int parentToChild[2];
    int childToParent[2];
    int controlPipe[2];
    struct partitionVal myPartition;
    
    if(pipe(parentToChild) < 0 || pipe(childToParent)  || pipe(controlPipe)) //create two communication pipes
        return -1;
   
    pid_t pid = fork(); //FORK TIME!!
    
    if(pid < 0){        //Catch child creation error
       printf("Error creating child\n");
       return -1;
    }
    
    
   
    /*
     The parent enters this first section and becomes the controller.
     It writes its PID, then partitions the array into sections based on the number
     of children its working with.
     We create an array for the parent to accept the return maximum values from the children
     Once we get the values we perform a our own Max value search and write the value to a file.
     
     Finally, we send a control signal to the children to let them know they can now search for
     the key values and output to file.
     
     Wait until all children are finished before allowing the parent to exit.
     */
    
    if(pid){
        //Parent and control logic
        //writeMyID();
        for(int i = 0; i < NUMCHILDREN; i++){                               //create the partitions and write to children
            partition(&myPartition);
            write(parentToChild[1], &myPartition, sizeof(struct partitionVal));
        }
        int *agregateMax;
        agregateMax = (int*)malloc(NUMCHILDREN * sizeof(int));             //create an array to hold the returned max values
        for(int i = 0; i < NUMCHILDREN; i++){                              //get the max values back from the children
            read(childToParent[0], &agregateMax[i], sizeof(int));
        }
        end = clock();
        cpu_time_used = ((double)(end-start)/CLOCKS_PER_SEC);
        //printf("Time to execute: %f\n", cpu_time_used);
        writeMax(findMax(agregateMax, 0, NUMCHILDREN-1), cpu_time_used);                  //output max to file
        
        for(int i = 0; i < NUMCHILDREN; i++){                              //send control signal to children, start key search
            write(controlPipe[1], &flag, sizeof(int));
        }
        
        for(int i = 0; i< NUMCHILDREN; i++){                               //wait for children to each exit
            wait(NULL);
        }
        
        exit(0);
    }
    
    /*
     The first child will enter here and spawn the desired number of children, as input
     in the definition at the top of the program.
     
     */
    else{
        int childGoFlag;
        
        for(int i = 0; i < NUMCHILDREN/2; i++){                             //fork to desired num children
        fork();
        }
        
        //writeMyID();                                                       //output our pid to file
        
        struct partitionVal childPartition;                                       //create partition storing struct
        
        read(parentToChild[0], &childPartition, sizeof(struct partitionVal)); //receive partition values from control parent
        
        int childMax = findMax(arrptr, childPartition.left, childPartition.right); //find the max of each child's partition responsability
        
        write(childToParent[1], &childMax, sizeof(int));                   //send max to parent
        
        read(controlPipe[0], &childGoFlag, sizeof(int));                   //force wait until parent uses controlPipe
        
        findKeyValues(arrptr, childPartition.left, childPartition.right);  //find keys and write location to file
    }
        
    return 0;
}

