#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "arrayHelpers.h"

int checkDiff( int** oldArray, int** newArray, int arraySize, int precision)
{
	int i,j, retVal = 1; // assume we will pass

	for(i = 1; i < arraySize - 1; i++)
	{
		for(j = 1; j < arraySize - 1; j++)
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





int main()
{
	int arraySize = 10;
	int precision = 1;

	printf("Creating 2 square arrays of arraySize %d\n", arraySize);

	int** currArray = make2DIntArray(arraySize, arraySize);
	int** nextArray = make2DIntArray(arraySize, arraySize);

	initArray(currArray, arraySize);
	initArray(nextArray, arraySize);

	printf("Initial array:\n");
	printSquareArray(nextArray, arraySize);

	int i,j, diff = 0, count = 0;

	while( diff == 0 )
	{
		count++;

		//copy the next array into the working copy.
		for( i = 1; i < arraySize - 1; i++)
		{
			memcpy(currArray[i], nextArray[i], arraySize * sizeof(int));
		}

		//from pos 1 to arraySize-2 as edges are fixed
		for(i = 1; i < arraySize - 1; i++)
		{
			for(j = 1; j < arraySize - 1; j++)
			{
				int n,s,e,w;
				n	=	currArray[i-1][j];
				s	=	currArray[i+1][j];
				e	=	currArray[i][j+1];
				w	=	currArray[i][j-1];

				nextArray[i][j] = (n + s + e + w) / 4;

			}
		}

		diff = checkDiff(currArray, nextArray, arraySize, precision);
		if ((count % 1000) == 0)
		{
			printf("Looped %d times\n", count);
		}
	}

	printf("Final output after %d loops:\n", count);
	printSquareArray(nextArray, arraySize);

	free2DIntArray(currArray, arraySize);
	free2DIntArray(nextArray, arraySize);
	system("pause");
	return 0;
}
