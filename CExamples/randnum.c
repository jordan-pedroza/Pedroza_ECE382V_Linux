#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    int i; 
    int dice_roll;

    for(i=0; i<20; i++){
        dice_roll = ((rand() % 6) + 1);
        printf("%d \n", dice_roll);
    }

    return 0; 
}