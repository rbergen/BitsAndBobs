/* Sieve of Eratosthenes

   Originally written by Dave Plummer 08/06/2024 for the PDP-11 running
   211-BSD.

   Modified by Rutger van Bergen to compile for an Intel 8086 and run on
   MS-DOS.

   This program calculates prime numbers using the Sieve of Eratosthenes
   algorithm. It follows the basic rules of the Primes competition in
   Dave's Garage, except that it isn't "faithful" due to lack of sieve
   encapsulation. For that, considering the capabilities of C, the sieve
   buffer and limit would have to be wrapped in a struct that is created
   and torn down with every sieve pass.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Defaults and other constants */

#define DEFAULT_LIMIT   1000L
#define DEFAULT_SECONDS 5
#define BITSPERBYTE     8
#define TRUE 			   1
#define FALSE           0

/* Macros for bit manipulation */

#define GET_BIT(array, n) ((array[(n) / BITSPERBYTE] >> ((n) % BITSPERBYTE)) & 1)
#define SET_BIT(array, n) (array[(n) / BITSPERBYTE] |= (1 << ((n) % BITSPERBYTE)))

/* Structure to hold program options */

typedef struct {
    long limit;
    int seconds;
    int oneshot;
    int dragrace;
    int quiet;
} Options;

/* Structure to hold the expected results for a given limit */

typedef struct {
    long limit;
    long count;
} Result;

Result results_dictionary[] = {
    {10L, 4L},
    {100L, 25L},
    {1000L, 168L},
    {10000L, 1229L},
    {100000L, 9592L},
    {1000000L, 78498L},
    {10000000L, 664579L},
};

/* Program Help */

void print_help(progname)
char *progname;
{
    printf("Usage: %s [/l limit] [/s seconds] [/1|/d] [/q] [/h|/?]\n", progname);
    printf("Options:\n");
    printf("  /l limit     Specify the upper limit for prime calculation (default: 1000)\n");
    printf("  /s seconds   Specify the target duration in seconds for the run (default: 5)\n");
    printf("  /1           Run the sieve only once (oneshot mode)\n");
    printf("  /d           Also print dragrace format output\n");
    printf("  /q           Suppress banners and extraneous output\n");
    printf("  /h, /?       Print this help message and exit\n");
}

/* Unset option if set, with message */

int unset_if_set(other_ptr, msg, selected_name)
int *other_ptr;
char *msg;
char *selected_name;
{
	if (!*other_ptr)
   	return FALSE;

   *other_ptr = FALSE;
   printf(msg, selected_name);
   return TRUE;
}

#define ONESHOT_DRAGRACE_MSG "Warning: /1 and /d are mutually exclusive. Selecting %s mode.\n"

/* Parse command-line arguments and put them in an Options structure */

int parse_args(argc, argv, options_ptr, exit_code_ptr)
int argc;
char *argv[];
Options *options_ptr;
int *exit_code_ptr;
{
    int i;
    int warning_shown;

    warning_shown = FALSE;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '/') {
            switch (argv[i][1]) {
            case 'l':
            case 'L':
                if (argc > i + 1) {
                    options_ptr->limit = atol(argv[++i]);
                    continue;
                }
                break;
            case 's':
            case 'S':
                if (argc > i + 1) {
                    options_ptr->seconds = atoi(argv[++i]);
                    continue;
                }
                break;
            case '1':
                options_ptr->oneshot = TRUE;
                warning_shown |= unset_if_set(&options_ptr->dragrace, ONESHOT_DRAGRACE_MSG, "oneshot");
                continue;
            case 'q':
            case 'Q':
                options_ptr->quiet = TRUE;
                continue;
            case 'd':
            case 'D':
                options_ptr->dragrace = TRUE;
                warning_shown |= unset_if_set(&options_ptr->oneshot, ONESHOT_DRAGRACE_MSG, "dragrace");
                continue;
            case 'h':
            case 'H':
            case '?':
                print_help(argv[0]);
                *exit_code_ptr = 0;
                return TRUE;
            }
        }

        print_help(argv[0]);
        *exit_code_ptr = 1;
        return TRUE;
    }

    if (warning_shown)
    	  printf("\n");
    return FALSE;
}

/* Validate a limit versus an expected result */

int validate_results(limit, count)
long limit;
long count;
{
    int i;
    for (i = 0; i < sizeof(results_dictionary) / sizeof(Result); i++) {
        if (results_dictionary[i].limit == limit) {
            return results_dictionary[i].count == count;
        }
    }
    return FALSE;  /* No matching limit found */
}

/* Main program. Runs the sieve in accordance with command-line arguments passed. */

int main(argc, argv)
int argc;
char *argv[];
{
    Options options;
    int exit_code;
    long i, j;
    size_t size;
    long count;
    int passes;
    char *sieve;
    clock_t start_time, end_time;
    double elapsed_time;
    clock_t tick_duration;

    options.limit = DEFAULT_LIMIT;
    options.seconds = DEFAULT_SECONDS;
    options.oneshot = FALSE;
    options.dragrace = FALSE;
    options.quiet = FALSE;

    if (parse_args(argc, argv, &options, &exit_code))
        return exit_code;

    if (!options.quiet) {
        printf("------------------------------------------------------------------\n");
        printf("Sieve of Eratosthenes by Davepl 2024 for the PDP-11 running 211BSD\n");
        printf("Modified by rbergen to compile for an Intel 8086 and run on MS-DOS\n");
        printf("------------------------------------------------------------------\n\n");
        if (options.oneshot)
            printf("Solving primes up to %ld for one pass...", options.limit);
        else
            printf("Solving primes up to %ld for %d seconds...", options.limit, options.seconds);
    }

    size = (options.limit / 2) / BITSPERBYTE + 1;
    count = 1;  /* 2 is a prime number */
    sieve = (char *) malloc(size);

    if (sieve == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    passes = 0;
    tick_duration = options.seconds * CLK_TCK;
    start_time = clock();

    do {
        memset(sieve, 0, size);

        for (i = 3; i * i <= options.limit; i += 2)
            if (!GET_BIT(sieve, i / 2))
                for (j = i * i; j <= options.limit; j += 2 * i)
                    SET_BIT(sieve, j / 2);

        passes++;
        end_time = clock();
    } while (!options.oneshot && (end_time - start_time) < tick_duration);

    elapsed_time = (end_time - start_time) / CLK_TCK;

    for (i = 3; i <= options.limit; i += 2)
        if (!GET_BIT(sieve, i / 2))
            count++;

    free(sieve);

    if (!options.quiet)
        printf("\n---------------------------------------------\n");

    printf("Total time taken      : %.3f seconds\n", elapsed_time);
    printf("Number of passes      : %d\n", passes);
    printf("Time per pass         : %.3f seconds\n", elapsed_time / passes);
    printf("Count of primes found : %ld\n", count);
    printf("Prime validator       : %s\n", validate_results(options.limit, count) ? "PASS" : "FAIL");

    if (options.dragrace)
        printf("\ndavepl;%d;%.3f;1;algorithm=base,faithful=no;bits=1", passes, elapsed_time);

    return 0;
}