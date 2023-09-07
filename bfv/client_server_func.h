#ifndef CLIENT_SERVER_FUNC_H
#define CLIENT_SERVER_FUNC_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include<stdint.h>

#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "poly.h"
#include "ntt.h"
#include "bfv.h"

typedef uint64_t u64;
typedef int64_t  i64;

struct Metadata {
  u64 size;
  i64 mod;
  i64 plaintext_mod; 
  i64 psi;
  u64  nsize;
  i64* private_key;
  struct PublicKey public_key;
  i64* zetas;
  i64* metas;
};

i64** load_image(const char* filename, i64* channels, i64* width, i64* height);
void free_image(i64** image, i64 channels, i64 width, i64 height);

i64**** load_conv_weights(const char* filename, i64* filters, i64* channels, i64* width, i64* height);
void free_conv_weights(i64**** weights, i64 filters, i64 channels, i64 width, i64 height);

i64** load_dense_weights(const char* filename, i64* channels, i64* size);
void free_dense_weights(i64** weights, i64 channels, i64 size);

i64* prepare_filters(i64** filter, i64 filter_width, i64 filter_height, i64 image_width, i64 image_height, u64 size);
void reformat_images(i64** images, i64 channels, i64 image_width, i64 image_height, i64 filter_width, i64 filter_height, i64 filters_per_ciphertext);
void reformat_image(i64* image, i64 image_width, i64 image_height, i64 filter_width, i64 filter_height);

i64** conv_layer(const char* filename, i64** input, i64* channels, i64* width, i64* height, i64 padded, struct Metadata data);
void scale_images(i64** images, i64 channels, i64 width, i64 height, i64 scale);
void scale_down(i64* vector, i64 size, i64 scale);
i64 *pad_image(i64* image, i64 *width, i64 *height);
void pad_images(i64** images, i64 *channels, i64 *width, i64 *height);
void ReLU_images(i64** images, i64 channels, i64 width, i64 height);
i64* mean_pool_image(i64* image, i64 *width, i64 *height);
void mean_pool_images(i64** images, i64 *channels, i64 *width, i64 *height);

i64* dense_layer(const char* filename, i64** input, i64* channels, i64* width, i64* height, struct Metadata data);
struct Ciphertext __dense_layer(struct Ciphertext* ct_inputs, i64* weights, i64 weights_size, i64 input_size, struct Metadata data);
i64* reverse_vector(i64* vector, i64 size);


struct Ciphertext* conv_preprocess(i64** input, i64* channels, i64* width, i64* height, i64 padded, struct Metadata data);
struct Ciphertext* conv_server(const char* filename, struct Ciphertext* ct_inputs, i64* channels, i64* width, i64* height, struct Metadata data, 
                                i64* filter_channels, i64* filter_width, i64* filter_height, i64* filters_per_ciphertext);
i64** conv_postprocess(struct Ciphertext* ct_outputs, i64* channels, i64* width, i64* height, struct Metadata data, 
                        i64 filter_channels, i64 filter_width, i64 filter_height, i64 filters_per_ciphertext);

struct Ciphertext* dense_preprocess(i64** input, i64* channels, i64* width, i64* height, i64* min_vector, struct Metadata data);
struct Ciphertext* dense_server(const char* filename, struct Ciphertext* ct_inputs, i64* channels, i64 min_vector, struct Metadata data);
i64* dense_postprocess(struct Ciphertext* ct_outputs, i64* channels, i64* width, i64* height, i64 min_vector, struct Metadata data);

#endif