#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{   
    int * points;
    points = (int*) malloc(5*sizeof(int));

    free(points);


    return 0;
}