#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// fills a 2D-square array with random values
void initArray( float** theArray, int arraySize )
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
}

void printSquareArray( float** theArray, int arraySize )
{
	int i, j;
	for( i = 0; i < arraySize; i++ )
	{
		for( j = 0; j < arraySize; j++ )
		{
			printf("%f\t", theArray[i][j]);
		}
		printf("\n");
	}
}


float** make2DFloatArray(int arraySizeX, int arraySizeY)
{
	int **theArray, i;
	theArray = (float**) malloc(arraySizeX*sizeof(float*));
	for (i = 0; i < arraySizeX; i++)
	{
		theArray[i] = (float*) malloc(arraySizeY*sizeof(float));
	}
	return theArray;
}

void free2DFloatArray(float** theArray, int arraySizeX)
{
	int i;
	for (i = 0; i < arraySizeX; i++)
	{
   		free(theArray[i]);
	}
	free(theArray);
}
