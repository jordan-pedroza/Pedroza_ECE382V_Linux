#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    char a = 'a';
    char b = 'F';
    char c = '3';

    printf("%c\n", toupper(a));
    printf("%c\n", toupper(b));
    printf("%c\n", toupper(c));

    //other string functions
    char ham[100] = "Hey ";

    strcat(ham, "Bucky " );
    strcat(ham, "you " );
    strcat(ham, "smell " );
    printf(" %s \n", ham);

    strcpy(ham, "Bucky is awesome");
    

    return 0; 
}