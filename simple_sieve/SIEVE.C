#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_LIMIT 1000L
#define DEFAULT_DURATION 5
#define BITSPERBYTE 8

#define GET_BIT(array, n) ((array[(n) / BITSPERBYTE] >> ((n) % BITSPERBYTE)) & 1)
#define SET_BIT(array, n) (array[(n) / BITSPERBYTE] |= (1 << ((n) % BITSPERBYTE)))

void print_help(progname)
char *progname;
{
    printf("Usage: %s [/l limit] [/d duration] [/h|/?]\n", progname);
    printf("Options:\n");
    printf("  /l limit     Specify the upper limit for prime calculation (default: 1000)\n");
    printf("  /d duration  Specify the target duration in seconds for the run (default: 5)\n");
    printf("  /h, /?       Print this help message and exit\n");
}

int parse_args(argc, argv, plimit, pduration, pexit_code)
int argc;
char *argv[];
long *plimit;
int *pduration;
int *pexit_code;
{
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '/') {
            switch (argv[i][1]) {
            case 'l':
            case 'L':
                if (argc > i + 1) {
                    *plimit = atol(argv[++i]);
                    continue;
                }
                break;
            case 'd':
            case 'D':
                if (argc > i + 1) {
                    *pduration = atoi(argv[++i]);
                    continue;
                }
                break;
            case 'h':
            case 'H':
            case '?':
                print_help(argv[0]);
                *pexit_code = 0;
                return 1;
            }
        }

        print_help(argv[0]);
        *pexit_code = 1;
        return 1;
    }

    return 0;
}

int main(argc, argv)
int argc;
char *argv[];
{
    long limit;
    int duration;
    int exit_code;
    long i, j;
    size_t size;
    long count;
    int iterations;
    char *sieve;
    clock_t start_time, end_time;
    double elapsed_time;
    clock_t tick_duration;

    limit = DEFAULT_LIMIT;
    duration = DEFAULT_DURATION;

    printf("Sieve of Eratosthenes by Davepl 2024 for the PDP-11 running 211BSD\n");
    printf("Modified by rbergen to compile for an 8086 and run on MS-DOS\n");
    printf("------------------------------------------------------------------\n");

    if (parse_args(argc, argv, &limit, &duration, &exit_code))
        return exit_code;

    size = (limit / 2) / BITSPERBYTE + 1;
    count = 1;  /* 2 is a prime number */
    sieve = (char *) malloc(size);

    printf("Solving primes up to %ld for %d seconds...\n", limit, duration);

    if (sieve == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    iterations = 0;
    tick_duration = duration * CLK_TCK;
    start_time = clock();

    do {
        memset(sieve, 0, size);

        for (i = 3; i * i <= limit; i += 2)
            if (!GET_BIT(sieve, i / 2))
                for (j = i * i; j <= limit; j += 2 * i)
                    SET_BIT(sieve, j / 2);

        iterations++;
        end_time = clock();
    } while ((end_time - start_time) < tick_duration);

    elapsed_time = (end_time - start_time) / CLK_TCK;

    for (i = 3; i <= limit; i += 2)
        if (!GET_BIT(sieve, i / 2))
            count++;

    printf("Number of primes found: %ld\n", count);
    printf("Ran %d iterations in %.3f seconds\n", iterations, elapsed_time);
    printf("Time per iteration: %.3f seconds\n", elapsed_time / iterations);

/*  printf("\ndavepl;%d;%.3f;1;algorithm=base,faithful=no;bits=1", iterations, elapsed_time);*/

    free(sieve);

    return 0;
}