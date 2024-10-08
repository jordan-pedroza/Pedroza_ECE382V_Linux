#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    char catsName[50];
    char catsFood[50];
    char sentence[100] = "";

    puts("What is the cats name?"),
    gets(catsName);

    puts("What is the cats food?"),
    gets(catsFood);    

    strcat(sentence, catsName);
    strcat(sentence, " loves to eat ");
    strcat(sentence, catsFood);

    printf(sentence);
    

    return 0; 
}