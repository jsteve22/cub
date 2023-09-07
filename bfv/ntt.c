#include "ntt.h"

i64* NTT(i64* poly, u64 size, i64 mod, i64* psi_powers) {
  i64 i, j;

  i64* res = copy_poly(poly, size);
  poly_mod(res, size, mod);
  // i64* res = poly;

  i64 m, k, jstart, jend, wi, t, u, l;

  m = 1;
  k = size>>1;
  for (;m < size; m = m<<1) {
    for (i = 0; i < m; i++) {
      jstart = 2 * i * k;
      jend = jstart + k;
      wi = psi_powers[ m+i ];
      for (j = jstart; j < jend; j++) {
        l = j + k;
        t = res[j] % mod;
        u = big_mult_mod((u64)res[l], (u64)wi, (u64)mod);
        /*
        if ((m == 256) && (j % 2 == 0)) {
          printf("m: %ld\ti: %ld\tj: %ld\tu: %ld\twi: %ld\tres[l]: %ld\n", m, i, j, u, wi, res[l]);
          // printf("k: %ld\n", k);
        }
        */
        res[j] = (t + u) % mod;
        res[l] = (t - u) % mod;
        if (res[j] < 0)   res[j] += mod;
        if (res[j+k] < 0) res[j+k] += mod;
      }
    }
    k = k>>1;
  }

  poly_mod(res, size, mod);

  return res;
}

i64* iNTT(i64* poly, u64 size, i64 mod, i64* invpsi_powers) {
  i64 i, j;

  i64* res = copy_poly(poly, size);

  i64 m, k, jstart, jend, wi, t, u, l, temp;

  m = size>>1;
  k = 1;
  for (;m >= 1; m = m>>1) {
    for (i = 0; i < m; i++) {
      jstart = 2 * i * k;
      jend = jstart + k;
      wi = invpsi_powers[ m+i ];
      for (j = jstart; j < jend; j++) {
        l = j + k;
        t = res[j] % mod;
        u = res[l] % mod;
        res[j]   = (t + u) % mod;
        temp = (t-u);
        if (temp < 0) temp += mod;
        res[l] = big_mult_mod((u64)temp, (u64)wi, (u64)mod);
        if (res[j] < 0) res[j] += mod;
        if (res[l] < 0) res[l] += mod;
      }
    }
    k = k<<1;
  }

  i64 invn = inverse_mod(size, mod);
  for (i = 0; i < size; i++) {
    res[i] = big_mult_mod((u64)res[i], (u64)invn, (u64)mod);
  }
  poly_mod(res, size, mod);

  return res;
}

i64* prepare_psi_powers(u64 size, i64 mod, i64 psi) {
  i64 i, j;
  i64* psi_powers = (i64*) malloc( sizeof(i64) * size );

  i64 logn = 0;
  i = 1;
  for (; i < size; i = i<<1)
    logn++;
  
  for (i = 0; i < size; i++) {
    psi_powers[i] = 1;
    for (j = 0; j < bitrev(i, logn); j++) {
      psi_powers[i] = big_mult_mod((u64)psi_powers[i], (u64)psi, (u64)mod);
    }
  }
  return psi_powers;
}

i64* prepare_invpsi_powers(u64 size, i64 mod, i64 invpsi) {
  i64 i, j;
  i64* invpsi_powers = (i64*) malloc( sizeof(i64) * size );

  i64 logn = 0;
  i = 1;
  for (; i < size; i = i<<1)
    logn++;

  for (i = 0; i < size; i++) {
    invpsi_powers[i] = 1;
    for (j = 0; j < bitrev(i, logn); j++) {
      invpsi_powers[i] = big_mult_mod((u64)invpsi_powers[i], (u64)invpsi, (u64)mod);
    }
  }
  return invpsi_powers;
}

i64 inverse_mod(i64 a, i64 q) {
  i64 t, nt, r, nr, temp, quo;

  t  = 0;
  nt = 1;
  r  = q;
  nr = a;

  while (nr != 0) {
    quo = r / nr;
    temp = nt;
    nt = (t - (quo * temp));
    t = temp;

    temp = nr;
    nr = (r - (quo * temp));
    r = temp;
  }

  if (t < 0)
    t += q;

  return t;
}

i64 bitrev(i64 num, u64 bitsize) {
  int i;
  i64 reversed = 0;
  for (i = 0; i < bitsize; i++) {
    if (num % 2 == 1) reversed++;
    reversed = reversed << 1;
    num = num >> 1;
  }

  return (reversed >> 1);
}

i64 big_mult_mod(u64 x, u64 y, u64 mod) {
  if ( (x < 0x100000000) && (y < 0x100000000) ) {
    return (x*y)%mod;
  }

  u64 x_low, x_high, y_low, y_high;
  u64 mask;
  mask = 0xffffffff;

  x_low  = (x&mask);
  x_high = (x>>16);
  x_high = (x_high>>16);

  y_low  = (y&mask);
  y_high = (y>>16);
  y_high = (y_high>>16);

  u64 z_low, z_mid, z_high;

  z_low  = x_low * y_low;
  z_mid  = (x_low * y_high) + (x_high * y_low);
  z_high = x_high * y_high;

  i64 shifting = 16;

  // shift and mod z_mid and z_high
  u64 i;
  for (i = 0; i < 32/shifting; i++) {
    z_mid  = (z_mid<<shifting) % mod;
    z_high = (z_high<<shifting) % mod;
    z_high = (z_high<<shifting) % mod;
  }

  i64 ret = (z_low%mod) + (z_mid%mod) + (z_high%mod);

  return (ret%mod);
}


void basemul(i64* r, i64* poly_a, i64* poly_b, i64 mod, i64 zeta) {
  r[0]  = big_mult_mod(poly_a[1], poly_b[1], mod);
  r[0]  = big_mult_mod(r[0], zeta, mod);
  r[0] += big_mult_mod(poly_a[0], poly_b[0], mod);
  r[0]  = (r[0] % mod);
  r[1]  = big_mult_mod(poly_a[0], poly_b[1], mod);
  r[1] += big_mult_mod(poly_a[1], poly_b[0], mod);
  r[1]  = (r[1] % mod);
  return;
}


i64* zeta_powers(u64 size, i64 mod, i64 psi, u64 nsize) {
  i64* zetas = (i64*) malloc(sizeof(i64) * size / nsize);
  i64 i, j, prod;

  i64 logn = 0;
  i = 1;
  for (; i < size/nsize; i = i<<1)
    logn++;

  for (i = 0; i < size/nsize; i++) {
    prod = 1;
    for (j = 0; j < bitrev(i, logn); j++) {
      prod = big_mult_mod(prod, psi, mod);
    }
    zetas[i] = prod;
  }
  return zetas;
}

i64* zeta_mults(u64 size, i64 mod, i64 psi, u64 nsize) {
  i64* zetas = (i64*) malloc(sizeof(i64) * size / nsize);
  i64 i, j, prod;

  i64 logn = 0;
  i = 1;
  for (; i < size/nsize; i = i<<1)
    logn++;

  for (i = 0; i < size/nsize; i++) {
    prod = 1;
    for (j = 0; j < 2*bitrev(i, logn)+1; j++) {
      prod = big_mult_mod(prod, psi, mod);
    }
    zetas[i] = prod;
  }
  return zetas;
}

i64* zNTT(i64* poly, u64 size, i64 mod, i64* zetas, u64 nsize) {
  i64 len, start, j, k, t, zeta;
  k = 1;
  i64 *r = (i64*) malloc(sizeof(i64) * size);
  memcpy(r, poly, sizeof(i64)*size );
  poly_mod(r, size, mod);
  for (len = size/2; len >= nsize; len = len >> 1) {
    for (start = 0; start < size; start = j + len) {
      zeta = zetas[k++];
      for (j = start; j < start + len; j++) {
        t = big_mult_mod(zeta, r[j + len], mod);
        r[j + len] = (r[j] - t) % mod;
        if (r[j + len] < 0) r[j + len] += mod;
        r[j] = (r[j] + t) % mod;
        if (r[j] < 0) r[j] += mod;
      }
    }
  }
  poly_mod(r, size, mod);
  return r;
}

i64* ziNTT(i64* poly, u64 size, i64 mod, i64* zetas, u64 nsize) {
  i64 len, start, j, k, t, zeta;
  k = (size/nsize) - 1;
  i64 *r = (i64*) malloc(sizeof(i64) * size);
  memcpy(r, poly, sizeof(i64)*size );
  poly_mod(r, size, mod);
  for (len = nsize; len <= size/2; len<<=1) {
    for (start = 0; start < size; start = j + len) {
      zeta = zetas[k--];
      for (j = start; j < start + len; j++) {
        t = r[j];
        r[j] = (t + r[j + len]) % mod;
        if (r[j] < 0) r[j] += mod;
        r[j + len] = (r[j + len] - t) % mod;
        if (r[j + len] < 0) r[j + len] += mod;
        r[j + len] = big_mult_mod(zeta, r[j + len], mod);
      }
    }
  }

  i64 invsize = inverse_mod(size/nsize, mod);
  for (j = 0; j < size; j++)
    r[j] = big_mult_mod(r[j], invsize, mod);
  poly_mod(r, size, mod);
  return r;
}

void testmul(i64* r, i64* poly_a, i64* poly_b, i64 mod, i64 zeta, u64 size) {
  i64 i, j;

  for (i = 1; i < size; i++) {
    for (j = size-i; j < size; j++) {
      r[i+j-size] += big_mult_mod(poly_a[i], poly_b[j], mod);
    }
  }

  for (i = 0; i < size; i++) {
    r[i] = big_mult_mod(r[i], zeta, mod);
  }

  for (i = 0; i < size; i++) {
    for (j = 0; j < size-i; j++) {
      r[i+j] += big_mult_mod(poly_a[i], poly_b[j], mod);
    }
  }

  poly_mod(r, size, mod);
}

i64* zmult(i64* poly_a, i64* poly_b, u64 size, i64 mod, i64* metas, u64 nsize) {
  i64 *prod = (i64*) malloc(sizeof(i64) * size);
  memset(prod, 0, sizeof(i64)*size);
  i64 i;
  for (i = 0; i < size/nsize; i++) {
    // basemul(&(prod[nsize*i]), &(poly_a[nsize*i]), &(poly_b[nsize*i]), mod, metas[i]);
    testmul(&(prod[nsize*i]), &(poly_a[nsize*i]), &(poly_b[nsize*i]), mod, metas[i], nsize);
  }

  return prod;
}