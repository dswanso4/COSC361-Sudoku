#ifndef SUDOKU_H
#define SUDOKU_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

// Constants

#define Column_Threads 3
#define Row_Threads 3
#define Block_Threads 3


#define SUDOKU_DEPTH 9

// Define VERBOSE to be 1 if you want all the debugging outputs
#define VERBOSE 0

// Definition of the used struct

typedef struct NumberArray{
    int **elements;
    int array_count;
    int isValid;
    int isTested;
} NumberArray;

// Function prototypes

int Test_Sudoku(char *);
int Test_Arrays(NumberArray *, NumberArray *, NumberArray *);
void *isValid(void *);
int *Copy_Row(int **, int);
int *Copy_Column(int **, int);
int *Copy_Block(int **, int);

void Initialize(NumberArray *, NumberArray *, NumberArray *);
void Free_Memory(NumberArray *, NumberArray *, NumberArray *);

int **Decode_Sudoku(char *);
void Print_Sudoku(int **);

int max(int, int);

#endif