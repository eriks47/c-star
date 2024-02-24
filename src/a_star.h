#ifndef A_STAR_H
#define A_STAR_H

#include <stdbool.h>
#include <stdio.h>

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))

#define AS_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "%s:%d: Assertion %s failed\n", __FILE__, __LINE__, #cond); \
            exit(1); \
        } \
    } while (0)

typedef struct {
    int x, y;
} coords_t;

coords_t compute_next_position(int *map, int w, int h, coords_t source, coords_t destination, bool(*is_traversable)(int));

#endif
