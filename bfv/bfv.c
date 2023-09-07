#include "bfv.h"

i64* private_key_generate(u64 size) {
  i64* private_key = (i64*) malloc( sizeof(i64) * size );
  int i;
  int hamming_weight = 64;
  if (size < 64)
    hamming_weight = size / 2;
  
  for (i = 0; i < size; i++) {
    private_key[i] = 0;
    if (i < hamming_weight)
      private_key[i] = 1;
  }

  shuffle(private_key, size);

  return private_key;
}

struct PublicKey public_key_generate(i64* private_key, u64 size, i64 mod) {
  struct PublicKey public_key;

  public_key.poly1 = (i64*) malloc( sizeof(i64) * size );
  i64* neg_a = (i64*) malloc( sizeof(i64) * size );

  // generate a
  int i;
  for (i = 0; i < size; i++) {
    public_key.poly1[i] = rand() % mod;
    neg_a[i] = public_key.poly1[i] * -1;
  }
  poly_mod( neg_a, size, mod );

  public_key.poly0 = poly_mult( neg_a, private_key, size );
  free(neg_a);

  double norm_dist_value;
  i64 norm_value;

  for (i = 0; i < size; i++) {
    norm_dist_value = randn(0, 4);
    norm_value = norm_dist_value;
    public_key.poly0[i] -= norm_value;
  }

  poly_mod( public_key.poly0, size, mod );
  poly_mod( public_key.poly1, size, mod );

  return public_key;
}

struct Ciphertext encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod) {
  i64 delta = mod / plaintext_mod;
  i64 *message = copy_poly(plaintext, size);

  int i;
  double norm_dist_value;
  i64 norm_value;
  
  for (i = 0; i < size; i++) {
    message[i] = message[i] * delta;
  }
  poly_mod(message, size, mod);

  i64 *u  = (i64*) malloc( sizeof(i64) * size );
  i64 *e1 = (i64*) malloc( sizeof(i64) * size );
  i64 *e2 = (i64*) malloc( sizeof(i64) * size );
  for (i = 0; i < size; i++) {
    u[i] = rand() % 2;
    norm_dist_value = randn(0, 2);
    norm_value = norm_dist_value;
    e1[i] = norm_value;
    norm_dist_value = randn(0, 2);
    norm_value = norm_dist_value;
    e2[i] = norm_value;
  }

  struct Ciphertext ciphertext;

  i64* temp0; 
  i64* temp1; 

  temp0 = poly_mult( public_key.poly0, u, size);
  temp1 = poly_add(temp0, e1, size);

  ciphertext.poly0 = poly_add(temp1, message, size);
  poly_mod(ciphertext.poly0, size, mod);
  free(temp0);
  free(temp1);

  temp0 = poly_mult( public_key.poly1, u, size);
  ciphertext.poly1 = poly_add(temp0, e2, size);
  poly_mod(ciphertext.poly1, size, mod);
  free(temp0);

  free(u);
  free(e1);
  free(e2);

  return ciphertext;
}

i64* decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod) {
  i64* temp;
  i64* ret;

  temp = poly_mult(ciphertext.poly1, private_key, size);
  poly_mod(temp, size, mod);

  ret = poly_add(ciphertext.poly0, temp, size);
  poly_mod(ret, size, mod);
  free(temp);

  int i;
  double scale, value, dividend;
  for (i = 0; i < size; i++) {
    value = ret[i];
    value *= (1.0 * plaintext_mod);
    dividend = mod;
    scale = (value / dividend) + 0.5;
    ret[i] = scale;
  }

  poly_mod(ret, size, plaintext_mod);

  return ret;
}

struct Ciphertext ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod) {
  struct Ciphertext product;
  product.poly0 = poly_mult(ciphertext.poly0, plaintext, size);
  product.poly1 = poly_mult(ciphertext.poly1, plaintext, size);

  poly_mod(product.poly0, size, mod);
  poly_mod(product.poly1, size, mod);

  return product;
}

struct Ciphertext ciphertext_add(struct Ciphertext ciphertext_a, struct Ciphertext ciphertext_b, u64 size, i64 mod) {
  struct Ciphertext sum;
  sum.poly0 = poly_add(ciphertext_a.poly0, ciphertext_b.poly0, size);
  sum.poly1 = poly_add(ciphertext_a.poly1, ciphertext_b.poly1, size);

  poly_mod(sum.poly0, size, mod);
  poly_mod(sum.poly1, size, mod);

  return sum;
}

double randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;
 
  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }
 
  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;
 
  call = !call;
 
  return (mu + sigma * (double) X1);
}

i64* ntt_private_key_generate(u64 size, u64 mod, i64* psi_powers) {
  i64* private_key = (i64*) malloc( sizeof(i64) * size );
  int i;
  int hamming_weight = 64;
  if (size < 64)
    hamming_weight = size / 2;
  
  for (i = 0; i < size; i++) {
    private_key[i] = 0;
    if (i < hamming_weight)
      private_key[i] = 1;
  }

  shuffle(private_key, size);

  i64* ret = NTT(private_key, size, mod, psi_powers);
  free(private_key);

  return ret;
}

struct PublicKey ntt_public_key_generate(i64* private_key, u64 size, i64 mod, i64* psi_powers) {
  struct PublicKey public_key;

  public_key.poly1 = (i64*) malloc( sizeof(i64) * size );
  i64* neg_a = (i64*) malloc( sizeof(i64) * size );

  // generate a
  int i;
  for (i = 0; i < size; i++) {
    public_key.poly1[i] = rand() % mod;
    neg_a[i] = public_key.poly1[i] * -1;
  }
  poly_mod( neg_a, size, mod );
  i64* temp = NTT(public_key.poly1, size, mod, psi_powers);
  free(public_key.poly1);
  public_key.poly1 = temp;
  temp = NTT(neg_a, size, mod, psi_powers);
  free(neg_a);
  neg_a = temp;

  public_key.poly0 = ntt_poly_mult( neg_a, private_key, size, mod);
  free(neg_a);

  double norm_dist_value;
  i64* norm_value = (i64*) malloc( sizeof(i64) * size );

  for (i = 0; i < size; i++) {
    norm_dist_value = randn(0, 4);
    norm_value[i] = norm_dist_value;
  }

  temp = NTT(norm_value, size, mod, psi_powers);
  free( norm_value );
  norm_value = poly_add(temp, public_key.poly0, size);
  free( public_key.poly0 );
  free( temp );
  public_key.poly0 = norm_value;

  poly_mod( public_key.poly0, size, mod );
  poly_mod( public_key.poly1, size, mod );

  return public_key;
}

struct Ciphertext ntt_encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod, i64* psi_powers) {
  i64 delta = mod / plaintext_mod;
  i64 *message = copy_poly(plaintext, size); // NTT(plaintext, size, mod, psi);
  i64 *temp;

  int i;
  double norm_dist_value;
  i64 norm_value;
  
  for (i = 0; i < size; i++) {
    message[i] = message[i] * delta;
  }
  poly_mod(message, size, mod);
  temp = NTT(message, size, mod, psi_powers);
  free(message);
  message = temp;

  i64 *u  = (i64*) malloc( sizeof(i64) * size );
  i64 *e1 = (i64*) malloc( sizeof(i64) * size );
  i64 *e2 = (i64*) malloc( sizeof(i64) * size );
  for (i = 0; i < size; i++) {
    u[i] = rand() % 2;
    norm_dist_value = randn(0, 4);
    norm_value = norm_dist_value;
    e1[i] = norm_value;
    norm_dist_value = randn(0, 4);
    norm_value = norm_dist_value;
    e2[i] = norm_value;
  }

  temp = NTT(u, size, mod, psi_powers);
  free(u);
  u = temp;

  temp = NTT(e1, size, mod, psi_powers);
  free(e1);
  e1 = temp;

  temp = NTT(e2, size, mod, psi_powers);
  free(e2);
  e2 = temp;

  struct Ciphertext ciphertext;

  i64* temp0; 
  i64* temp1; 

  temp0 = ntt_poly_mult( public_key.poly0, u, size, mod);
  temp1 = poly_add(temp0, e1, size);

  ciphertext.poly0 = poly_add(temp1, message, size);
  poly_mod(ciphertext.poly0, size, mod);
  free(temp0);
  free(temp1);

  temp0 = ntt_poly_mult( public_key.poly1, u, size, mod);
  ciphertext.poly1 = poly_add(temp0, e2, size);
  poly_mod(ciphertext.poly1, size, mod);
  free(temp0);

  free(u);
  free(e1);
  free(e2);

  return ciphertext;
}

i64* ntt_decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod, i64* invpsi_powers) {
  i64* temp;
  i64* ret;

  temp = ntt_poly_mult(ciphertext.poly1, private_key, size, mod);
  poly_mod(temp, size, mod);

  ret = poly_add(ciphertext.poly0, temp, size);
  poly_mod(ret, size, mod);
  free(temp);
  temp = iNTT(ret, size, mod, invpsi_powers);
  free(ret);
  ret = temp;

  int i;
  double scale, value, dividend;
  for (i = 0; i < size; i++) {
    value = ret[i];
    value *= (1.0 * plaintext_mod);
    dividend = mod;
    scale = (value / dividend) + 0.5;
    ret[i] = scale;
  }

  poly_mod(ret, size, plaintext_mod);

  return ret;
}

struct Ciphertext ntt_ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod, i64* psi_powers) {
  struct Ciphertext product;
  i64* temp = NTT(plaintext, size, mod, psi_powers);
  plaintext = temp;
  product.poly0 = ntt_poly_mult(ciphertext.poly0, plaintext, size, mod);
  product.poly1 = ntt_poly_mult(ciphertext.poly1, plaintext, size, mod);

  poly_mod(product.poly0, size, mod);
  poly_mod(product.poly1, size, mod);

  return product;
}

i64* ntt_poly_mult(i64* poly_a, i64* poly_b, u64 size, i64 mod) {
  i64 *ret = (i64*) malloc( sizeof(i64) * size );

  int i;

  for (i = 0; i < size; i++) {
    ret[i] = big_mult_mod(poly_a[i], poly_b[i], mod);
  }

  return ret;
}


i64* zntt_private_key_generate(u64 size, u64 mod, i64* zetas, u64 nsize) {
  i64* private_key = (i64*) malloc( sizeof(i64) * size );
  int i;
  int hamming_weight = 64;
  if (size < 64)
    hamming_weight = size / 2;
  
  for (i = 0; i < size; i++) {
    private_key[i] = 0;
    if (i < hamming_weight)
      private_key[i] = 1;
  }

  shuffle(private_key, size);

  i64* ret = zNTT(private_key, size, mod, zetas, nsize);
  free(private_key);

  return ret;
}

struct PublicKey zntt_public_key_generate(i64* private_key, u64 size, i64 mod, i64* zetas, i64* metas, u64 nsize) {
  struct PublicKey public_key;

  public_key.poly1 = (i64*) malloc( sizeof(i64) * size );
  i64* neg_a = (i64*) malloc( sizeof(i64) * size );

  // generate a
  int i;
  for (i = 0; i < size; i++) {
    public_key.poly1[i] = rand() % mod;
    neg_a[i] = public_key.poly1[i] * -1;
  }
  poly_mod( neg_a, size, mod );
  i64* temp = zNTT(public_key.poly1, size, mod, zetas, nsize);
  free(public_key.poly1);
  public_key.poly1 = temp;
  temp = zNTT(neg_a, size, mod, zetas, nsize);
  free(neg_a);
  neg_a = temp;

  /*
  fprintf(stdout, "z private key: ");
  poly_print(ziNTT(private_key, size, mod, zetas), size);
  fprintf(stdout, "\n\n");
  fprintf(stdout, "z neg_a: ");
  poly_print(ziNTT(neg_a, size, mod, zetas), size);
  fprintf(stdout, "\n\n");
  */

  /*
  printf("metas: ");
  poly_print(metas, size/2);
  printf("\n");
  printf("zetas: ");
  poly_print(zetas, size/2);
  printf("\n");
  */

  public_key.poly0 = zmult( neg_a, private_key, size, mod, metas, nsize);
  free(neg_a);

  double norm_dist_value;
  i64* norm_value = (i64*) malloc( sizeof(i64) * size );

  for (i = 0; i < size; i++) {
    norm_dist_value = randn(0, 1);
    norm_value[i] = norm_dist_value;
  }

  temp = zNTT(norm_value, size, mod, zetas, nsize);
  free( norm_value );
  norm_value = poly_add(temp, public_key.poly0, size);
  free( public_key.poly0 );
  free( temp );
  public_key.poly0 = norm_value;

  poly_mod( public_key.poly0, size, mod );
  poly_mod( public_key.poly1, size, mod );

  return public_key;
}

struct Ciphertext zntt_encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod, i64* zetas, i64* metas, u64 nsize) {
  i64 delta = mod / plaintext_mod;
  i64 *message = copy_poly(plaintext, size); // NTT(plaintext, size, mod, psi);
  i64 *temp;

  int i;
  double norm_dist_value;
  i64 norm_value;
  
  for (i = 0; i < size; i++) {
    message[i] = message[i] * delta;
  }
  poly_mod(message, size, mod);
  temp = zNTT(message, size, mod, zetas, nsize);
  free(message);
  message = temp;

  i64 *u  = (i64*) malloc( sizeof(i64) * size );
  i64 *e1 = (i64*) malloc( sizeof(i64) * size );
  i64 *e2 = (i64*) malloc( sizeof(i64) * size );
  for (i = 0; i < size; i++) {
    u[i] = rand() % 2;
    norm_dist_value = randn(0, 1);
    norm_value = norm_dist_value;
    e1[i] = norm_value;
    norm_dist_value = randn(0, 1);
    norm_value = norm_dist_value;
    e2[i] = norm_value;
  }

  temp = zNTT(u, size, mod, zetas, nsize);
  free(u);
  u = temp;

  temp = zNTT(e1, size, mod, zetas, nsize);
  free(e1);
  e1 = temp;

  temp = zNTT(e2, size, mod, zetas, nsize);
  free(e2);
  e2 = temp;

  struct Ciphertext ciphertext;

  i64* temp0; 
  i64* temp1; 

  temp0 = zntt_poly_mult( public_key.poly0, u, size, mod, metas, nsize);
  temp1 = poly_add(temp0, e1, size);

  ciphertext.poly0 = poly_add(temp1, message, size);
  poly_mod(ciphertext.poly0, size, mod);
  free(temp0);
  free(temp1);

  temp0 = zntt_poly_mult( public_key.poly1, u, size, mod, metas, nsize);
  ciphertext.poly1 = poly_add(temp0, e2, size);
  poly_mod(ciphertext.poly1, size, mod);
  free(temp0);

  free(u);
  free(e1);
  free(e2);

  return ciphertext;
}

i64* zntt_decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod, i64* zetas, i64* metas, u64 nsize) {
  i64* temp;
  i64* ret;

  temp = zntt_poly_mult(ciphertext.poly1, private_key, size, mod, metas, nsize);
  poly_mod(temp, size, mod);

  ret = poly_add(ciphertext.poly0, temp, size);
  poly_mod(ret, size, mod);
  free(temp);
  temp = ziNTT(ret, size, mod, zetas, nsize);
  free(ret);
  ret = temp;

  int i;
  double scale, value, dividend;
  for (i = 0; i < size; i++) {
    value = ret[i];
    value *= (1.0 * plaintext_mod);
    dividend = mod;
    scale = (value / dividend) + 0.5;
    ret[i] = scale;
  }

  poly_mod(ret, size, plaintext_mod);

  return ret;
}

struct Ciphertext zntt_ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod, i64* zetas, i64* metas, u64 nsize) {
  struct Ciphertext product;
  i64* temp = zNTT(plaintext, size, mod, zetas, nsize);
  plaintext = temp;
  product.poly0 = zntt_poly_mult(ciphertext.poly0, plaintext, size, mod, metas, nsize);
  product.poly1 = zntt_poly_mult(ciphertext.poly1, plaintext, size, mod, metas, nsize);

  poly_mod(product.poly0, size, mod);
  poly_mod(product.poly1, size, mod);

  return product;

 /*
  struct Ciphertext product;
  product.poly0 = (i64*) calloc(sizeof(i64), size);
  product.poly1 = (i64*) calloc(sizeof(i64), size);
  struct Ciphertext ct_mask;

  i64 bits = 64;
  i64 shift = 10;
  i64 mask = (1<<shift)-1;

  i64 mask_cnt = bits / shift;
  mask_cnt = 3;

  i64 *pt_mask = (i64*) malloc(sizeof(i64) * size);
  i64* temp;

  i64 i, j, k;
  for (i = 0; i < mask_cnt; i++) {
    for (j = 0; j < size; j++) {
      pt_mask[j] = (plaintext[j] >> (shift*i));
      if (i != mask_cnt-1) {
        pt_mask[j] &= mask;
      }
    }

    temp = zNTT(pt_mask, size, mod, zetas, nsize); free(pt_mask); pt_mask = temp;

    ct_mask.poly0 = zntt_poly_mult(ciphertext.poly0, pt_mask, size, mod, metas, nsize);
    ct_mask.poly1 = zntt_poly_mult(ciphertext.poly1, pt_mask, size, mod, metas, nsize);
    for (j = 0; j < size; j++) {
      for (k = 0; k < i; k++) {
        ct_mask.poly0[j] = (ct_mask.poly0[j] << shift) % mod;
        ct_mask.poly1[j] = (ct_mask.poly1[j] << shift) % mod;
      }
    }
    product = ciphertext_add(product, ct_mask, size, mod);
    ciphertext_free(ct_mask);
  }

  return product;
  */

  /*
  struct Ciphertext product;
  struct Ciphertext ct_bot, ct_mid, ct_top;
  i64* p_bot = (i64*) malloc( sizeof(i64)*size );
  i64* p_mid = (i64*) malloc( sizeof(i64)*size );
  i64* p_top = (i64*) malloc( sizeof(i64)*size );

  i64 shift = 25;
  i64 mask = (1<<shift)-1;

  i64 i;
  for (i = 0; i < size; i++) {
    p_bot[i] = plaintext[i] & mask;
    p_mid[i] = (plaintext[i] >> shift) & mask;
    p_top[i] = plaintext[i] >> (2*shift);
  }

  i64* temp;
  temp = zNTT(p_bot, size, mod, zetas, nsize); free(p_bot); p_bot = temp;
  temp = zNTT(p_mid, size, mod, zetas, nsize); free(p_mid); p_mid = temp;
  temp = zNTT(p_top, size, mod, zetas, nsize); free(p_top); p_top = temp;

  ct_bot.poly0 = zntt_poly_mult(ciphertext.poly0, p_bot, size, mod, metas, nsize);
  ct_bot.poly1 = zntt_poly_mult(ciphertext.poly1, p_bot, size, mod, metas, nsize);

  ct_mid.poly0 = zntt_poly_mult(ciphertext.poly0, p_mid, size, mod, metas, nsize);
  ct_mid.poly1 = zntt_poly_mult(ciphertext.poly1, p_mid, size, mod, metas, nsize);

  ct_top.poly0 = zntt_poly_mult(ciphertext.poly0, p_top, size, mod, metas, nsize);
  ct_top.poly1 = zntt_poly_mult(ciphertext.poly1, p_top, size, mod, metas, nsize);

  for (i = 0; i < size; i++) {
    ct_mid.poly0[i] = (ct_mid.poly0[i] << shift) % mod;
    ct_mid.poly1[i] = (ct_mid.poly1[i] << shift) % mod;

    ct_top.poly0[i] = (ct_top.poly0[i] << shift) % mod;
    ct_top.poly1[i] = (ct_top.poly1[i] << shift) % mod;
    ct_top.poly0[i] = (ct_top.poly0[i] << shift) % mod;
    ct_top.poly1[i] = (ct_top.poly1[i] << shift) % mod;
  }

  product = ciphertext_add(ct_bot, ct_top, size, mod);
  product = ciphertext_add(product, ct_mid, size, mod);
  free(p_bot);
  free(p_mid);
  free(p_top);
  ciphertext_free(ct_bot);
  ciphertext_free(ct_mid);
  ciphertext_free(ct_top);
  return product;
  */
}

i64* zntt_poly_mult(i64* poly_a, i64* poly_b, u64 size, i64 mod, i64* metas, u64 nsize) {
  i64 *ret;

  ret = zmult(poly_a, poly_b, size, mod, metas, nsize);

  return ret;
}



void ciphertext_free(struct Ciphertext ciphertext) {
  free(ciphertext.poly0);
  free(ciphertext.poly1);
}

void publickey_free(struct PublicKey public_key) {
  free(public_key.poly0);
  free(public_key.poly1);
}

void centerlift_polynomial(i64* poly, u64 size, i64 plaintext_mod) {
  u64 i;
  i64 half_plaintext_mod = plaintext_mod / 2;
  for (i = 0; i < size; i++) {
    if (poly[i] > half_plaintext_mod) {
      poly[i] -= plaintext_mod;
    }
  }
}