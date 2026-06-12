/*
 * exp_a.c — Experiment A: Row-major matrix traversal (CACHE-FRIENDLY)
 *
 * What we do:
 *   We have a big grid of numbers (a 2D matrix).
 *   In this experiment we go through it ROW BY ROW:
 *     matrix[0][0], matrix[0][1], matrix[0][2], ...
 *     matrix[1][0], matrix[1][1], matrix[1][2], ...
 *
 * Why this is fast:
 *   In memory, a row is stored as one long line of numbers.
 *   When the CPU reads matrix[0][0], it also grabs the next
 *   64 bytes into cache — which includes [0][1], [0][2], etc.
 *   So the next 15 or so reads are FREE (already in cache).
 *   This gives us a very HIGH cache hit rate → fast!
 *
 * Matrix size: 2048 x 2048 ints = 16 MB (larger than L2 cache)
 */

#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

#define ROWS 2048
#define COLS 2048
#define RUNS 8        /* run the loop 8 times for stable results */

int main(void) {

    /* --- Step 1: Allocate a 2D matrix on the heap --- */
    /* We use a pointer-to-array so rows are stored in one block */
    int (*matrix)[COLS] = malloc(ROWS * sizeof(*matrix));
    if (!matrix) {
        fprintf(stderr, "ERROR: not enough memory\n");
        return 1;
    }

    /* --- Step 2: Fill the matrix with some data --- */
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = (i * 3 + j * 7) % 256;

    /* --- Step 3: Print CSV header --- */
    printf("experiment,run,time_ms\n");

    /* --- Step 4: Run the experiment RUNS times --- */
    for (int r = 0; r < RUNS; r++) {

        long long sum = 0;
        double start = get_time_ms();

        /*
         * THE EXPERIMENT: row-major traversal
         * Outer loop = rows (i), inner loop = columns (j)
         * This reads memory left-to-right, in order — cache loves this
         */
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                sum += matrix[i][j];

        double elapsed = get_time_ms() - start;

        /* Print one CSV row per run */
        printf("A_row_major,%d,%.3f\n", r + 1, elapsed);

        /*
         * This line stops the compiler from removing our loop.
         * A smart compiler sees "sum is never used" and deletes
         * the whole loop to make it faster. We trick it here.
         */
        if (sum < 0) printf("sum=%lld\n", sum);
    }

    free(matrix);
    return 0;
}
