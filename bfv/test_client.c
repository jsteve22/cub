#include "client_server_func.h"

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));

  if (argc > 1 && strcmp(argv[1], "2") == 0) {
    fprintf(stdout, "server\n");
    return 0;
  }

  u64 poly_degree = 1<<11;
  u64 size = poly_degree;
  i64 mod = (i64)274877908993;
  i64 plaintext_mod = 1<<23+1;
  u64  nsize = 2;

  // size  <<= 3;
  // nsize <<= 3;

  // i64 plaintext_mod = 2061584302081;
  i64 psi = (i64)124878001;
  i64 *zetas = zeta_powers(size, mod, psi, nsize);
  i64 *metas = zeta_mults(size, mod, psi, nsize);


  i64 *private_key = zntt_private_key_generate(size, mod, zetas, nsize);
  struct PublicKey public_key = zntt_public_key_generate(private_key, size, mod, zetas, metas, nsize);

  struct Metadata data;
  data.private_key = private_key;
  data.public_key = public_key;
  data.size = size;
  data.mod = mod;
  data.plaintext_mod = plaintext_mod;
  data.psi = psi;
  data.nsize = nsize;
  data.zetas = zetas;
  data.metas = metas;

  i64 i, j, k;
  i64 ff, fw, fh, filters_per_ciphertext;

  i64 channels, width, height;
  i64** image = load_image("./test_image/cifar_image.txt", &channels, &width, &height);


  struct Ciphertext *ct_images;
  struct Ciphertext *ct_outputs;

  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  fprintf(stdout, "conv preprocess done\n");
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "conv server done\n");
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "conv postprocess done\n");

  ReLU_images(image, channels, width, height);
  fprintf(stdout, "Done conv2d\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  /*
  image = conv_layer("./miniONN_cifar_model/conv2d.kernel.txt", image, &channels, &width, &height, 1, data);
  fprintf(stdout, "Done conv2d\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);
  */

  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  /*
  image = conv_layer("./miniONN_cifar_model/conv2d_1.kernel.txt", image, &channels, &width, &height, 1, data);
  fprintf(stdout, "Done conv2d_1\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);
  */
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_1.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  ReLU_images(image, channels, width, height);
  fprintf(stdout, "Done conv2d_1\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  mean_pool_images(image, &channels, &width, &height);
  fprintf(stdout, "Done mean_pooling\n");

  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  image = conv_layer("./miniONN_cifar_model/conv2d_2.kernel.txt", image, &channels, &width, &height, 1, data);
  fprintf(stdout, "Done conv2d_2\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  image = conv_layer("./miniONN_cifar_model/conv2d_3.kernel.txt", image, &channels, &width, &height, 1, data);
  fprintf(stdout, "Done conv2d_3\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  /*
  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
  */

  mean_pool_images(image, &channels, &width, &height);
  fprintf(stdout, "Done mean_pooling_1\n");

  image = conv_layer("./miniONN_cifar_model/conv2d_4.kernel.txt", image, &channels, &width, &height, 1, data);
  fprintf(stdout, "Done conv2d_4\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  image = conv_layer("./miniONN_cifar_model/conv2d_5.kernel.txt", image, &channels, &width, &height, 0, data);
  fprintf(stdout, "Done conv2d_5\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  image = conv_layer("./miniONN_cifar_model/conv2d_6.kernel.txt", image, &channels, &width, &height, 0, data);
  fprintf(stdout, "Done conv2d_6\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);

  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  /*
  i64* results = dense_layer("./miniONN_cifar_model/dense.kernel.txt", image, &channels, &width, &height, data);
  */

 i64 min_vector;

 ct_images  = dense_preprocess(image, &channels, &width, &height, &min_vector, data);
 ct_outputs = dense_server("./miniONN_cifar_model/dense.kernel.txt", ct_images, &channels, min_vector, data);
 i64* results = dense_postprocess(ct_outputs, &channels, &width, &height, min_vector, data);

  scale_down(results, 10, 256);
  for (i = 0; i < 10; i++) {
    fprintf(stdout, "%ld ", results[i]);
  }
  fprintf(stdout, "\n");

  // free_image(image, channels, width, height);
  free(private_key);
  publickey_free(public_key);

  return 0;
}
