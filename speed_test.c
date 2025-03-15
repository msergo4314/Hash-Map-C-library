#include <stdio.h>
#include "hashmap.h"
#include <time.h>

#define NUMBER_OF_KEYS (int)(1e7)
#define RANGE 1000

int main(void) {
    srand(time(0));

    clock_t start = clock();
    printf("Attempting to batch insert %.2e keys into hashmap\n", (double)NUMBER_OF_KEYS);

    int *keys = malloc(sizeof(int) * NUMBER_OF_KEYS);
    int *values = malloc(sizeof(int) * NUMBER_OF_KEYS);

    for (int i = 0; i < NUMBER_OF_KEYS; i++) {
        keys[i] = i;
        values[i] = (rand() % (RANGE - 1) + 1);
    }
    HashMap *map = hash_table_create(NUMBER_OF_KEYS, INTEGER_TYPE);
    clock_t begin = clock();
    
    if (!hash_table_batch_insert(map, keys, values, NUMBER_OF_KEYS, INTEGER_TYPE, INTEGER_TYPE)) {
        printf("error with insertion\n");
        goto exit;
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("operation took %.4lf seconds\n", time_spent);

    hash_table_info_print(map);
    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("total time to setup, insert, and clean up is: %.4lf seconds\n", total_time);
    exit:
    hash_table_destroy(&map);
    free(keys);
    free(values);
    return 0;
}