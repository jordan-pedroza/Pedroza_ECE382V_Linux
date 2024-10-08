#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{
    char movie1[] = "The return of Jordans C program";

    //movie[] = "hey now"; //cant change becasue you'll leak memory

    char * movie2 = "Bucky is awesome I love ham";

    puts(movie2);

    movie2 = "new movie title";
    puts(movie2);




    /*int i;
    int meatballs[5] = {7, 5, 9, 34, 78};

    printf("Element \t Address \t Value \n");

    for (i = 0; i < 5; i++){
        printf("meatballs[%d] \t %p \t %d \n", i, &meatballs[i], meatballs[i]);
    }
    // array names are just pointers to the first element
    printf("\nmeatballs \t\t %p \n", meatballs);// same as the beginning of the first element
    //dereference it 
    printf("\n*meatballs \t\t %d \n", *meatballs);

    printf("\n*(meatballs+2) \t\t %d \n", *meatballs+2);*/

    return 0;
}