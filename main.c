#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "arrayHelpers.h"

int checkDiff( int** oldArray, int** newArray, int arrayX, int arrayY, int precision)
{
	int i,j, retVal = 1; // assume we will pass

	for(i = 1; i < arrayX - 1; i++)
	{
		for(j = 1; j < arrayY - 1; j++)
		{
			int oldVal = oldArray[i][j],
				newVal = newArray[i][j];
			// if the values differ by more than the precision
			if( abs(oldVal - newVal) > precision )
			{
				retVal = 0;	// the arrays are too different
			}
		}
	}

	return retVal;
}


void averageFour( int** inArray, int** outArray, int arrayX, int arrayY)
{
	int i, j;
	//from pos 1 to arraySize-2 as edges are fixed
	for(i = 1; i < arrayX - 1; i++)
	{
		for(j = 1; j < arrayY - 1; j++)
		{
			int n,s,e,w;
			n	=	inArray[i-1][j];
			s	=	inArray[i+1][j];
			e	=	inArray[i][j+1];
			w	=	inArray[i][j-1];

			outArray[i][j] = (n + s + e + w) / 4;

		}
	}

}

void threadAverage( int** inArray, int** outArray, int arrayX, int arrayY, int precision)
{
	int i, diff = 0, count = 0;

	int** currArray = make2DIntArray(arrayX, arrayY);
	int** nextArray = make2DIntArray(arrayX, arrayY);

	for( i = 1; i < arrayX - 1; i++)
	{
		memcpy(nextArray[i], inArray[i], arrayX * sizeof(int));
	}

	while( diff == 0 )
	{
		count++;

		//copy the next array into the working copy.
		for( i = 1; i < arrayX - 1; i++)
		{
			memcpy(currArray[i], nextArray[i], arrayX * sizeof(int));
		}

		averageFour(currArray, nextArray, arrayX, arrayY);

		diff = checkDiff(currArray, nextArray, arrayX, arrayY, precision);
		if ((count % 1000) == 0)
		{
			printf("Looped %d times\n", count);
		}
	}

	for( i = 1; i < arrayX - 1; i++)
	{
		memcpy(outArray[i], nextArray[i], arrayX * sizeof(int));
	}

}






int main()
{
	int arraySize = 10;
	int precision = 10;
	int numThreads = 1;

	printf("Creating 2 square arrays of arraySize %d\n", arraySize);

	int** currArray = make2DIntArray(arraySize, arraySize);
	int** nextArray = make2DIntArray(arraySize, arraySize);

	initArray(currArray, arraySize);
	initArray(nextArray, arraySize);

	printf("Initial array:\n");
	printSquareArray(currArray, arraySize);

	threadAverage(currArray, nextArray, arraySize, arraySize, precision);

	printf("Final output:\n");
	printSquareArray(nextArray, arraySize);

	free2DIntArray(currArray, arraySize);
	free2DIntArray(nextArray, arraySize);
	system("pause");
	return 0;
}
