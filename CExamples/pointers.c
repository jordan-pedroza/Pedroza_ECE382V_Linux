#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{
    int tuna = 19;
    int * pTuna = &tuna;

    printf("Address\tName\tValue\n");
    printf("%p\t%s\t%d\n", pTuna, "tuna", tuna);
    printf("%p\t%s\t%p\n", &pTuna, "pTuna", pTuna);

    printf("\n*pTuna: %d\n", *pTuna);

    //*pTuna = 19
    //tuna = 19
    // *pTuna = tuna

    return 0;
}