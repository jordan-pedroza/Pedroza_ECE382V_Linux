#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    int a;
    int how_many;
    int max_amount = 10;

    printf(" How many times do you wnat this to loop? (up to 10) ");
    scanf(" %d", &how_many);

    for(a=1; a<=max_amount; a++){
        printf(" %d\n", a);
        if (a == how_many){
            break;
        }
    }
    return 0; 
}