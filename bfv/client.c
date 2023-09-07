#include "client_server_func.h"

i64 send_Metadata(i64 sockfd, struct Metadata data);
i64 send_convdata(i64 sockfd, struct Ciphertext* ct_images, u64 size, i64 channels, i64 width, i64 height);
i64 send_ciphertext(i64 sockfd, struct Ciphertext ciphertext, u64 size);
struct Ciphertext* receive_convresults(i64 sockfd, u64 size, i64 *ff, i64 *fw, i64 *fh, i64* filters_per_ciphertext);
struct Ciphertext receive_ciphertext(i64 sockfd, u64 size);
i64 send_densedata(i64 sockfd, struct Ciphertext* ct_images, u64 size, i64 channels, i64 min_vector);
struct Ciphertext* receive_denseresults(i64 sockfd, u64 size, i64 *channels);

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));

  u64 poly_degree = 1<<11;
  u64 size = poly_degree;
  i64 mod = (i64)274877908993;
  i64 plaintext_mod = 1<<23+1;
  u64  nsize = 2;

  // size  <<= 2;
  // nsize <<= 2;

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


  // set up the connection to the server
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// set socket to nonblock mode
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

  i64 port_num = 12345;
  const char* address = "127.0.0.1";
  if (argc > 1) {
    sscanf(argv[1], "%ld", &port_num);
  }

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port_num);
	serverAddr.sin_addr.s_addr = inet_addr(address);
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	struct in_addr addr;

	if (inet_aton(address, &addr) == 0) {
		fprintf(stderr, "ERROR: Invalid Address\n");
		return 1;
	}

	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

	// connect to the server
	connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }

	int so_error;
	socklen_t len = sizeof(so_error);

	getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

	if (so_error != 0) {
		fprintf(stderr, "ERROR: Unable to connect to server\n");
		close(sockfd);
		return 1;
	}

  send_Metadata(sockfd, data);

  i64 i, j, k;
  i64 ff, fw, fh, filters_per_ciphertext;

  i64 channels, width, height;
  i64** image = load_image("./test_image/cifar_image.txt", &channels, &width, &height);


  struct Ciphertext *ct_images;
  struct Ciphertext *ct_outputs;

  // conv2d
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  // fprintf(stdout, "\tconv preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  // fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  // fprintf(stdout, "\tconv postprocess done\n");
  // fprintf(stdout, "\tclient (channels, width, height): (%ld, %ld, %ld)\n", channels, width, height);
  ReLU_images(image, channels, width, height);

  /*
  fprintf(stdout, "Done conv2d\n");
  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);
  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
  */

  // conv2d_1
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  fprintf(stdout, "\tclient (channels, width, height): (%ld, %ld, %ld)\n", channels, width, height);
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  fprintf(stdout, "\tclient: send_convdata done\n");
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  if (ct_outputs == NULL) { fprintf(stderr, "results == NULL\n"); return 1; }
  fprintf(stdout, "\tclient: receive_convresults done\n");
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  ReLU_images(image, channels, width, height);
  fprintf(stdout, "\tclient (channels, width, height): (%ld, %ld, %ld)\n", channels, width, height);

  mean_pool_images(image, &channels, &width, &height);
  fprintf(stdout, "\tDone mean_pooling\n");

  fprintf(stdout, "(%ld, %ld, %ld)\n", channels, width, height);
  for (i = 0; i < width*height; i++) {
    fprintf(stdout, "%2ld ", image[0][i]);
    if ( i % 15 == 14 )
      fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");

  // conv2d_2
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  fprintf(stdout, "\tconv_2 preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "\tconv_2 postprocess done\n");
  ReLU_images(image, channels, width, height);

  // conv2d_3
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  fprintf(stdout, "\tconv_3 preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "\tconv_3 postprocess done\n");
  ReLU_images(image, channels, width, height);

  mean_pool_images(image, &channels, &width, &height);
  fprintf(stdout, "\tDone mean_pooling\n");

  // conv2d_4
  ct_images = conv_preprocess(image, &channels, &width, &height, 1, data);
  fprintf(stdout, "\tconv_4 preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "\tconv_4 postprocess done\n");
  ReLU_images(image, channels, width, height);

  // conv2d_5
  ct_images = conv_preprocess(image, &channels, &width, &height, 0, data);
  fprintf(stdout, "\tconv_5 preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "\tconv_5 postprocess done\n");
  ReLU_images(image, channels, width, height);

  // conv2d_6
  ct_images = conv_preprocess(image, &channels, &width, &height, 0, data);
  fprintf(stdout, "\tconv_6 preprocess done\n");
  // send data for server to process
  send_convdata(sockfd, ct_images, size, channels, width, height);
  // receive data
  ct_outputs = receive_convresults(sockfd, size, &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "\tclient (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  image = conv_postprocess(ct_outputs, &channels, &width, &height, data, ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "\tconv_6 postprocess done\n");
  ReLU_images(image, channels, width, height);

  i64 min_vector;

  ct_images  = dense_preprocess(image, &channels, &width, &height, &min_vector, data);
  fprintf(stdout, "\tdense preprocess done\n");
  send_densedata(sockfd, ct_images, size, channels, min_vector);
  // receive data
  ct_outputs = receive_denseresults(sockfd, size, &channels);
  fprintf(stdout, "\tclient (channels): (%ld)\n", channels);
  // ct_outputs = dense_server("./miniONN_cifar_model/dense.kernel.txt", ct_images, &channels, min_vector, data);
  i64* results = dense_postprocess(ct_outputs, &channels, &width, &height, min_vector, data);
  fprintf(stdout, "\tdense postprocess done\n");

  scale_down(results, 10, 256);
  for (i = 0; i < 10; i++) {
    fprintf(stdout, "%ld ", results[i]);
  }
  fprintf(stdout, "\n");

  close(sockfd);
  return 0;
}

i64 send_Metadata(i64 sockfd, struct Metadata data) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;


  i64 size  = data.size;
  i64 nsize = data.nsize;

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &data, sizeof(struct Metadata), 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: data send failed");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, data.public_key.poly0, sizeof(i64) * size, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: public_key.poly0 send failed");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, data.public_key.poly1, sizeof(i64) * size, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: public_key.poly0 send failed");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, data.zetas, sizeof(i64) * size/nsize, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: zetas send failed");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, data.metas, sizeof(i64) * size/nsize, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: metas send failed");
    close(sockfd);
    return 1;
  }

  return 0;
}

i64 send_convdata(i64 sockfd, struct Ciphertext* ct_images, u64 size, i64 channels, i64 width, i64 height) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;


	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &channels, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &width, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &height, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

  i64 i;
  for (i = 0; i < channels; i++) {
    if (send_ciphertext(sockfd, ct_images[i], size))
      return 1;
  }
  return 0;
}

i64 send_densedata(i64 sockfd, struct Ciphertext* ct_images, u64 size, i64 channels, i64 min_vector) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &channels, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &min_vector, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

  i64 i;
  for (i = 0; i < channels; i++) {
    if (send_ciphertext(sockfd, ct_images[i], size))
      return 1;
  }
  return 0;
}


i64 send_ciphertext(i64 sockfd, struct Ciphertext ciphertext, u64 size) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, ciphertext.poly0, sizeof(i64) * size, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: poly0 send failed\n");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, ciphertext.poly1, sizeof(i64) * size, 0) == -1) {
    perror("send");
    fprintf(stderr, "ERROR: poly1 send failed\n");
    close(sockfd);
    return 1;
  }
  return 0;
}

struct Ciphertext receive_ciphertext(i64 sockfd, u64 size) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

  struct Ciphertext ciphertext;
  ciphertext.poly0 = (i64*) malloc( sizeof(i64) * size );
  ciphertext.poly1 = (i64*) malloc( sizeof(i64) * size );

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, ciphertext.poly0, sizeof(i64) * size)) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: Ciphertext.poly0 receive failed\n");
    exit(1);
    return ciphertext; 
  }

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, ciphertext.poly1, sizeof(i64) * size)) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: Ciphertext.poly1 receive failed\n");
    exit(1);
    return ciphertext;
  }

  return ciphertext;
}

struct Ciphertext* receive_convresults(i64 sockfd, u64 size, i64 *ff, i64 *fw, i64 *fh, i64* filters_per_ciphertext) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;


  struct Ciphertext* ct_outputs;

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, ff, sizeof(i64))) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: ff receive failed\n");
    exit(1);
    return NULL;
  }

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, fw, sizeof(i64))) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: fw receive failed\n");
    exit(1);
    return NULL;
  }

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, fh, sizeof(i64))) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: fh receive failed\n");
    exit(1);
    return NULL;
  }

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, filters_per_ciphertext, sizeof(i64))) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: filters_per_ciphertext receive failed\n");
    exit(1);
    return NULL;
  }
  // fprintf(stdout, "client (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", *ff, *fw, *fh, *filters_per_ciphertext);

  ct_outputs = (struct Ciphertext*) malloc(sizeof(struct Ciphertext) * (*ff));

  i64 i;
  for (i = 0; i < (*ff); i++) {
    ct_outputs[i] = receive_ciphertext(sockfd, size);
  }

  return ct_outputs;
}

struct Ciphertext* receive_denseresults(i64 sockfd, u64 size, i64 *channels) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

  struct Ciphertext* ct_outputs;

	FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(sockfd, &rfds); retval = select( sockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(sockfd, channels, sizeof(i64))) <= 0) { 
    perror("read");
    fprintf(stderr, "ERROR: channels receive failed\n");
    exit(1);
    return NULL;
  }

  ct_outputs = (struct Ciphertext*) malloc(sizeof(struct Ciphertext) * (*channels));

  i64 i;
  for (i = 0; i < (*channels); i++) {
    ct_outputs[i] = receive_ciphertext(sockfd, size);
  }

  return ct_outputs;
}