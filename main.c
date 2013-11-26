#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "arrayHelpers.h"

typedef struct {
	float** inArray;
	float** outArray;
	int arrayX;
	int arrayY;
	float precision;
} LoopData;

int __VERBOSE;


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



void* threadLoop( void* inData)
{
	LoopData* theData = (LoopData*) inData;

	int i, diff = 0, count = 0;

	// Little bit of indirection to make access easier/simpler
	float** currArray	= theData->inArray;
	float** nextArray	= theData->outArray;
	int   	arrayX		= theData->arrayX;
	int   	arrayY		= theData->arrayY;
	float 	precision 	= theData->precision;

	for( i = 0; i < arrayX; i++)
	{
		memcpy(nextArray[i], theData->inArray[i], arrayY * sizeof(float));
	}

	while( diff == 0 )
	{
		count++;

		//copy the next array into the working copy.
		for( i = 1; i < arrayX - 1; i++)
		{
			memcpy(currArray[i], nextArray[i], arrayY * sizeof(float));
		}

		averageFour(currArray, nextArray, arrayX, arrayY);

		diff = checkDiff(currArray, nextArray, arrayX, arrayY, precision);

	}

	if (__VERBOSE)
	{
		//printf("Relaxation finished for thread %d", (int)pthread_self());
	}

	return 0;
}


void relaxationThreaded(float** inArray,
						float** outArray,
						int arraySize,
						float precision,
						int numThreads)
{
	if (numThreads > 0)
	{
		//== Threading setup ==

		// Calculate granularity
		int currPos = 0;
		// plus 2 because we want to overlap and edges are
		// kept constant by the functions.
		int threadChunk = (arraySize / numThreads) + 2;

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
	}
	else
	{
		//== Serial Computation ==
		LoopData data;
		data.inArray 	= inArray;
		data.outArray 	= outArray;
		data.arrayX 	= arraySize;
		data.arrayY		= arraySize;
		data.precision 	= precision;

		threadLoop((void*)&data);
	}
}

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
#ifdef PTHREAD_THREADS_MAX
	if (numThreads > PTHREAD_THREADS_MAX)
	{
		numThreads = PTHREAD_THREADS_MAX;
	}
#endif
	clock_t start_t, end_t, durr_t;

	start_t = clock();

	printf("Starting to relax %d square array to precision %f.",
			arraySize, precision);

	printf("Using %d thread%s (other than main). starttick: %ld",
		numThreads, numThreads == 1 ? "" : "s", start_t);

	printf(". Seed: %d.\n", arrSeed);

	// Initializing and mallocing the arrays
	float** initialArray	= make2DFloatArray(arraySize, arraySize);
	float** finalArray 		= make2DFloatArray(arraySize, arraySize);
	initArray(initialArray, arraySize, arrSeed);

	relaxationThreaded(initialArray, finalArray, arraySize, precision, numThreads);

	free2DFloatArray(initialArray, arraySize);
	free2DFloatArray(finalArray, arraySize);

	end_t = clock();
	durr_t = end_t - start_t;
	float durr_s = (durr_t * 1.0f) / (CLOCKS_PER_SEC * 1.0f);

	printf("Relaxed %d square matrix in %ld ticks (%f sec, endticks: %ld)\n",
		 arraySize, durr_t, durr_s, end_t);

	// Windows command to stop console applications closing immediately.
	system("pause");

	return 0;
}
