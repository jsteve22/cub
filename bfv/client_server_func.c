#include "client_server_func.h"

i64** load_image(const char* filename, i64* channels, i64* width, i64* height) {
  FILE* fptr = fopen(filename, "r");

  fscanf(fptr, "%ld %ld %ld\n", channels, height, width);

  i64** image = (i64**) malloc( sizeof(i64*) * *channels );
  i64 i, j;
  i64 image_size = (*width) * (*height);

  for (i = 0; i < *channels; i++) {
    image[i] = (i64*) malloc( sizeof(i64) *  image_size );
    for (j = 0; j < image_size; j++) {
      fscanf(fptr, "%ld", &(image[i][j]));
    }
  }

  fclose(fptr);
  return image;
}


void free_image(i64** image, i64 channels, i64 width, i64 height) {
  i64 i;
  for (i = 0; i < channels; i++) {
    free(image[i]);
  }
  free(image);
}

i64**** load_conv_weights(const char* filename, i64* filters, i64* channels, i64* width, i64* height) {
  FILE* fptr = fopen(filename, "r");

  fscanf(fptr, "%ld %ld %ld %ld\n", filters, channels, height, width);

  i64**** weights = (i64****) malloc( sizeof(i64***) * *filters );
  i64 i, j, k, h;

  for (i = 0; i < *filters; i++) {
    weights[i] = (i64***) malloc( sizeof(i64**) * *channels );
    for (j = 0; j < *channels; j++) {
      weights[i][j] = (i64**) malloc( sizeof(i64*) * *width );
      for (k = 0; k < *width; k++) {
        weights[i][j][k] = (i64*) malloc( sizeof(i64) * *height );
        for (h = 0; h < *height; h++) {
          fscanf(fptr, "%ld", &(weights[i][j][k][h]));
        }
      }
    }
  }

  fclose(fptr);
  return weights;
}

void free_conv_weights(i64**** weights, i64 filters, i64 channels, i64 width, i64 height) {
  i64 i, j, k;
  for (i = 0; i < filters; i++) {
    for (j = 0; j < channels; j++) {
      for (k = 0; k < width; k++) {
        free(weights[i][j][k]);
      }
      free(weights[i][j]);
    }
    free(weights[i]);
  }
  free(weights);
}

i64** load_dense_weights(const char* filename, i64* channels, i64* size) {
  FILE* fptr = fopen(filename, "r");

  fscanf(fptr, "%ld %ld\n", channels, size);

  i64** weights = (i64**) malloc( sizeof(i64*) * *channels );
  i64 i, j;

  for (i = 0; i < *channels; i++) {
    weights[i] = (i64*) malloc( sizeof(i64) *  *size );
    for (j = 0; j < *size; j++) {
      fscanf(fptr, "%ld", &(weights[i][j]));
    }
  }

  fclose(fptr);
  return weights;
}


void free_dense_weights(i64** weights, i64 channels, i64 size) {
  i64 i;
  for (i = 0; i < channels; i++) {
    free(weights[i]);
  }
  free(weights);
}

i64* prepare_filters(i64** filter, i64 filter_width, i64 filter_height, i64 image_width, i64 image_height, u64 size) {
  i64* remapped_filters = (i64*) malloc( sizeof(i64) * size );
  memset(remapped_filters, 0, sizeof(i64)*size);

  i64 fw = filter_width - 1;
  i64 fh = filter_height - 1;
  i64 i, j;

  for (i = 0; i < filter_height * image_width; i += image_width) {
    fw = filter_width - 1;
    for (j = 0; j < filter_width; j++) {
      remapped_filters[i+j] = filter[fh][fw];
      fw--;
    }
    fh--;
  }

  return remapped_filters;
}

void reformat_image(i64* image, i64 image_width, i64 image_height, i64 filter_width, i64 filter_height) {
  i64 i, j;
  i64 new_image_index, temp;

  i64 start_position = (image_width * (filter_height-1)) + filter_width - 1;

  new_image_index = 0;
  for (i = start_position; i < image_width*image_height; i += image_width) {
    for (j = 0; j < image_width - filter_width + 1; j++) {
      temp = image[i+j];
      image[i+j] = image[new_image_index];
      image[new_image_index] = temp;
      new_image_index++;
    }
  }
}

void reformat_images(i64** images, i64 channels, i64 image_width, i64 image_height, i64 filter_width, i64 filter_height, i64 filters_per_ciphertext) {
  i64 i, j;
  i64 new_image_index, temp;

  i64 start_position = (image_width * (filter_height-1)) + filter_width - 1;

  i64 *temp_image; 

  for (i = 0; i < channels; i+=filters_per_ciphertext) {
    for (j = 1; j < filters_per_ciphertext; j++) {
      temp_image = malloc(sizeof(i64) * image_width * image_height);
      memcpy(temp_image, &(images[i][j*image_width*image_height]), sizeof(i64) * image_width * image_height);
      reformat_image(temp_image, image_width, image_height, filter_width, filter_height);
      images[i+j] = temp_image;
    }
    reformat_image(images[i], image_width, image_height, filter_width, filter_height);
  }
}

void scale_images(i64** images, i64 channels, i64 width, i64 height, i64 scale) {
  i64 i, j;
  i64 image_size = width * height;
  for (i = 0; i < channels; i++) {
    for (j = 0; j < image_size; j++) {
      images[i][j] = images[i][j] / scale;
    }
  }
}

void ReLU_images(i64** images, i64 channels, i64 width, i64 height) {
  i64 i, j;
  i64 image_size = width * height;
  for (i = 0; i < channels; i++) {
    for (j = 0; j < image_size; j++) {
      if (images[i][j] < 0)
        images[i][j] = 0;
    }
  }
}

i64 *pad_image(i64* image, i64 *width, i64 *height) { 
  i64 padding = 2;
  i64 padded_width = (*width)+padding;
  i64 padded_height = (*height)+padding;
  i64 *padded_image = (i64*) malloc( sizeof(i64) * (padded_width) * (padded_height));
  memset(padded_image, 0, sizeof(i64)  * (padded_width) * (padded_height));
  i64 i, j, image_i, image_j;
  i64 start_index = padding/2;
  for (i = start_index; i < (*width)+start_index; i++) {
    for (j = start_index; j < (*height)+start_index; j++) {
      image_i = i - start_index;
      image_j = j - start_index;
      padded_image[(i*(padded_width))+j] = image[((image_i)*(*width))+image_j];
    }
  }
  *width  = padded_width;
  *height = padded_height;
  return padded_image;
}

void pad_images(i64** images, i64 *channels, i64 *width, i64 *height) {
  i64 orig_width = *width, orig_height = *height;
  i64 output_width, output_height;

  i64 *padded_image;

  i64 i;
  for (i = 0; i < *channels; i++) {
    padded_image = pad_image( images[i], &orig_width, &orig_height);

    free(images[i]);
    images[i] = padded_image;

    output_width = orig_width;
    output_height = orig_height;
    orig_width = *width;
    orig_height = *height;
  }

  // fprintf(stdout, "before pad_images - (width, height): (%ld, %ld)\n", *width, *height);
  *width = output_width;
  *height = output_height;
  // fprintf(stdout, "after  pad_images - (width, height): (%ld, %ld)\n", *width, *height);
}

i64* mean_pool_image(i64* image, i64 *width, i64 *height) {
  i64 filter_shape = 2;
  i64 pooled_width = (*width) / filter_shape;
  i64 pooled_height = (*height) / filter_shape;

  i64 i, j, filter_i, filter_j, sum;
  i64 *pooled_image = (i64*) malloc( sizeof(i64) * (pooled_width*pooled_height) );

  for (i = 0; i < pooled_width; i++) {
    for (j = 0; j < pooled_height; j++) {
      sum = 0;
      for (filter_i = 0; filter_i < filter_shape; filter_i++) {
        for (filter_j = 0; filter_j < filter_shape; filter_j++) {
          sum += image[ ((2*i+filter_i) * (*width)) + 2*j + filter_j ];
        }
      }
      pooled_image[(i*pooled_width)+j] = sum / (filter_shape * filter_shape);
    }
  }
  
  *width = pooled_width;
  *height = pooled_height;
  return pooled_image;
}

void mean_pool_images(i64** images, i64 *channels, i64 *width, i64 *height) {
  i64 orig_width = *width, orig_height = *height;
  i64 output_width, output_height;

  i64 *pooled_image;

  i64 i;
  for (i = 0; i < *channels; i++) {
    pooled_image = mean_pool_image( images[i], &orig_width, &orig_height);

    free(images[i]);
    images[i] = pooled_image;

    output_width = orig_width;
    output_height = orig_height;
    orig_width = *width;
    orig_height = *height;
  }

  *width = output_width;
  *height = output_height;
}

i64** conv_layer(const char* filename, i64** input, i64* channels, i64* width, i64* height, i64 padded, struct Metadata data) {
  // Load in metadata necessary for encryption/decryption
  struct Ciphertext *send_to_server;
  {
    u64 size = data.size;
    i64 mod = data.mod;
    i64 plaintext_mod = data.plaintext_mod;
    i64 *private_key = data.private_key;
    struct PublicKey public_key = data.public_key;
    u64 nsize = data.nsize;
    i64 *zetas = data.zetas;
    i64 *metas = data.metas;
    i64 i, j, k;

    // Image Pre-Processing

    // pad the images
    if (padded) {
      pad_images(input, channels, width, height);
    }
    if ( (*width) * (*height) > size ) {
      fprintf(stderr, "Error: cannot fit image into ciphertext\n");
    }
    // client side pre-processing (encrypting images)
    struct Ciphertext *ct_images = (struct Ciphertext*) malloc(sizeof(struct Ciphertext)* *channels);
    i64 *extended_image = (i64*) malloc( sizeof(i64) * size);
    for (i = 0; i < *channels; i++) {
      memset(extended_image, 0, sizeof(i64) * size);
      memcpy(extended_image, input[i], sizeof(i64) * (*width) * (*height) );
      ct_images[i] = zntt_encrypt(extended_image, public_key, size, mod, plaintext_mod, zetas, metas, nsize);
    }
    free(extended_image);

    // send to server
    send_to_server = ct_images;
  }
  
  i64 client_ff, client_fw, client_fh;
  struct Ciphertext *send_to_client;
  i64 client_filter_count;
  {
    struct Ciphertext *ct_images = send_to_server;
    u64 size = data.size;
    i64 mod = data.mod;
    i64 plaintext_mod = data.plaintext_mod;
    struct PublicKey public_key = data.public_key;
    u64 nsize = data.nsize;
    i64 *zetas = data.zetas;
    i64 *metas = data.metas;
    i64 i, j, k;
    // server side loading in the weights
    i64 ff, fc, fw, fh;
    i64**** weights = load_conv_weights(filename, &ff, &fc, &fw, &fh);

    // filter packing
    i64 filter_size = ((fw-1) * (*width)) + fh;
    i64 max_index_for_filters = size - filter_size;
    i64 image_size = (*width) * (*height);
    fprintf(stdout, "max_index_for_filters: %ld\n", max_index_for_filters);
    i64 filters_per_ciphertext = max_index_for_filters / image_size;
    if (filters_per_ciphertext == 0)
      filters_per_ciphertext = 1;
    while (ff % filters_per_ciphertext != 0)
      filters_per_ciphertext--;
    fprintf(stdout, "filters_per_ciphertext: %ld\n", filters_per_ciphertext);

    // initialize ciphertexts to 0
    struct Ciphertext *ct_outputs = (struct Ciphertext*) malloc(sizeof(struct Ciphertext) * (ff/filters_per_ciphertext) );
    i64* remapped_filter = (i64*) malloc(sizeof(i64) * size);
    i64* temp_filter;
    i64 chan = 0;
    for (i = 0; i < (ff/filters_per_ciphertext); i++) {
      ct_outputs[i].poly0 = calloc(size, sizeof(i64));
      ct_outputs[i].poly1 = calloc(size, sizeof(i64));
    }

    // Perform convolutions
    struct Ciphertext temp_a, temp_b;
    for (chan = 0; chan < *channels; chan++) {
      for (i = 0; i < ff/filters_per_ciphertext; i++) {
        memset(remapped_filter, 0, sizeof(i64) * size);
        for (j = 0; j < filters_per_ciphertext; j++) {
          temp_filter = prepare_filters(weights[(i*filters_per_ciphertext)+j][chan], fw, fh, *width, *height, size);
          memcpy(&(remapped_filter[j*image_size]), temp_filter, sizeof(i64) * filter_size);
          free(temp_filter);
        }
        temp_a = zntt_ciphertext_plaintext_poly_mult( ct_images[chan], remapped_filter, size, mod, zetas, metas, nsize);
        temp_b = ct_outputs[i];
        ct_outputs[i] = ciphertext_add( temp_b, temp_a, size, mod );
        ciphertext_free(temp_a);
        ciphertext_free(temp_b);
      }
    }
    free(remapped_filter);
    free_conv_weights(weights, ff, fc, fw, fh);

    // send to client 
    client_ff = ff;
    client_fh = fh;
    client_fw = fw;
    send_to_client = ct_outputs;
    client_filter_count = filters_per_ciphertext;
  }

  {
    i64 ff = client_ff;
    i64 fh = client_fh;
    i64 fw = client_fw;
    struct Ciphertext *ct_outputs = send_to_client;
    struct Ciphertext *ct_images = send_to_server;
    i64 filters_per_ciphertext = client_filter_count;
    u64 size = data.size;
    i64 mod = data.mod;
    i64 plaintext_mod = data.plaintext_mod;
    i64 *private_key = data.private_key;
    struct PublicKey public_key = data.public_key;
    u64 nsize = data.nsize;
    i64 *zetas = data.zetas;
    i64 *metas = data.metas;
    i64 i, j, k;
    // client side stuff
    // Decrypt all of the ciphertexts
    i64** outputs = (i64**) malloc(sizeof(i64*) * ff);
    for (i = 0; i < ff; i+=filters_per_ciphertext) {
      outputs[i] = zntt_decrypt( ct_outputs[i/filters_per_ciphertext], private_key, size, mod, plaintext_mod, zetas, metas, nsize);
      centerlift_polynomial(outputs[i], size, plaintext_mod);
      ciphertext_free( ct_outputs[i/filters_per_ciphertext] );
    }

    // Reformat all of the images
    reformat_images(outputs, ff, *width, *height, fw, fh, filters_per_ciphertext);

    // scale down the images
    scale_images(outputs, ff, *width, *height, 256);
    ReLU_images(outputs, ff, *width, *height);

    for (i = 0; i < *channels; i++) {
      ciphertext_free(ct_images[i]);
    }
    free(ct_images);
    free_image(input, *channels, *width, *height);

    *channels = ff;
    *width  = (*width - fw) + 1;
    *height = (*height - fh) + 1;
    return outputs;
  }
}

i64* reverse_vector(i64* vector, i64 size) {
  i64 *reversed = (i64*) malloc( sizeof(i64) * size );
  i64 i;

  for (i = 0; i < size; i++) {
    reversed[i] = vector[size-i-1];
  }

  return reversed;
}

void scale_down(i64* vector, i64 size, i64 scale) {
  i64 i;
  for (i = 0; i < size; i++) {
    vector[i] = vector[i] / scale;
  }
}

i64* dense_layer(const char* filename, i64** input, i64* channels, i64* width, i64* height, struct Metadata data) {
  struct Ciphertext *send_to_server;
  i64 minv;
  {
    // Load in metadata necessary for encryption/decryption
    u64 size = data.size;
    i64 mod = data.mod;
    i64 plaintext_mod = data.plaintext_mod;
    i64 *private_key = data.private_key;
    struct PublicKey public_key = data.public_key;
    u64 nsize = data.nsize;
    i64 *zetas = data.zetas;
    i64 *metas = data.metas;

    i64 i, j, k;

    i64 image_size = (*width) * (*height);
    i64 vector_size = (*channels) * image_size;
    i64 min_vector = size;
    while (vector_size % min_vector != 0) 
      min_vector--;

    // reshape the input as a 1d array
    i64* input_vector = (i64*) malloc( sizeof(i64) * image_size * *channels );
    for (i = 0; i < *channels; i++) {
      for (j = 0; j < image_size; j++) {
        input_vector[(i*image_size) + j] = input[i][j];
      }
    }

    i64 ciphertexts_per_vector = vector_size / min_vector;

    // Load the input as encrypted images
    struct Ciphertext *ct_inputs = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * ciphertexts_per_vector);
    i64 *vector_to_encrypt = (i64*) malloc( sizeof(i64) * size);
    for (i = 0; i < ciphertexts_per_vector; i++) {
      memset(vector_to_encrypt, 0, sizeof(i64) * size );
      memcpy(vector_to_encrypt, &(input_vector[i*min_vector]), sizeof(i64) * min_vector);
      ct_inputs[i] = zntt_encrypt(vector_to_encrypt, public_key, size, mod, plaintext_mod, zetas, metas, nsize);
    }

    send_to_server = ct_inputs;
    minv = min_vector;
  }

  i64 filter_channels;
  struct Ciphertext *send_to_client;
  {
    struct Ciphertext *ct_inputs = send_to_server;
    i64 min_vector = minv;
    i64 i, j, k;
    i64 fc, fs;
    i64 **weights = load_dense_weights(filename, &fc, &fs);
    // do the dot product all encrypted
    struct Ciphertext *ct_outputs = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * fc);
    for (i = 0; i < fc; i++) {
      ct_outputs[i] = __dense_layer(ct_inputs, weights[i], fs, min_vector, data);
    }
    filter_channels = fc;
    send_to_client = ct_outputs;
  }

  {
    u64 size = data.size;
    i64 mod = data.mod;
    i64 plaintext_mod = data.plaintext_mod;
    i64 *private_key = data.private_key;
    struct PublicKey public_key = data.public_key;
    u64 nsize = data.nsize;
    i64 *zetas = data.zetas;
    i64 *metas = data.metas;

    struct Ciphertext *ct_outputs = send_to_client;
    i64 fc = filter_channels;
    i64 min_vector = minv;
    i64 i, j, k;

    // decrypt the dot product
    i64 *output = (i64*) malloc( sizeof(i64) * fc );
    i64 *decrypted_output;
    for (i = 0; i < fc; i++) {
      decrypted_output = zntt_decrypt(ct_outputs[i], private_key, size, mod, plaintext_mod, zetas, metas, nsize);
      output[i] = decrypted_output[min_vector-1];
      free(decrypted_output);
    }

    centerlift_polynomial(output, fc, plaintext_mod);
    scale_down(output, fc, 256);
    return output;
  }
}

struct Ciphertext __dense_layer(struct Ciphertext* ct_inputs, i64* weights, i64 weights_size, i64 input_size, struct Metadata data) {
  // Load in metadata necessary for encryption/decryption
  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  i64 *private_key = data.private_key;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;

  i64 i, j, k;

  i64 ciphertext_count = weights_size / input_size;

  struct Ciphertext output;

  // get the dot product of first `input_size` elements
  i64 *reversed_slice, *temp_slice;
  i64 ct_index = 0;
  reversed_slice = (i64*) malloc( sizeof(i64) * size );
  memset(reversed_slice, 0, sizeof(i64) * size );

  // first dot product
  temp_slice = reverse_vector(weights, input_size);
  memset(reversed_slice, 0, sizeof(i64) * size );
  memcpy(reversed_slice, temp_slice, sizeof(i64) * input_size);
  free(temp_slice);
  output = zntt_ciphertext_plaintext_poly_mult( ct_inputs[ ct_index ], reversed_slice, size, mod, zetas, metas, nsize);

  // sum of rest of the dot products
  struct Ciphertext temp_a, temp_b;
  for (ct_index = 1; ct_index < ciphertext_count; ct_index++) {
    temp_slice = reverse_vector(&(weights[input_size*ct_index]), input_size);
    memset(reversed_slice, 0, sizeof(i64) * size );
    memcpy(reversed_slice, temp_slice, sizeof(i64) * input_size);
    free(temp_slice);
    temp_a = zntt_ciphertext_plaintext_poly_mult( ct_inputs[ ct_index ], reversed_slice, size, mod, zetas, metas, nsize);
    temp_b = output;
    output = ciphertext_add(temp_a, temp_b, size, mod);
    ciphertext_free(temp_a);
    ciphertext_free(temp_b);

    // boot strap
    /*
    if (ct_index % 2 == 0) {
      i64* decrypted = decrypt(output, private_key, size, mod, plaintext_mod);
      ciphertext_free(output);
      centerlift_polynomial(decrypted, size, plaintext_mod);
      output = encrypt(decrypted, public_key, size, mod, plaintext_mod);
      free(decrypted);
    }
    */
  }
  free(reversed_slice);

  return output;
}


struct Ciphertext* conv_preprocess(i64** input, i64* channels, i64* width, i64* height, i64 padded, struct Metadata data) {

  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  i64 *private_key = data.private_key;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;
  i64 i, j, k;

  // Image Pre-Processing

  // pad the images
  if (padded) {
    pad_images(input, channels, width, height);
  }
  if ( (*width) * (*height) > size ) {
    fprintf(stderr, "Error: cannot fit image into ciphertext\n");
  }
  // client side pre-processing (encrypting images)
  struct Ciphertext *ct_images = (struct Ciphertext*) malloc(sizeof(struct Ciphertext)* (*channels));
  i64 *extended_image = (i64*) malloc( sizeof(i64) * size);
  for (i = 0; i < *channels; i++) {
    memset(extended_image, 0, sizeof(i64) * size);
    memcpy(extended_image, input[i], sizeof(i64) * ((*width) * (*height)) );
    ct_images[i] = zntt_encrypt(extended_image, public_key, size, mod, plaintext_mod, zetas, metas, nsize);
  }
  free(extended_image);

  return ct_images;
}

struct Ciphertext* conv_server(const char* filename, struct Ciphertext* ct_images, i64* channels, i64* width, i64* height, struct Metadata data,
                                i64* filter_channels, i64* filter_width, i64* filter_height, i64* filters_per_ciphertext) {
  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;
  i64 i, j, k;
  // server side loading in the weights
  i64 ff, fc, fw, fh;
  i64**** weights = load_conv_weights(filename, &ff, &fc, &fw, &fh);

  // filter packing
  i64 filter_size = ((fw-1) * (*width)) + fh;
  i64 max_index_for_filters = size - filter_size;
  i64 image_size = (*width) * (*height);
  fprintf(stdout, "max_index_for_filters: %ld\n", max_index_for_filters);
  if (*filters_per_ciphertext == 0)
    *filters_per_ciphertext = 1;
  while (ff % *filters_per_ciphertext != 0)
    (*filters_per_ciphertext)--;
  fprintf(stdout, "filters_per_ciphertext: %ld\n", *filters_per_ciphertext);

  // initialize ciphertexts to 0
  struct Ciphertext *ct_outputs = (struct Ciphertext*) malloc(sizeof(struct Ciphertext) * (ff/(*filters_per_ciphertext)) );
  i64* remapped_filter = (i64*) malloc(sizeof(i64) * size);
  i64* temp_filter;
  i64 chan = 0;
  for (i = 0; i < (ff/(*filters_per_ciphertext)); i++) {
    ct_outputs[i].poly0 = calloc(size, sizeof(i64));
    ct_outputs[i].poly1 = calloc(size, sizeof(i64));
  }

  // Perform convolutions
  struct Ciphertext temp_a, temp_b;
  for (chan = 0; chan < *channels; chan++) {
    for (i = 0; i < ff/(*filters_per_ciphertext); i++) {
      memset(remapped_filter, 0, sizeof(i64) * size);
      for (j = 0; j < *filters_per_ciphertext; j++) {
        temp_filter = prepare_filters(weights[(i*(*filters_per_ciphertext))+j][chan], fw, fh, *width, *height, size);
        memcpy(&(remapped_filter[j*image_size]), temp_filter, sizeof(i64) * filter_size);
        free(temp_filter);
      }
      temp_a = zntt_ciphertext_plaintext_poly_mult( ct_images[chan], remapped_filter, size, mod, zetas, metas, nsize);
      temp_b = ct_outputs[i];
      ct_outputs[i] = ciphertext_add( temp_b, temp_a, size, mod );
      ciphertext_free(temp_a);
      ciphertext_free(temp_b);
    }
  }
  free(remapped_filter);
  free_conv_weights(weights, ff, fc, fw, fh);

  *filter_channels = ff;
  *filter_width   = fw;
  *filter_height  = fh;

  // send to client 
  return ct_outputs;
}

// struct ConvPostProcessOutput conv_postprocess(struct ConvPostProcessArgs args) {
i64** conv_postprocess(struct Ciphertext* ct_outputs, i64* channels, i64* width, i64* height, struct Metadata data, 
                        i64 filter_channels, i64 filter_width, i64 filter_height, i64 filters_per_ciphertext) {
  i64 ff = filter_channels;
  i64 fh = filter_height;
  i64 fw = filter_width;
  
  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  i64 *private_key = data.private_key;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;
  i64 i, j, k;

  // client side stuff
  // Decrypt all of the ciphertexts
  i64** outputs = (i64**) malloc(sizeof(i64*) * ff);
  for (i = 0; i < ff; i+=filters_per_ciphertext) {
    outputs[i] = zntt_decrypt( ct_outputs[i/filters_per_ciphertext], private_key, size, mod, plaintext_mod, zetas, metas, nsize);
    centerlift_polynomial(outputs[i], size, plaintext_mod);
    ciphertext_free( ct_outputs[i/filters_per_ciphertext] );
  }

  // Reformat all of the images
  reformat_images(outputs, ff, *width, *height, fw, fh, filters_per_ciphertext);

  // scale down the images
  scale_images(outputs, ff, *width, *height, 256);
  // ReLU_images(outputs, ff, width, height);

  *width = (*width) - fw + 1;
  *height = (*height) - fh + 1;
  *channels = ff;
  return outputs;
}


struct Ciphertext* dense_preprocess(i64** input, i64* channels, i64* width, i64* height, i64* min_vector, struct Metadata data) {
  // Load in metadata necessary for encryption/decryption
  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  i64 *private_key = data.private_key;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;

  i64 i, j, k;

  i64 image_size = (*width) * (*height);
  i64 vector_size = (*channels) * image_size;
  *min_vector = size;
  while (vector_size % (*min_vector) != 0) 
    (*min_vector)--;

  // reshape the input as a 1d array
  i64* input_vector = (i64*) malloc( sizeof(i64) * image_size * *channels );
  for (i = 0; i < *channels; i++) {
    for (j = 0; j < image_size; j++) {
      input_vector[(i*image_size) + j] = input[i][j];
    }
  }

  i64 ciphertexts_per_vector = vector_size / (*min_vector);

  // Load the input as encrypted images
  struct Ciphertext *ct_inputs = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * ciphertexts_per_vector);
  i64 *vector_to_encrypt = (i64*) malloc( sizeof(i64) * size);
  for (i = 0; i < ciphertexts_per_vector; i++) {
    memset(vector_to_encrypt, 0, sizeof(i64) * size );
    memcpy(vector_to_encrypt, &(input_vector[i*(*min_vector)]), sizeof(i64) * (*min_vector));
    ct_inputs[i] = zntt_encrypt(vector_to_encrypt, public_key, size, mod, plaintext_mod, zetas, metas, nsize);
  }

  *channels = ciphertexts_per_vector;
  return ct_inputs;
}

struct Ciphertext* dense_server(const char* filename, struct Ciphertext* ct_inputs, i64* channels, i64 min_vector, struct Metadata data) {
  i64 i;
  i64 fc, fs;
  i64 **weights = load_dense_weights(filename, &fc, &fs);
  // do the dot product all encrypted
  struct Ciphertext *ct_outputs = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * fc);
  for (i = 0; i < fc; i++) {
    ct_outputs[i] = __dense_layer(ct_inputs, weights[i], fs, min_vector, data);
  }

  *channels = fc;
  return ct_outputs;
}


i64* dense_postprocess(struct Ciphertext* ct_outputs, i64* channels, i64* width, i64* height, i64 min_vector, struct Metadata data) {
  u64 size = data.size;
  i64 mod = data.mod;
  i64 plaintext_mod = data.plaintext_mod;
  i64 *private_key = data.private_key;
  struct PublicKey public_key = data.public_key;
  u64 nsize = data.nsize;
  i64 *zetas = data.zetas;
  i64 *metas = data.metas;

  i64 fc = *channels;
  i64 i;

  // decrypt the dot product
  i64 *output = (i64*) malloc( sizeof(i64) * fc );
  i64 *decrypted_output;
  for (i = 0; i < fc; i++) {
    decrypted_output = zntt_decrypt(ct_outputs[i], private_key, size, mod, plaintext_mod, zetas, metas, nsize);
    output[i] = decrypted_output[min_vector-1];
    free(decrypted_output);
  }

  centerlift_polynomial(output, fc, plaintext_mod);
  scale_down(output, fc, 256);
  return output;
}