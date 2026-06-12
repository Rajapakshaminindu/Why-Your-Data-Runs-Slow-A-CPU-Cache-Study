/*
 * exp_d.c — Experiment D: Random array access (CACHE-UNFRIENDLY)
 *
 * What we do:
 *   SAME array as Experiment C. SAME number of reads.
 *   But this time we access elements in a RANDOM, shuffled order:
 *     arr[indices[0]], arr[indices[1]], arr[indices[2]], ...
 *   where 'indices' is a scrambled list of 0..SIZE-1
 *
 * Why this is SLOW:
 *   The CPU's prefetcher is completely blind. It cannot predict
 *   what random index comes next. Every single access is a
 *   cache MISS — the CPU must wait for RAM every time.
 *
 *   This is similar to what happens with pandas when you:
 *   - Use df.loc[] with non-sorted indices
 *   - Access a DataFrame column that isn't in cache
 *   - Do operations on un-sorted, scattered data
 *
 *   Data scientists call this "memory-bound" code — the code
 *   is slow not because the math is hard, but because memory
 *   access is the bottleneck.
 *
 * Array size: 16 million integers = 64 MB
 */

#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

#define SIZE  (16 * 1024 * 1024)   /* 16 million integers — same as exp_c */
#define RUNS  8

/*
 * Fisher-Yates shuffle: scrambles an array in-place.
 * After this, indices[0..SIZE-1] is a random permutation
 * of 0..SIZE-1. Every index appears exactly once.
 */
static void shuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main(void) {

    /* --- Step 1: Allocate the data array (same as exp_c) --- */
    int *arr = malloc(SIZE * sizeof(int));
    if (!arr) {
        fprintf(stderr, "ERROR: not enough memory for arr\n");
        return 1;
    }

    /* --- Step 2: Allocate a second array for the random indices --- */
    int *indices = malloc(SIZE * sizeof(int));
    if (!indices) {
        fprintf(stderr, "ERROR: not enough memory for indices\n");
        free(arr);
        return 1;
    }

    /* --- Step 3: Fill data and create the scrambled index list --- */
    for (int i = 0; i < SIZE; i++) {
        arr[i]     = (i * 13 + 7) % 256;   /* same data as exp_c */
        indices[i] = i;                      /* start as 0,1,2,3,... */
    }
    srand(42);          /* fixed seed = same shuffle every run (reproducible) */
    shuffle(indices, SIZE);

    /* --- Step 4: CSV header --- */
    printf("experiment,run,time_ms\n");

    /* --- Step 5: Run the experiment --- */
    for (int r = 0; r < RUNS; r++) {

        long long sum = 0;
        double start = get_time_ms();

        /*
         * THE EXPERIMENT: random access via shuffled indices
         * indices[i] is a random number in [0, SIZE)
         * So arr[indices[i]] jumps to a random location in memory.
         * The CPU cannot predict where next. Cache miss every time.
         */
        for (int i = 0; i < SIZE; i++)
            sum += arr[indices[i]];

        double elapsed = get_time_ms() - start;

        printf("D_random,%d,%.3f\n", r + 1, elapsed);

        if (sum < 0) printf("sum=%lld\n", sum);
    }

    free(arr);
    free(indices);
    return 0;
}
