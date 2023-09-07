#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdint.h>

#include "poly.h"
#include "ntt.h"
#include "bfv.h"

typedef uint64_t u64;
typedef int64_t  i64;

int main() {
  srand((unsigned int)time(NULL));
  srand((unsigned int)1337);

  u64 poly_degree = 1<<11;
  u64 size = poly_degree;
  i64 mod = (i64)274877908993;
  i64 plaintext_mod = (1<<23)+1;
  i64 nsize = 2;

  size  *= 2;
  nsize *= 2;

  i64 psi = (i64)124878001;
  // size = 256; mod = 3329; psi = 17;
  i64 invpsi = inverse_mod(psi, mod);
  i64 *psi_powers = prepare_psi_powers(size, mod, psi);
  i64 *invpsi_powers = prepare_invpsi_powers(size, mod, invpsi);
  i64 *zetas = zeta_powers(size, mod, psi, nsize);
  i64 *metas = zeta_mults(size, mod, psi, nsize);

  /*
  printf("metas: ");
  poly_print(metas, size/2);
  printf("\n");
  printf("zetas: ");
  poly_print(zetas, size/2);
  printf("\n");
  */

  i64 *test = (i64*) malloc( sizeof(i64) * size );
  i64 t;
  for (t = 0; t < size; t++) {
    test[t] = 0;
    if (t < 10)
      test[t] = t;
  }
  i64 *zest = zNTT(test, size, mod, zetas, nsize);
  i64 *p = private_key_generate(size);
  i64 *mest = zmult(zest, zNTT(p, size, mod, zetas, nsize), size, mod, metas, nsize);
  // i64 *mest = poly_add(zest, zest, size);
  i64 *best = ziNTT(mest, size, mod, zetas, nsize);

  printf("\n");
  poly_print(best, 64);
  printf("\n");
  best = poly_mult(test, p, size);
  poly_mod(best, size, mod);
  poly_print(best, 64);

  return 0;

  // fprintf(stdout, "mod = %ld\npsi = %ld\n", mod, psi);
  // fprintf(stdout, "p(mod n) = %ld\n", plaintext_mod % size);

  // i64 invsize = inverse_mod(size, mod);

  // i64 *private_key = private_key_generate(size);
  // struct PublicKey public_key = public_key_generate(private_key, size, mod);
  i64 *private_key = zntt_private_key_generate(size, mod, zetas, nsize);
  // struct PublicKey public_key = zntt_public_key_generate(private_key, size, mod, zetas, metas);
  // private_key = ziNTT(private_key, size, mod, zetas);
  // struct PublicKey public_key = public_key_generate(private_key, size, mod);
  struct PublicKey public_key = zntt_public_key_generate(private_key, size, mod, zetas, metas, nsize);
  // public_key.poly0 = ziNTT(public_key.poly0, size, mod, zetas);
  // public_key.poly1 = ziNTT(public_key.poly1, size, mod, zetas);
  // private_key = ziNTT(private_key, size, mod, zetas);
  // poly_print(public_key.poly0, 10);
  // poly_print(public_key.poly1, 10);
  // printf("\n\n");

  i64 *plaintext = (i64*) malloc( sizeof(i64) * size );
  i64 *filter = (i64*) malloc( sizeof(i64) * size );
  int i;
  for (i = 0; i < size; i++) {
    plaintext[i] = 0;
    filter[i] = 0;
    if (i < 9)
      plaintext[i] = i;
  }
  filter[0] = 4;
  filter[1] = 3;
  filter[3] = 2;
  filter[4] = 1;

  // struct Ciphertext ciphertext = encrypt(plaintext, public_key, size, mod, plaintext_mod);
  // struct Ciphertext product = ciphertext_plaintext_poly_mult(ciphertext, filter, size, mod);
  struct Ciphertext ciphertext = zntt_encrypt(plaintext, public_key, size, mod, plaintext_mod, zetas, metas, nsize);
  // ciphertext.poly0 = ziNTT(ciphertext.poly0, size, mod, zetas);
  // ciphertext.poly1 = ziNTT(ciphertext.poly1, size, mod, zetas);
  struct Ciphertext product = zntt_ciphertext_plaintext_poly_mult(ciphertext, filter, size, mod, zetas, metas, nsize);

  // i64 *decrypted_plaintext = decrypt(product, private_key, size, mod, plaintext_mod);
  i64 *decrypted_plaintext = zntt_decrypt(product, private_key, size, mod, plaintext_mod, zetas, metas, nsize);

  centerlift_polynomial(decrypted_plaintext, size, plaintext_mod);
  poly_print(decrypted_plaintext, 20);

  free(private_key);
  free(plaintext);
  free(decrypted_plaintext);
  free(filter);
  ciphertext_free(ciphertext);
  ciphertext_free(product);
  publickey_free(public_key);
  free(psi_powers);
  free(invpsi_powers);
  return 0;
}