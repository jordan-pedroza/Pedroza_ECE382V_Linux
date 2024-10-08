#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int main()
{
    int i;
    int player[5] = {58, 66, 68, 71, 87};
    int goals[5] = {26, 39, 25, 29, 31};
    int games_played[5] = {30, 30, 28, 30, 26};
    float ppg[5];
    float best_ppg = 0.0;
    int best_player;

    for (i = 0; i<5; i++){
        ppg[i] = (float)goals[i] / (float)games_played[i];
        printf("%d \t %d \t %d \t %.2f\n", player[i], goals[i], games_played[i], ppg[i]);

        if (ppg[i] > best_ppg){
            best_ppg = ppg[i];
            best_player = player[i];
        }
    }
    printf("The best player is %d \n", best_player);




    return 0; 
}