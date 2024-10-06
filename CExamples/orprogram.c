#include <stdio.h>
#include <stdlib.h>

int main()
{

    int friends = 87;

    printf("i have %d friend%s", friends, (friends != 1) ? "s" : "");



    /*char answer;

    printf("Do you like Bagels? (y/n) \n");
    scanf(" %c", &answer);

    if ((answer == 'n') || (answer == 'y')){
        printf("Good job you didnt mess anything up!");

    }
    else{
        printf("Keyboard much?");
    }
*/
    return 0; 
}