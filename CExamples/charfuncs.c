#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    int tuna = 'f';//

    if (isalpha(tuna)){

        if (isupper(tuna)){
            printf(" %c is an upper case letter", tuna);
        }else{
            printf(" %c is an lowercase case letter", tuna);

        }
    }

    else if (isdigit(tuna)){
        printf(" %c is a number", tuna);
    }
    else{
        printf(" %c what is that?", tuna);
    }

    return 0; 
}