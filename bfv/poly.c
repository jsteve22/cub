#include "poly.h"


i64* poly_mult(i64* poly_a, i64* poly_b, u64 size) {
  i64 *ret = (i64*) malloc( sizeof(i64) * size );

  int i, j;
  for (i = 0; i < size; i++)
    ret[i] = 0; 

  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      if ( (i+j) >= size ) ret[i+j-size] -= ( poly_a[i] * poly_b[j] );
      else ret[i+j] += ( poly_a[i] * poly_b[j] );
    }
  }

  return ret;
}

i64* poly_add(i64* poly_a, i64* poly_b, u64 size) {
  i64 *ret = (i64*) malloc( sizeof(i64) * size );

  int i;
  for (i = 0; i < size; i++)
    ret[i] = poly_a[i] + poly_b[i]; 

  return ret;
}

void poly_mod(i64* poly, u64 size, i64 mod) {
  int i;
  for (i = 0; i < size; i++) {
    poly[i] = (poly[i] % mod);
    if (poly[i] < 0) poly[i] += mod;
  }
}

void shuffle(i64* poly, u64 size) {
  int i, random;
  i64 temp;

  for (i = 0; i < size; i++) {
    random = rand() % size;
    temp = poly[i];
    poly[i] = poly[random];
    poly[random] = temp;
  }
}

i64* copy_poly(i64* poly, u64 size) {
  i64 *ret = (i64*) malloc( sizeof(i64) * size );

  u64 i;
  for (i = 0; i < size; i++)
    ret[i] = poly[i];

  return ret;
}

void poly_print(i64* poly, u64 size) {
  int i;
  for (i = 0; i < size; i++)
    fprintf(stdout, "%ld ", poly[i]);
  fprintf(stdout, "\n");
}
