#include <stdio.h>
#include <stdlib.h>

int main()
{
    char first_name[20];
    char crush[20];
    int numBabies;

    printf("What is your name? \n");
    scanf("%s", first_name);//

    printf("Who are you going to marry? \n");
    scanf("%s", crush);//

    printf("how many kids will you have? \n");
    scanf("%d", &numBabies);// we are sending this to the address of the int

    printf("%s and %s are in love and will have %d babies.\n", first_name, crush, numBabies);

    return 0;
}