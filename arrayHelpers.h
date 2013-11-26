#ifndef ARRAYHELPERS_H_INCLUDED
#define ARRAYHELPERS_H_INCLUDED

// fills a 2D-square array with random values
void initArray( float** theArray, int arraySize, int seed );

void printSquareArray( float** theArray, int arraySize );

float** make2DFloatArray(int arraySizeX, int arraySizeY);

void free2DFloatArray(float** theArray, int arraySizeX);

#endif // ARRAYHELPERS_H_INCLUDED
