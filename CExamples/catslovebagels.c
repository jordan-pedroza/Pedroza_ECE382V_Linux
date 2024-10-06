#include <stdio.h>
#include <stdlib.h>

int main()
{

    int a = 4 + 2 * 6; //PEMDAS
    printf("Result is: %d\n", a);

    a = (4 + 2) * 6; //PEMDAS
    printf("Result is: %d\n", a);
    
    return 0;
}