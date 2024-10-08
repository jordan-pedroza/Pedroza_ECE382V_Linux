#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    float bacon1 = 9.365;
    float bacon2 = 1.8;

    printf("bacon1 is %.2f\n ", floor(bacon1));
    printf("bacon2 is %.2f\n ", floor(bacon2));

    printf("bacon1 is %.2f\n ", ceil(bacon1));
    printf("bacon2 is %.2f\n ", ceil(bacon2));
    

    return 0; 
}