#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    int y1;
    int y2;
    int age;

    printf("Enter a year\n");
    scanf(" %d", &y1);

    printf("Enter a year\n");
    scanf(" %d", &y2);

    age = y1 - y2;
    printf("%d \n", age);
    age = abs(age);
    printf("%d \n", age);

    //pow()this is exponential used pow(5,3) equals 5^3

    return 0; 
}