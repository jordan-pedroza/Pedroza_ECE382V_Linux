#include <stdio.h>
#include <stdlib.h>

int main()
{

    float grade = 0;
    float scoreEntered = 0;
    float numOfTest = 0;
    float average = 0;

    printf("Press 0 to quit the program. \n\n");


    do {
        printf("Tests: %.0f\t Average: %.2f \n", numOfTest, average);
        printf("\nEnter test score: ");
        scanf(" %f", &scoreEntered);

        grade+= scoreEntered;
        numOfTest++;
        average = grade / numOfTest;

    }
    while (scoreEntered != 0);

    return 0; 
}