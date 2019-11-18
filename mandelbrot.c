#include "mandelbrot.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>


#define POWER 2
#define MAX_ITERS 2000
#define DIVERGENCE_BOUND 1.e5

typedef union {
    void* ptr;
    double fval;
    uint64_t lval;
    struct {
        uint32_t ivalh;
        uint32_t ivall;
    };
} ftoi;


static calc_t logP;
static calc_t logN;


// total index within data, frame number is
// (frame_idx / (width * height)), and index
// is similarly (frame_idx % (width * height))
static volatile atomic_uint frame_idx;
// checked each iteration of each thread, if not set,
// the thread terminates execution
static int work_should_continue;


static inline complex_t complex_add(const complex_t a, const complex_t b) {
    complex_t ret = {a.re + b.re, a.im + b.im};
    return ret;
}

static inline complex_t complex_sq(const complex_t z) {
    complex_t ret = {z.re * z.re - z.im * z.im, 2 * (z.re * z.im)};
    return ret;
}

static inline calc_t complex_mag2(const complex_t z) {
    return z.re * z.re + z.im * z.im;
}

static inline void complex_print(const complex_t c) {
    printf("re: %f\tim: %f\n", c.re, c.im);
}

static calc_t divergence(const complex_t c) {
    complex_t z = {0., 0.};
    uint i;
    double mag;
    for (i = 0; i < MAX_ITERS; ++i) {
        z = complex_add(complex_sq(z), c);
        mag = complex_mag2(z);
        if (mag >= (DIVERGENCE_BOUND * DIVERGENCE_BOUND)) {
            return i - log(log(mag) / logN) / logP;
        }
    }
    return -1.;
}


static void* _mandelbrot_worker_thread(void* args) {
    calc_t* dst = ((ftoi*) args)[0].ptr;
    const uint w = ((ftoi*) args)[1].ivalh;
    const uint h = ((ftoi*) args)[1].ivall;
    complex_t center = {((ftoi*) args)[2].fval, ((ftoi*) args)[3].fval};
    const calc_t width = ((ftoi*) args)[4].fval;
    size_t frames = ((ftoi*) args)[5].lval;
    double scale = ((ftoi*) args)[6].fval;

    uint32_t fi, max = w * h * frames;
    uint32_t frame, last_frame = 1000000, idx;

    complex_t in;
    calc_t blx, bly, dx, dy, aspect_ratio, wid;
    uint x, y;


    while (work_should_continue && (fi = atomic_fetch_add(&frame_idx, 1)) < max) {
        frame = fi / (w * h);
        idx = fi % (w * h);
        if (frame != last_frame) {
            wid = (frame == last_frame + 1) ? wid * scale : width * pow(scale, frame);
            aspect_ratio = (((calc_t) h) / ((calc_t) w));

            blx = center.re - wid;
            bly = center.im - aspect_ratio * wid;

            dx = 2 * wid / (calc_t) w;
            dy = 2 * aspect_ratio * wid / (calc_t) h;
        }
        last_frame = frame;
        y = idx / w;
        x = idx % w;
        in.re = blx + x * dx;
        in.im = bly + y * dy;
        dst[fi] = divergence(in);
    }
    return NULL;
}

static void _mandelbrot_single_thread(
            calc_t* dst, uint dst_width, uint dst_height, complex_t center,
            calc_t width, size_t frames, double scale) {
    complex_t in;
    calc_t blx, bly, dx, dy, aspect_ratio;
    uint x, y, idx = 0;

    aspect_ratio = (((calc_t) dst_height) / ((calc_t) dst_width));

    for (size_t frame = 0; frame < frames; ++frame) {
        blx = center.re - width;
        bly = center.im - aspect_ratio * width;

        dx = 2 * width / (calc_t) dst_width;
        dy = 2 * aspect_ratio * width / (calc_t) dst_height;
        for (y = 0; y < dst_height; ++y) {
            for (x = 0; x < dst_width; x++) {
                in.re = blx + x * dx;
                in.im = bly + y * dy;
                dst[idx++] = divergence(in);
            }
        }
        width *= scale;
    }
}


void mandelbrot(
            calc_t* dst, uint dst_width, uint dst_height, complex_t center,
            calc_t width, size_t frames, double scale, uint nr_threads) {
    logP = log(POWER);
    logN = log(DIVERGENCE_BOUND);

    if (nr_threads == 1) {
        _mandelbrot_single_thread(dst, dst_width, dst_height, center, width, frames, scale);
    }
    else {
        uint i;
        // construct argument list
        ftoi args[7];
        args[0].ptr = dst;
        args[1].ivalh = dst_width;
        args[1].ivall = dst_height;
        args[2].fval = center.re;
        args[3].fval = center.im;
        args[4].fval = width;
        args[5].lval = frames;
        args[6].fval = scale;

        pthread_t *threads = (pthread_t*) malloc(nr_threads * sizeof(pthread_t));
        if (threads == NULL) {
            printf("Unable to malloc struct for threads\n");
            return;
        }
        atomic_store(&frame_idx, 0);
        work_should_continue = 1;
        for (i = 0; i < nr_threads; ++i) {
            if (pthread_create(&threads[i], NULL, &_mandelbrot_worker_thread, (void*) args) != 0) {
                // terminate other threads
                printf("Failed to create a thread\n");
                work_should_continue = 0;
                nr_threads = i;
            }
        }
        for (i = 0; i < nr_threads; ++i) {
            pthread_join(threads[i], 0);
        }
    }
}


uint32_t word_endian_reverse(uint32_t fval) {
    return (((fval & 0x000000ff) << 24u) |
            ((fval & 0x0000ff00) <<  8u) |
            ((fval & 0x00ff0000) >>  8u) |
            ((fval & 0xff000000) >> 24u));
}

uint64_t dword_endian_reverse(uint64_t fval) {
    return (((fval & 0x00000000000000ff) << 56u) |
            ((fval & 0x000000000000ff00) << 40u) |
            ((fval & 0x0000000000ff0000) << 24u) |
            ((fval & 0x00000000ff000000) <<  8u) |
            ((fval & 0x000000ff00000000) >>  8u) |
            ((fval & 0x0000ff0000000000) >> 24u) |
            ((fval & 0x00ff000000000000) >> 40u) |
            ((fval & 0xff00000000000000) >> 56u));
}

int mandelbrot_write(calc_t* data, uint data_width, uint data_height, size_t frames, char *f, int endian_reverse) {
    FILE *file = fopen(f, "w");
    size_t size, i;
    if (file == NULL) {
        printf("Unable to open file %s for writing", f);
        return 0;
    }
    size = data_width * data_height * frames;
    if (endian_reverse) {
        for (i = 0; i < size; ++i) {
            ((ftoi*) data)[i].lval = dword_endian_reverse(((ftoi*) data)[i].lval);
        }
        data_width = word_endian_reverse(data_width);
        data_height = word_endian_reverse(data_height);
        frames = dword_endian_reverse(frames);
    }
    fwrite(&data_width, sizeof(uint), 1, file);
    fwrite(&data_height, sizeof(uint), 1, file);
    fwrite(&frames, sizeof(size_t), 1, file);
    if (fwrite(data, sizeof(calc_t), size, file) < size) {
        printf("Failed to write all %lu bytes of data\n", size);
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int mandelbrot_read(calc_t* dst, uint *w, uint *h, size_t *frames, char *f, int endian_reverse) {
    FILE *file = fopen(f, "r");
    size_t size, i;
    if (file == NULL) {
        printf("Unable to open file %s for writing", f);
        return 0;
    }
    if (   !fread(w, sizeof(uint), 1, file)
        || !fread(h, sizeof(uint), 1, file)
        || !fread(frames, sizeof(size_t), 1, file))
    {
        printf("Failed to read parameters\n");
        fclose(file);
        return 0;
    }
    if (endian_reverse) {
        *w = word_endian_reverse(*w);
        *h = word_endian_reverse(*h);
        *frames = dword_endian_reverse(*frames);
    }
    size = (*w) * (*h) * (*frames);
    if (fread(dst, sizeof(calc_t), size, file) < size)
    {
        printf("Failed to read data\n");
        fclose(file);
        return 0;
    }
    if (endian_reverse) {
        for (i = 0; i < size; ++i) {
            ((ftoi*) dst)[i].lval = dword_endian_reverse(((ftoi*) dst)[i].lval);
        }
    }
    fclose(file);
    return 1;
}
