#include <stdio.h>
#include <stdlib.h>

int main()
{

    // (test) ? truecode : falsecode

    char last_name[20];
    printf("Enter your last name\n");
    scanf(" %s", last_name);

    (last_name[0] < 'M' ) ? printf("Blue team") : printf("Red team");

    return 0; 
}