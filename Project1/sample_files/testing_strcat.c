#include <string.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>


int main(void){
    char *string1;
    char *string2;

    string1 = readline("String 1 here: ");
    string2 = readline("String 2 here: ");

    strcat(string1, string2);

    printf("%s \n", string1);

    return 0;

}