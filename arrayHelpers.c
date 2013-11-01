#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// fills a 2D-square array with random values
void initArray( int** theArray, int arraySize )
{
	srand( time( 0 ) );
	int i, j;
	for( i = 0; i < arraySize; i++ )
	{
		for( j = 0; j < arraySize; j++ )
		{
			int value =  rand();
			theArray[i][j] = value;
		}
	}

	printf("Array filled with pseudo-random values\n");

}

void printSquareArray( int** theArray, int arraySize )
{
	int i, j;
	for( i = 0; i < arraySize; i++ )
	{
		for( j = 0; j < arraySize; j++ )
		{
			printf("%d\t", theArray[i][j]);
		}
		printf("\n");
	}
}


int** make2DIntArray(int arraySizeX, int arraySizeY)
{
	int **theArray, i;
	theArray = (int**) malloc(arraySizeX*sizeof(int*));
	for (i = 0; i < arraySizeX; i++)
	{
		theArray[i] = (int*) malloc(arraySizeY*sizeof(int));
	}
	return theArray;
}

void free2DIntArray(int** theArray, int arraySizeX)
{
	int i;
	for (i = 0; i < arraySizeX; i++)
	{
   		free(theArray[i]);
	}
	free(theArray);
}
