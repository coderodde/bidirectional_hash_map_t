//
//  main.c
//  bidimap
//
//  Created by Rodion Efremov on 12/06/2017.
//  Copyright © 2017 coderodde.net. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

int cmpf(const void* a, const void* b)
{
    return (*(int*)b - *(int*)a);
}

void get_change(float money, int* coins, size_t coins_len)
{
    qsort(coins, coins_len, sizeof(int), cmpf);
    printf("%d", coins[0]);
}

int main()
{
    int coins[] = {1, 17};
    get_change(1.00, coins, 2);
}
