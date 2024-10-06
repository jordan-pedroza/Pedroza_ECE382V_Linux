#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{

    char grade = 'C';

    switch(grade){
        case 'A' : printf("Sweet\n");
                    break;
        case 'B' : printf("Could have tried harder!\n");
                    break;
        case 'C' : printf("I C you didn't study!\n");
                    break;
        case 'D' : printf("You love the D\n");
                    break;        
        case 'F' : printf("embarassing!\n");
                    break;
        default : printf("This isn't a valid grade!");
    }

    return 0; 
}