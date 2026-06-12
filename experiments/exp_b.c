/*
 * exp_b.c — Experiment B: Column-major matrix traversal (CACHE-UNFRIENDLY)
 *
 * What we do:
 *   SAME matrix as Experiment A. SAME work (sum all numbers).
 *   But now we go through it COLUMN BY COLUMN:
 *     matrix[0][0], matrix[1][0], matrix[2][0], ...  (all of column 0)
 *     matrix[0][1], matrix[1][1], matrix[2][1], ...  (all of column 1)
 *
 * Why this is SLOW:
 *   In memory, rows are stored in order. Column 0 of each row
 *   is spread far apart in memory (2048 integers = 8 KB apart).
 *   When the CPU fetches matrix[0][0] into cache, it also grabs
 *   the nearby bytes — but the NEXT access is matrix[1][0]
 *   which is 8 KB away! That cache line is completely useless.
 *   Every single access is a cache MISS → slow trip to RAM.
 *
 * This is the EXACT same code as exp_a.c — only i and j are swapped.
 * Same result. Very different speed. That gap is what we measure.
 *
 * Matrix size: 2048 x 2048 ints = 16 MB
 */

#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

#define ROWS 2048
#define COLS 2048
#define RUNS 8

int main(void) {

    /* --- Step 1: Same matrix allocation as exp_a --- */
    int (*matrix)[COLS] = malloc(ROWS * sizeof(*matrix));
    if (!matrix) {
        fprintf(stderr, "ERROR: not enough memory\n");
        return 1;
    }

    /* --- Step 2: Same data as exp_a (important: identical input) --- */
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = (i * 3 + j * 7) % 256;

    /* --- Step 3: CSV header --- */
    printf("experiment,run,time_ms\n");

    /* --- Step 4: Run the experiment RUNS times --- */
    for (int r = 0; r < RUNS; r++) {

        long long sum = 0;
        double start = get_time_ms();

        /*
         * THE EXPERIMENT: column-major traversal
         * Outer loop = columns (j), inner loop = rows (i)
         * This jumps 2048*4 = 8192 bytes between reads — cache hates this
         *
         * ONLY THESE TWO LINES ARE DIFFERENT FROM exp_a.c:
         *   exp_a: for (i)  for (j)  sum += matrix[i][j]
         *   exp_b: for (j)  for (i)  sum += matrix[i][j]
         */
        for (int j = 0; j < COLS; j++)
            for (int i = 0; i < ROWS; i++)
                sum += matrix[i][j];

        double elapsed = get_time_ms() - start;

        printf("B_col_major,%d,%.3f\n", r + 1, elapsed);

        if (sum < 0) printf("sum=%lld\n", sum);
    }

    free(matrix);
    return 0;
}
