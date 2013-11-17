#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "arrayHelpers.h"

typedef struct {
	int** inArray;
	int** outArray;
	int arrayX;
	int arrayY;
	int precision;
	sem_t* flagComplete;
} LoopData;


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



void* threadLoop( void* inData)
{
	LoopData* theData = (LoopData*) inData;

	int i, diff = 0, count = 0;

	// Little bit of indirection to make access easier/simpler
	int** currArray = theData->inArray;
	int** nextArray = theData->outArray;
	int   arrayX	= theData->arrayX;
	int   arrayY	= theData->arrayY;
	int	  precision = theData->precision;

	for( i = 0; i < arrayX; i++)
	{
		memcpy(nextArray[i], theData->inArray[i], arrayY * sizeof(int));
	}

	while( diff == 0 )
	{
		count++;

		//copy the next array into the working copy.
		for( i = 1; i < arrayX - 1; i++)
		{
			memcpy(currArray[i], nextArray[i], arrayY * sizeof(int));
		}

		averageFour(currArray, nextArray, arrayX, arrayY);

		diff = checkDiff(currArray, nextArray, arrayX, arrayY, precision);

	}

	printf("Relaxed array %d x %d to precision %d in %d loops on thread %d\n", arrayX, arrayY, precision, count, (int)pthread_self().p);

	return 0;
}


void relaxationThreaded(int** inArray, int** outArray, int arraySize, int precision, int numThreads)
{
	//== Threading setup==

	// Calculate granularity
	int currPos = 0;
	int threadChunk = (arraySize / numThreads) + 2; // plus 2 because we want to overlap and edges are kept constant by the functions

	// To store the data for the thread function
	LoopData	loopDataArray[numThreads];
	pthread_t 	threads[numThreads];

	// Loop and create/start threads
	int i;
	for ( i = 0; i < numThreads; i++)
	{
		//pthread_t newThread;

		int columnsRemaining = (arraySize - currPos);

		loopDataArray[i].inArray 		= &inArray[currPos];
		loopDataArray[i].outArray		= &outArray[currPos];
		loopDataArray[i].arrayX 		= threadChunk > columnsRemaining ?
									 	  columnsRemaining : threadChunk;
		loopDataArray[i].arrayY 		= arraySize;
		loopDataArray[i].precision 		= precision;

		pthread_create(&threads[i], NULL, threadLoop, (void*)&loopDataArray[i]);

		currPos = threadChunk - 2;

	}

	for( i = 0; i < numThreads; i++)
	{
		pthread_join(threads[i], NULL);
	}
}



int main()
{
	// Initial values (should get from cmd line)
	int arraySize = 10;
	int precision = 10;
	int numThreads = 2;

	if (numThreads > PTHREAD_THREADS_MAX)
	{
		numThreads = PTHREAD_THREADS_MAX;
	}

	// Initializing and mallocing the arrays
	printf("Creating 2 square arrays of arraySize %d\n", arraySize);
	int** currArray = make2DIntArray(arraySize, arraySize);
	int** nextArray = make2DIntArray(arraySize, arraySize);
	initArray(currArray, arraySize);
	//initArray(nextArray, arraySize);
	printf("Initial array:\n");
	printSquareArray(currArray, arraySize);

	relaxationThreaded(currArray, nextArray, arraySize, precision, numThreads);

	printf("Final output:\n");
	printSquareArray(nextArray, arraySize);

	free2DIntArray(currArray, arraySize);
	free2DIntArray(nextArray, arraySize);
	system("pause");
	return 0;
}
