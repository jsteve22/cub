#ifndef NTT_H
#define NTT_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include<stdint.h>

#include "poly.h"

typedef uint64_t u64;
typedef int64_t  i64;

i64 inverse_mod(i64 a, i64 q);
i64* NTT(i64* poly, u64 size, i64 mod, i64* psi_powers);
i64* iNTT(i64* poly, u64 size, i64 mod, i64* invpsi_powers);
i64* prepare_psi_powers(u64 size, i64 mod, i64 psi);
i64* prepare_invpsi_powers(u64 size, i64 mod, i64 invpsi);
i64 bitrev(i64 num, u64 bitsize);
i64 big_mult_mod(u64 x, u64 y, u64 mod);

i64* zeta_powers(u64 size, i64 mod, i64 psi, u64 nsize);
i64* zeta_mults(u64 size, i64 mod, i64 psi, u64 nsize);

i64* zNTT(i64* poly, u64 size, i64 mod, i64* zetas, u64 nsize);
i64* ziNTT(i64* poly, u64 size, i64 mod, i64* zetas, u64 nsize);
i64* zmult(i64* poly_a, i64* poly_b, u64 size, i64 mod, i64* metas, u64 nsize);
void basemul(i64* r, i64* poly_a, i64* poly_b, i64 mod, i64 zeta);
void testmul(i64* r, i64* poly_a, i64* poly_b, i64 mod, i64 zeta, u64 nsize);

#endif