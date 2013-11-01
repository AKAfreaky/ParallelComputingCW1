#ifndef ARRAYHELPERS_H_INCLUDED
#define ARRAYHELPERS_H_INCLUDED

// fills a 2D-square array with random values
void initArray( int** theArray, int arraySize );

void printSquareArray( int** theArray, int arraySize );

int** make2DIntArray(int arraySizeX, int arraySizeY);

void free2DIntArray(int** theArray, int arraySizeX);

#endif // ARRAYHELPERS_H_INCLUDED
