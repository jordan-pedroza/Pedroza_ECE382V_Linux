#include <stdio.h>
#include <stdlib.h>

int main()
{
    float age1, age2, age3, average;

    age1 = age2 = 4.0;

    printf("Enter your age\n");
    scanf("%f", &age3);

    average = (age1 + age2 + age3) / 3;

    printf("\n The average age of the group is: %f", average);


    /*
    int a;
    int b;
    int c;

    a = b = c = 100;//c equals 100 then b equals c then a equals b

    printf("%d %d %d", a, b, c);*/
    
    return 0;
}