#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fibpan.h"
#include "mandelbrot.h"

#define OUTPUT "test.dat"
#define NTHREADS 13


void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result) {
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}


int main(int argc, char *argv[]) {
    time_t f, e;

    const size_t frames = 150LU;
    const uint w = 1200, h = 1200;
    struct timespec start, end, elapsed;

    calc_t *data = (calc_t*) malloc(w * h * frames * sizeof(calc_t));
    if (data == NULL) {
        printf("Unable to malloc %lu bytes\n", w * h * frames * sizeof(calc_t));
        return -1;
    }

    complex_t center = {-.77568377, .13646737};
    calc_t wid = .3;

#define CLOCK CLOCK_MONOTONIC
    clock_gettime(CLOCK, &start);
    mandelbrot(data, w, h, center, wid, frames, .95, NTHREADS);
    clock_gettime(CLOCK, &end);
    timespec_diff(&start, &end, &elapsed);
    printf("%lus\t%luus\n", elapsed.tv_sec, elapsed.tv_nsec / 1000);
#undef CLOCK

    mandelbrot_write(data, w, h, frames, OUTPUT, ENCODE_BIG_ENDIAN);
    free(data);

    // if (argc < 2)
    //     return -1;

    // arg = atoi(argv[1]);
    // f = clock();
    // res = fib(arg);
    // e = clock();
    // printf("fib(%d) = %lu\t in %f seconds\n", arg, res, ((float) (e - f)) / CLOCKS_PER_SEC);
    
    return 0;
}
