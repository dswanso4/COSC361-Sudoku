#include "sudoku.h"

/*
  Main function of the code. Checks a known correct sudoku puzzle and a known incorrect sudoku puzzle, 
  then allows the user to check a puzzle.
*/
int main(int argc, char *argv[])
{
	char given_sudoku[81];
	srand(time(NULL));
	
	char correct_sudokustring[] = "378145629149862753526397148835921476261473895794658312983514267617289534452736981";

	printf("Result: %s\n", (Test_Sudoku(correct_sudokustring) == 1) ? "Correct" : "Incorrect" );
	
	char incorrect_sudokustring[] = "103567894456189237789234156214356789365798412897412365532641978648973521977895643";

	printf("Result: %s\n", (Test_Sudoku(incorrect_sudokustring) == 1) ? "Correct" : "Incorrect" );

	printf("Input sudoku solution that you want to be validated.\n");
	scanf("%81s", given_sudoku);

	printf("Result: %s\n", (Test_Sudoku(given_sudoku) == 1) ? "Correct" : "Incorrect" );
	return 0;
}

/*
  Function that tests if a given string of numbers between 0 and 9 is a solution to for a suduko puzzle. The function
  returns 1 if it is a solution and 0 if not.
*/
int Test_Sudoku(char *sudokustring)
{
	int **given_sudoku;
	int result;

	// Returns an error if the string is not in the correct format
	if(strlen(sudokustring) != 81){
		printf("Invalid input size %d.\n", strlen(sudokustring));
		return -1;
	}

	// Decodes the string into a 9x9 array for ease of use.
	given_sudoku = Decode_Sudoku(sudokustring);
	Print_Sudoku(given_sudoku);


	// Declare/Initialize based on the number of threads
	NumberArray row_NumberArrays[Row_Threads];
	NumberArray column_NumberArrays[Column_Threads];
	NumberArray block_NumberArrays[Block_Threads];
	Initialize(row_NumberArrays, column_NumberArrays, block_NumberArrays);

	int i, row_cursor, col_cursor, block_cursor;

	i = 0;

	// Defines 9-element arrays to each of the structs, with rollover cursors that are dependent on amount of avialable structs.
	while(i < 9)
	{
		row_cursor = i % Row_Threads;
		col_cursor = i % Column_Threads;
		block_cursor = i % Block_Threads;

		if(VERBOSE) printf("i : %d. row_cursor: %d.\n", i, row_cursor);

		// Generates copies of the row, column, and block, and allocates the copies into the section of the struct for the current cursor position for row, column, and block.
		row_NumberArrays[row_cursor].elements[row_NumberArrays[row_cursor].array_count++] = Copy_Row(given_sudoku, i);
		column_NumberArrays[col_cursor].elements[column_NumberArrays[col_cursor].array_count++] = Copy_Column(given_sudoku, i);
		block_NumberArrays[block_cursor].elements[block_NumberArrays[block_cursor].array_count++] = Copy_Block(given_sudoku, i);

		i++;

		if(VERBOSE) printf("array_count: %d\n\n", row_NumberArrays[row_cursor].array_count);
	}

	// Result is set to be the solution for the function that actually checks if the solution works.
	result = Test_Arrays(row_NumberArrays, column_NumberArrays, block_NumberArrays);


	// Free the memory for the system.
	Free_Memory(row_NumberArrays, column_NumberArrays, block_NumberArrays);
	for(i = 0; i < 9; i++)
		free(given_sudoku[i]);
	free(given_sudoku);

	return result;
}

/*
  Function that runs all the rows, columns, and blocks through the isValid() checker function. If any of those calls return 0, 
  then the solution does not work and this function returns 0, Otherwise it returns 1.
*/

int Test_Arrays(NumberArray *row_numberarrays, NumberArray *column_numberarrays, NumberArray *block_numberarrays)
{
	pthread_t rowThreads[Row_Threads];
	pthread_t columnThreads[Column_Threads];
	pthread_t blockThreads[Block_Threads];
	int i, max_threads;

	// The maximum threads are calculated so that unnecessary loops are eliminated.
	max_threads = max(max(Row_Threads, Column_Threads), Block_Threads);


	// Create the threads for each row, column, and block.
	for(i = 0; i < max_threads; i++)
	{
		if(i < Row_Threads)
			if(pthread_create(&rowThreads[i], NULL, isValid, (&row_numberarrays[i]) ))
				if(VERBOSE) printf("Couldn't create thread for row %d.\n", i);

		if(i < Column_Threads)
			if(pthread_create(&columnThreads[i], NULL, isValid, (&column_numberarrays[i]) ))
				if(VERBOSE) printf("Couldn't create thread for column %d.\n", i);

		if(i < Block_Threads)
			if(pthread_create(&blockThreads[i], NULL, isValid, (&block_numberarrays[i]) ))
				if(VERBOSE) printf("Couldn't create thread for column %d.\n", i);

	}

	// Start execution on the threads that have been created
	for(i = 0; i < max_threads; i++)
	{
		if(i < Row_Threads)
			if(pthread_join(rowThreads[i], NULL))
				if(VERBOSE) printf("Couldn't create thread for row %d.\n", i);

		if(i < Column_Threads)
			if(pthread_join(columnThreads[i], NULL))
				if(VERBOSE) printf("Couldn't create thread for column %d.\n", i);

		if(i < Block_Threads)
			if(pthread_join(blockThreads[i], NULL))
				if(VERBOSE) printf("Couldn't create thread for column %d.\n", i);
	}

	// Test all the structs isTested bits incrementally until they all == 1. 
	// If any fo the isValid members == 0, break the loop and set isvalid to 0.
	int isValid = 1;
	for(i = 0; i < max_threads; i++)
	{
		if(i < Row_Threads){
			// Wait until the struct is tested
			while(row_numberarrays[i].isTested == 0)
			;
			if(row_numberarrays[i].isValid == 0){
				isValid = 0;
				break;
			}

		}
		if(i < Column_Threads){
			while(column_numberarrays[i].isTested == 0)
			;
			if(column_numberarrays[i].isValid == 0){
				isValid = 0;
				break;
			}
		}
		if(i < Block_Threads){
			while(block_numberarrays[i].isTested == 0)
			;
			if(block_numberarrays[i].isValid == 0){
				isValid = 0;
				break;
			}
		}
	}

	if(VERBOSE) printf("------------%d\n", isValid);

	return isValid;
}

/*
  Function that tests if the row, column, or block has valid numbers. To verify that the thread has been tested, 
  the isTested bit is set to 1 at the end of the funtion.
*/

void *isValid(void *arg)
{
	int i, j;
	NumberArray *testNumberArray = arg;

	// Iterate through each 9-element array
	for(i = 0; i < testNumberArray->array_count; i++)
	{
		// Keep track of what elements have been seen within the 9-element array
		int numbersFound[SUDOKU_DEPTH] = {0,0,0,0,0,0,0,0,0};
		for(j = 0; j < SUDOKU_DEPTH; j++)
		{
			// If the element has been seen already, the array is not valid
			if((testNumberArray->elements[i][j])
				&& numbersFound[testNumberArray->elements[i][j] - 1] == 1
				&& testNumberArray->elements[i][j] >= 1
				&& testNumberArray->elements[i][j] <= 9)
			{
				testNumberArray->isValid = 0;
				if(VERBOSE) printf("Array not valid! The duplicate is %d\n", testNumberArray->elements[i][j]);
				break;
			}
			// If the element has not been seen, the array is valid
			else{
				if (testNumberArray->elements[i][j])
					numbersFound[testNumberArray->elements[i][j] - 1] = 1;
			}
		}
		// Break early since the structure is not valid
		if(testNumberArray->isValid == 0)
			break;
	}
	// Set isTested to 1 to let the dispatcher of the threads that this struct has been tested.
	testNumberArray->isTested = 1;
}

/*
  Function that copies the row and returns a pointer to the dynamic copy
*/

int *Copy_Row(int **given_sudoku, int row)
{
	int *temp = malloc(sizeof(int)*9);
	int i;

	for(i = 0; i < 9; i++)
	{
		temp[i] = given_sudoku[row][i];
		if(VERBOSE) printf("%d", temp[i]);
	}
	if(VERBOSE) printf("\n");
	return temp;
}

/*
  Function that copies the column and returns a pointer to the dynamic copy
*/

int *Copy_Column(int **given_sudoku, int column)
{
	int *temp = malloc(sizeof(int)*9);
	int i;

	for(i = 0; i < 9; i++)
	{
		temp[i] = given_sudoku[i][column];
		if(VERBOSE) printf("%d\n", temp[i]);
	}
	
	return temp;
}

/*
  Function that copies the block and returns a pointer to the dynamic copy
*/

int *Copy_Block(int **given_sudoku, int blocknumber)
{
	int *temp = malloc(sizeof(int)*9);
	// Correct vertical position to either 0, 3, or 6
	int vertical_pos = (blocknumber / 3) * 3;
	// Correct horizontal position to either 0, 3, or 6
	int horizontal_pos = blocknumber % 3 * 3;

	int i, j, k;

	// Iterate a 9x9 block leftwards/downwards to form the block
	i = 0;
	for(j = vertical_pos; j < vertical_pos + 3; j++)
	{
		for(k = horizontal_pos; k < horizontal_pos + 3; k++)
		{
			temp[i] = given_sudoku[j][k];
			if(VERBOSE) printf("%d", temp[i]);
			i++;
		}
		if(VERBOSE) printf("\n");
	}
	if(VERBOSE) printf("\n");
	return temp;
}

/*
  Function that initializes the struct and pointer values.
*/
void Initialize(NumberArray *row_numberarrays, NumberArray *column_numberarrays, NumberArray *block_numberarrays)
{
	int i;

	for(i = 0; i < Row_Threads; i++)
	{
		row_numberarrays[i].array_count = 0;
		row_numberarrays[i].isValid = 1;
		row_numberarrays[i].isTested = 0;
		// Allocate memory for the pointers of the pointers
		row_numberarrays[i].elements = malloc(sizeof(int *) * (SUDOKU_DEPTH / Row_Threads + 1));
	}

	for(i = 0; i < Column_Threads; i++)
	{
		column_numberarrays[i].array_count = 0;
		column_numberarrays[i].isValid = 1;
		column_numberarrays[i].isTested = 0;
		// Allocate memory for the pointers of the pointers
		column_numberarrays[i].elements = malloc(sizeof(int *) * (SUDOKU_DEPTH / Column_Threads + 1));
	}

	for(i = 0; i < Block_Threads; i++)
	{
		block_numberarrays[i].array_count = 0;
		block_numberarrays[i].isValid = 1;
		block_numberarrays[i].isTested = 0;
		// Allocate memory for the pointers of the pointers
		block_numberarrays[i].elements = malloc(sizeof(int *) * (SUDOKU_DEPTH / Block_Threads + 1));
	}
}

/*
  Function that frees up storage within the struct for better performance
*/
void Free_Memory(NumberArray *row_numberarrays, NumberArray *column_numberarrays, NumberArray *block_numberarrays)
{
	int i, j;
	for(i = 0; i < Row_Threads; i++)
	{
		for(j = 0; j < row_numberarrays[i].array_count; j++)
			free(row_numberarrays[i].elements[j]);

		free(row_numberarrays[i].elements);
	}
	for(i = 0; i < Column_Threads; i++)
	{
		for(j = 0; j < column_numberarrays[i].array_count; j++)
			free(column_numberarrays[i].elements[j]);

		free(column_numberarrays[i].elements);
	}
	for(i = 0; i < Block_Threads; i++)
	{
		for(j = 0; j < block_numberarrays[i].array_count; j++)
			free(block_numberarrays[i].elements[j]);

		free(block_numberarrays[i].elements);
	}
}

/*
  Function that creates the 9x9 sudoku grid. a pointer to this grid is returned.
*/
int **Decode_Sudoku(char *sudokustring)
{
	int i, j;

	int **given_sudoku;

	// Allocate memory for the two dimensional 9x9 array
	given_sudoku = malloc(sizeof(int*) * 9);
	for(i = 0; i < 9; i++)
		given_sudoku[i] = malloc(sizeof(int) * 9);

	for(i = 0; i < 9; i++){
		for(j = 0; j < 9; j++){
			// Correct ASCII to integer with 0x30
			given_sudoku[i][j] = sudokustring[i * 9 + j] - 0x30;
		}
	}
	return given_sudoku;
}

/*
  Function that prints the sudoku puzzle in the usually format. This is used so that the user can confirm the inputs
*/

void Print_Sudoku(int ** given_sudoku)
{
	int i, j;

	for(i = 0; i < 9; i++){
		// Formatting to make it look nice
		if(i == 3 || i == 6)
			printf("---------------------\n");
		for(j = 0; j < 9; j++){
			printf("%c ", (!(given_sudoku[i][j]) ? '_' : (char) ((int) '0') + given_sudoku[i][j]));
			// More formatting
			if(j == 2 || j == 5)
				printf("| ");
		}
		printf("\n");
	}
	printf("\n");
}

/*
  Function that returns the max of two numbers
*/

int max(int num1, int num2)
{
	if(num1 >= num2)
		return num1;
	else
		return num2;
}
