#ifndef CHECK_H
#define CHECK_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* One failure ends the test program, with the line that failed. */
#define CHECK(condition) do { \
    if (!(condition)) { \
        printf("%s:%d: FAILED %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

#define CHECK_NEAR(actual, expected, tolerance) CHECK(fabs((double)(actual) - (double)(expected)) <= (tolerance))

#define RUN(test) do { test(); printf("ok  %s\n", #test); } while (0)

#endif
