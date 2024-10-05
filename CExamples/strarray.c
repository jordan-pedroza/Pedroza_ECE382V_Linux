#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int main()
{

    char name[20] = "Jordan Pedroza\n";
    printf(" My name is %s", name);
    name[2] = 'R';
    printf(" My name is %s", name);

    char food[50] = "tuna";
    printf("The best food is %s\n", food);

    strcpy(food, "something else");
    printf("The best food is %s\n", food);

    return 0;
}