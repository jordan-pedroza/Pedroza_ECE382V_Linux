#include <stdio.h>
#include <stdlib.h>

int main()
{

int a = 5, b = 10, answer = 0;
// add one to a here
answer = ++a * b ;
printf("Answer: %d \n", answer);

a = 5, b = 10, answer = 0;
answer = a++ * b ;
// add one to a here
printf("Answer: %d \n", answer);

    return 0; 
}