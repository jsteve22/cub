#ifndef BFV_H
#define BFV_H

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdint.h>

#include "poly.h"
#include "ntt.h"

typedef uint64_t u64;
typedef int64_t  i64;

struct Ciphertext {
  i64* poly0;
  i64* poly1;
};

struct PublicKey {
  i64* poly0;
  i64* poly1;
};

i64* private_key_generate(u64 size);
struct PublicKey public_key_generate(i64* private_key, u64 size, i64 mod);
struct Ciphertext encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod);
i64* decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod);
struct Ciphertext ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod);
struct Ciphertext ciphertext_add(struct Ciphertext ciphertext_a, struct Ciphertext ciphertext_b, u64 size, i64 mod);
double randn(double mu, double sigma);

i64* ntt_private_key_generate(u64 size, u64 mod, i64* psi_powers);
struct PublicKey ntt_public_key_generate(i64* private_key, u64 size, i64 mod, i64* psi_powers);
struct Ciphertext ntt_encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod, i64* psi_powers);
i64* ntt_decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod, i64* invpsi_powers);
struct Ciphertext ntt_ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod, i64* psi_powers);
i64* ntt_poly_mult(i64* poly_a, i64* poly_b, u64 size, i64 mod);

i64* zntt_private_key_generate(u64 size, u64 mod, i64* zetas, u64 nsize);
struct PublicKey zntt_public_key_generate(i64* private_key, u64 size, i64 mod, i64* zetas, i64* metas, u64 nsize);
struct Ciphertext zntt_encrypt(i64* plaintext, struct PublicKey public_key, u64 size, i64 mod, i64 plaintext_mod, i64* zetas, i64* metas, u64 nsize);
i64* zntt_decrypt(struct Ciphertext ciphertext, i64* private_key, u64 size, i64 mod, i64 plaintext_mod, i64* zetas, i64* metas, u64 nsize);
struct Ciphertext zntt_ciphertext_plaintext_poly_mult(struct Ciphertext ciphertext, i64* plaintext, u64 size, i64 mod, i64* zetas, i64* metas, u64 nsize);
i64* zntt_poly_mult(i64* poly_a, i64* poly_b, u64 size, i64 mod, i64* metas, u64 nsize);

void ciphertext_free(struct Ciphertext ciphertext);
void publickey_free(struct PublicKey public_key);

void centerlift_polynomial(i64* poly, u64 size, i64 plaintext_mod);

#endif