/*
 * exp_c.c — Experiment C: Sequential array access (CACHE-FRIENDLY)
 *
 * What we do:
 *   We have a large 1D array of numbers.
 *   We read it from index 0 to the end, in perfect order:
 *     arr[0], arr[1], arr[2], arr[3], ...
 *
 * Why this is fast:
 *   This is the most cache-friendly access pattern possible.
 *   The CPU reads ahead ("hardware prefetcher") and predicts
 *   that you will read the next item. It loads it into cache
 *   BEFORE you even ask for it. Almost every access is a hit.
 *
 *   This is also how NumPy works internally — it stores arrays
 *   in one continuous block and reads them in order. That's why
 *   NumPy is so fast compared to Python lists!
 *
 * Array size: 16 million integers = 64 MB
 */

#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

#define SIZE  (16 * 1024 * 1024)   /* 16 million integers */
#define RUNS  8

int main(void) {

    /* --- Step 1: Allocate a large 1D array --- */
    int *arr = malloc(SIZE * sizeof(int));
    if (!arr) {
        fprintf(stderr, "ERROR: not enough memory\n");
        return 1;
    }

    /* --- Step 2: Fill with data --- */
    for (int i = 0; i < SIZE; i++)
        arr[i] = (i * 13 + 7) % 256;

    /* --- Step 3: CSV header --- */
    printf("experiment,run,time_ms\n");

    /* --- Step 4: Run the experiment --- */
    for (int r = 0; r < RUNS; r++) {

        long long sum = 0;
        double start = get_time_ms();

        /*
         * THE EXPERIMENT: sequential access
         * Go from arr[0] to arr[SIZE-1] in order.
         * The CPU's prefetcher loads the next cache line
         * while we are still working on the current one.
         * Result: almost zero waiting time.
         */
        for (int i = 0; i < SIZE; i++)
            sum += arr[i];

        double elapsed = get_time_ms() - start;

        printf("C_sequential,%d,%.3f\n", r + 1, elapsed);

        if (sum < 0) printf("sum=%lld\n", sum);
    }

    free(arr);
    return 0;
}
