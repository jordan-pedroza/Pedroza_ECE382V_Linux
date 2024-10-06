#include <stdio.h>
#include <stdlib.h>

int main()
{
    int day = 1;
    float amount = 0.01;

    while (day <= 31){
        printf("Day: %d\t Amount: %.2f \n", day, amount);
        amount *= 2;
        day ++;
    }



//while(test){} 

//int tuna = 1;

//while(tuna < 5){
    //printf("tuna is now %d \n", tuna);
    //tuna++;
//}


    return 0; 
}