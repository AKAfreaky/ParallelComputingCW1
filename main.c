#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "arrayHelpers.h"

// Struct for passing data to threads
typedef struct {
	/* Data */
	float** inArray;
	float** outArray;
	int arrayX;
	int arrayY;
	float precision;
	/* Sync stuff */
	int numThreads;
	int* finishedThreads;
	pthread_barrier_t* barrier;
	pthread_mutex_t* finLock;
} LoopData;

// Global 'debugging' variable
int __VERBOSE;


/* Checks if the values in oldArray differ from the values in newArray by less
	than the precision. Returns 1 if the don't differ, 0 if they do.
*/
int checkDiff( 	float** oldArray,
				float** newArray,
				int arrayX,
				int arrayY,
				float precision)
{
	int i,j;

	for(i = 1; i < arrayX - 1; i++)
	{
		for(j = 1; j < arrayY - 1; j++)
		{
			float oldVal = oldArray[i][j],
				  newVal = newArray[i][j];

			// if the values differ by more than the precision
			if( abs(oldVal - newVal) > precision )
			{
				if (__VERBOSE)
				{
					printf("checkDiff failed on pos(%d, %d).\n", i, j);
				}

				return 0;	// the arrays are too different
			}
		}
	}

	// Assume we passed
	return 1;
}


/* Averages the values surrounding cardinal values in inArray and sets the
	average to outArray. Ignores edges.
*/
void averageFour( float** inArray, float** outArray, int arrayX, int arrayY)
{
	int i, j;
	//from pos 1 to arraySize-2 as edges are fixed
	for(i = 1; i < arrayX - 1; i++)
	{
		for(j = 1; j < arrayY - 1; j++)
		{
			float n,s,e,w;
			n	=	inArray[i-1][j];
			s	=	inArray[i+1][j];
			e	=	inArray[i][j+1];
			w	=	inArray[i][j-1];

			outArray[i][j] = (n + s + e + w) / 4.0f;

		}
	}

}


/* The processing loop. Kinda superstep style, everyone copies,
	everyone averages and then everyone checks if they're done.
	Will quit when every thread says it has finished.
*/
void* threadLoop( void* inData)
{
	LoopData* theData = (LoopData*) inData;

	int i, diff = 0;

	// Little bit of indirection to make access easier/simpler
	float** currArray		= theData->inArray;
	float** nextArray		= theData->outArray;
	int   	arrayX			= theData->arrayX;
	int   	arrayY			= theData->arrayY;
	float 	precision 		= theData->precision;
	int* 	finishedThreads	= theData->finishedThreads;

	for( i = 0; i < arrayX; i++)
	{
		memcpy(nextArray[i], theData->inArray[i], arrayY * sizeof(float));
	}

	while( 1 )
	{
		//copy the next array into the working copy.
		for( i = 1; i < arrayX - 1; i++)
		{
			memcpy(currArray[i], nextArray[i], arrayY * sizeof(float));
		}

		//Wait until everyones done that
		if(theData->barrier != NULL)
		{
			pthread_barrier_wait(theData->barrier);
		}

		averageFour(currArray, nextArray, arrayX, arrayY);

		//Wait until everyones done that
		if(theData->barrier != NULL)
		{
			pthread_barrier_wait(theData->barrier);
		}


		diff = checkDiff(currArray, nextArray, arrayX, arrayY, precision);

		if (diff != 0)
		{

			if(theData->finLock != NULL)
			{
				pthread_mutex_lock(theData->finLock);
			}

			(*finishedThreads)++;

			if(theData->finLock != NULL)
			{
				pthread_mutex_unlock(theData->finLock);
			}
		}

		// Wait for everyone again
		if(theData->barrier != NULL)
		{
			pthread_barrier_wait(theData->barrier);
		}

		// If everyone is done we can go
		if ((*finishedThreads) == theData->numThreads)
		{
			break;
		}
		else //otherwise we have to try again
		{
			(*finishedThreads) = 0;
			// Didn't lock as we're setting to a constant and the only other
			// modification of the variable is behind a barrier in the loop.
		}

	}

	// Getting an id for a pthread depends on the platform, so this is disabled for now
	//if (__VERBOSE)
	//{
	//	printf("Relaxation finished for thread %d.\n");
	//}

	return 0;
}


/* Sets up and runs the threads
*/
void relaxationThreaded(float** inArray,
						float** outArray,
						int arraySize,
						float precision,
						int numThreads)
{
	if (numThreads > 0)
	{
		// Calculate granularity
		int currPos = 0;
		// plus 2 because we want to overlap and edges are
		// kept constant by the functions.
		int threadChunk = (arraySize / numThreads) + 2;

		// To store the data for the thread function
		LoopData	loopDataArray[numThreads];
		pthread_t 	threads[numThreads];

		// All threads must reach the barrier before we continue
		pthread_barrier_t theBarrier;
		pthread_barrier_init(&theBarrier, NULL, numThreads);

		// Shared counter so processes know when they're finished
		int finishedThreads = 0;

		// Lock for the counter
		pthread_mutex_t theLock;
		pthread_mutex_init(&theLock, NULL);

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
			loopDataArray[i].barrier		= &theBarrier;
			loopDataArray[i].numThreads		= numThreads;
			loopDataArray[i].finishedThreads= &finishedThreads;
			loopDataArray[i].finLock		= &theLock;

			if (__VERBOSE)
			{
				printf("Starting thread %d.\n", i);
			}

			pthread_create(&threads[i], NULL,
							threadLoop, (void*)&loopDataArray[i]);

			currPos = threadChunk - 2;

		}

		// join will block if the thread is going, otherwise doesn't block.
		for( i = 0; i < numThreads; i++)
		{
			pthread_join(threads[i], NULL);
		}

		// Cleanup
		pthread_barrier_destroy(&theBarrier);
		pthread_mutex_destroy(&theLock);
	}
	else
	{
		//== Serial Computation ==
		LoopData data;
		int finishedThreads = 0;

		data.inArray 	= inArray;
		data.outArray 	= outArray;
		data.arrayX 	= arraySize;
		data.arrayY		= arraySize;
		data.precision 	= precision;
		data.barrier 	= NULL;
		data.finLock	= NULL;
		data.numThreads = 1;
		data.finishedThreads = &finishedThreads;

		threadLoop((void*)&data);
	}
}


/* Hopefully useful information on how to run the program.
*/
void printUsage()
{
	printf("Arguments are:\n"
			"\t-s\t:\tInteger\t-\tThe size of the matrix\n"
			"\t-p\t:\tFloat\t-\tThe precision to work to\n"
			"\t-t\t:\tInteger\t-\tThe number of threads to use\n"
			"\t-r\t:\tInteger\t-\tSeed to use when filling the array. "
			"Zero will use current time() as the seed\n"
			"\t-v\t:\tNone\t-\tFlag to enable more console spew\n");
	// Windows command to stop console applications closing immediately.
	system("pause");
	exit(0);
}


int main(int argc, char **argv)
{
	// Initial values (should get from cmd line)
	int arraySize	= 10;
	float precision	= 10;
	int numThreads 	= 2;
	__VERBOSE 		= 0;
	int arrSeed		= time(0);

	// Read options
	// -s is the size, -p is the precision and -t is number of threads.
	// -v turns on some debug spew
	int c;
	opterr = 0;
	while ((c = getopt (argc, argv, "s:p:t:vr:")) != -1)
	{
		switch (c)
		{
			case 'p':
            	if (sscanf(optarg, "%f", &precision) != 1)
				{
					fprintf (stderr,
					"Option -%c requires a float argument.\n",
					 optopt);
					printUsage();
				}
            	break;
			case 'r':
				if (sscanf(optarg, "%i", &arrSeed) != 1)
				{
					fprintf (stderr,
					"Option -%c requires an integer argument.\n",
					 optopt);
					printUsage();
				}
				break;
			case 's':
				if (sscanf(optarg, "%i", &arraySize) != 1)
				{
					fprintf (stderr,
					"Option -%c requires an integer argument.\n",
					 optopt);
					printUsage();
				}
            	break;
           	case 't':
             	if (sscanf(optarg, "%i", &numThreads) != 1)
				{
					fprintf (stderr,
					"Option -%c requires an integer argument.\n",
					 optopt);
					printUsage();
				}
            	break;
			case 'v':
				__VERBOSE = 1;
				break;
          	default:
          		printUsage();
           }
	}

#ifdef PTHREAD_THREADS_MAX // Linux don't got this
	if (numThreads > PTHREAD_THREADS_MAX)
	{
		numThreads = PTHREAD_THREADS_MAX;
	}
#endif

	// Clock() seems slightly more accurate than time()
	// at least on aquila it looks like milliseconds.
	clock_t start_t, end_t, durr_t;
	start_t = clock();

	printf("Starting to relax %d square array to precision %f.",
			arraySize, precision);

	printf("Using %d thread%s (other than main). starttick: %ld",
		numThreads, numThreads == 1 ? "" : "s", start_t);

	printf(". Seed: %d.\n", arrSeed);

	// Initializing and mallocing the arrays
	float** currArray = make2DFloatArray(arraySize, arraySize);
	float** nextArray = make2DFloatArray(arraySize, arraySize);
	initArray(currArray, arraySize, arrSeed);

	relaxationThreaded(currArray, nextArray, arraySize, precision, numThreads);

	// Stop timer here.
	end_t = clock();

	// Create 2 more arrays, we'll do it again serially
	float** currArray2 = make2DFloatArray(arraySize, arraySize);
	float** nextArray2 = make2DFloatArray(arraySize, arraySize);
	initArray(currArray2, arraySize, arrSeed);

	if(__VERBOSE)
	{
		printf("Original:\n");
		printSquareArray(currArray, arraySize);
	}

	relaxationThreaded(currArray2, nextArray2, arraySize, precision, 0);

	int correct = checkDiff(nextArray, nextArray2, arraySize, arraySize, precision);

	if(__VERBOSE)
	{
		printf("\nThreaded:\n");
		printSquareArray(nextArray, arraySize);

		printf("\nSerial:\n");
		printSquareArray(nextArray2, arraySize);
	}

	// Cleanup
	free2DFloatArray(currArray, arraySize);
	free2DFloatArray(nextArray, arraySize);

	free2DFloatArray(currArray2, arraySize);
	free2DFloatArray(nextArray2, arraySize);

	// *1.0f so we do float division rather than int division (which floors)
	durr_t = end_t - start_t;
	float durr_s = (durr_t * 1.0f) / (CLOCKS_PER_SEC * 1.0f);

	printf("Relaxed %d square matrix in %ld ticks (%f sec, endticks: %ld)\n",
		 arraySize, durr_t, durr_s, end_t);

	printf("Threaded relaxation result %s the serial result.\n", correct ? "matched" : "didnt match");

	// Windows command to stop console applications closing immediately.
	system("pause");

	return 0;
}
