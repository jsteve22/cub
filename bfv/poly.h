#ifndef POLY_H
#define POLY_H

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdint.h>

typedef uint64_t u64;
typedef int64_t  i64;

i64* poly_mult(i64* poly_a, i64* poly_b, u64 size);
i64* poly_add(i64* poly_a, i64* poly_b, u64 size);
void poly_mod(i64* poly, u64 size, i64 mod);
void shuffle(i64* poly, u64 size);
i64* copy_poly(i64* poly, u64 size);
void poly_print(i64* poly, u64 size);

#endif