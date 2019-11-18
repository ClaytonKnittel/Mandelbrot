#include <stdint.h>
#include <stddef.h>

typedef uint32_t uint;
typedef double calc_t;

#define ENCODE_LITTLE_ENDIAN 0
#define ENCODE_BIG_ENDIAN 1

typedef struct complex {
    calc_t re, im;
} complex_t;

void mandelbrot(calc_t* dst, uint dst_width, uint dst_height, complex_t center, calc_t width, size_t frames, double scale, uint nr_threads);

int mandelbrot_write(calc_t* data, uint data_width, uint data_height, size_t frames, char *f, int endian_reverse);
int mandelbrot_read(calc_t* dst, uint *w, uint *h, size_t *frames, char *f, int endian_reverse);
