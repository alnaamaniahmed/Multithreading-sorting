#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXIMUM_INTS 500 // Maximum number of integers to be sorted
#define BUFFER_SIZE 4096 // Buffer size for reading from the file
// Structure to pass parameters to threads
typedef struct
{
    int starting_index;
    int ending_index;
    int middle;
    int* numbers;
} parameters;
// Comparison function for qsort
int cmpfunc (const void * a, const void *b) {
    return (*(int *)a - *(int *)b);
}
// Thread function for sorting a sublist
void* sorter(void * params){
    parameters*  p = (parameters *) params;
    qsort(p->numbers + p->starting_index, p->ending_index - p->starting_index + 1, sizeof (int), cmpfunc);
    pthread_exit(NULL);
}
// Thread function for merging two sorted sublists
void* merger(void * params){
    parameters* p = (parameters *) params;
    int sizeLeft  = p->middle - p->starting_index + 1;
    int sizeRight = p->ending_index - p->middle;
    int left[sizeLeft];
    int right[sizeRight];
    // Copy the sublists into the temporary arrays
    for(int i = 0; i < sizeLeft; i++){
        left[i] = p->numbers[p->starting_index + i];
    }
    for(int j = 0; j < sizeRight; j++){
        right[j] = p->numbers[p->middle + 1 + j];
    }
    // Merge the sublists back into the original array
    int i = 0;
    int j = 0;
    int k = p->starting_index;
    while(i < sizeLeft && j < sizeRight){
        if(left[i] <= right[j]){
            p->numbers[k] = left[i];
            i++;
        } else {
            p->numbers[k] = right[j];
            j++;
        }
        k++;
    }
    // Copy any remaining elements
    while (i < sizeLeft) {
        p->numbers[k] = left[i];
        i++;
        k++;
    }
    while (j < sizeRight){
        p->numbers[k] = right[j];
        j++;
        k++;
    }
    pthread_exit(NULL);
}

int main() {
    // Array to store the numbers read from the file
    int numbers[MAXIMUM_INTS];
    // Open the input file
    FILE *file = fopen("IntegerList.txt", "r");
    if(!file){
        perror("Error opening File");
        return 1;
    }
    // Read the entire line of numbers from the file
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, file);
    fclose(file);


    // Tokenize the string and convert to integers
    char *token = strtok(buffer, ",");
    int count = 0;
    while(token != NULL && count < MAXIMUM_INTS) {
        numbers[count++] = atoi(token);
        token = strtok(NULL, ",");
    }
    // Allocate and initialize structures for sorting sublists
    parameters *sublist1 = (parameters *) malloc(sizeof (parameters));
    parameters *sublist2 = (parameters *) malloc(sizeof (parameters));
    sublist1->numbers = numbers;
    sublist2->numbers = numbers;
    int middle = count/2;
    // Assign indices to sublists based on the number of elements
    if(count % 2 == 0){
        sublist1->starting_index = 0;
        sublist1->ending_index =  middle - 1;
        sublist2->starting_index = middle;
        sublist2->ending_index =  count - 1;
    } else {
        sublist1->starting_index = 0;
        sublist1->ending_index = middle;
        sublist2->starting_index = middle + 1;
        sublist2->ending_index = count - 1;
    }
    // Create and start sorting threads
    pthread_t tid_1, tid_2, tid_3;
    pthread_create(&tid_1, NULL, sorter, sublist1);
    pthread_create(&tid_2,NULL, sorter, sublist2);

    pthread_join(tid_1,NULL);
    pthread_join(tid_2,NULL);
    //Create and start merging thread
    parameters *mergedList = (parameters *) malloc(sizeof (parameters));
    mergedList->numbers = numbers;
    mergedList->starting_index = 0;
    mergedList->middle = sublist1->ending_index;
    mergedList->ending_index = count - 1;
    pthread_create(&tid_3, NULL, merger, mergedList);
    pthread_join(tid_3,NULL);

    //write the mergedList into the sortedList.txt
    FILE *output_file = fopen("SortedList.txt", "w");
    if(!output_file) {
        perror("Error opening file");
        return 1;
    }
    for(int i = 0; i < count; i++){
        fprintf(output_file, "%d", numbers[i]);
        if (i < count - 1) {
            fprintf(output_file, ", ");
        }
    }
    //free memory
    fclose(output_file);
    free(sublist1);
    free(sublist2);
    free(mergedList);
}
