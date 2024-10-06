#include <stdio.h>
#include <stdlib.h>

int main()
{

    int page_views = 0;

    page_views = page_views + 1;
    printf("Page views: %d\n", page_views);
    page_views = page_views + 1;
    printf("Page views: %d\n", page_views);
    page_views = page_views + 1;
    printf("Page views: %d\n", page_views);

    float balance = 1000.00;

    balance *= 1.1;
    //balance = balance * 1.1;
    printf("Balance: %f\n", balance);
    balance *= 1.1;
    //balance = balance * 1.1;
    printf("Balance: %f\n", balance);
    balance *= 1.1;
    //balance = balance * 1.1;
    printf("Balance: %f\n", balance);
    balance *= 1.1;
    //balance = balance * 1.1;
    printf("Balance: %f\n", balance);

    return 0;
}