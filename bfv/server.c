#include "client_server_func.h"


void handle_connection(i64 clientSockfd, i64 client_count);

struct Metadata receive_Metadata(i64 clientSockfd);
struct Ciphertext *receive_convdata(i64 clientSockfd, u64 size, i64 *channels, i64 *width, i64 *height);
struct Ciphertext receive_ciphertext(i64 clientSockfd, u64 size);
i64 send_ciphertext(i64 sockfd, struct Ciphertext ciphertext, u64 size);
i64 send_convresults(i64 sockfd, struct Ciphertext* ct_outputs, u64 size, i64 ff, i64 fw, i64 fh, i64 filters_per_ciphertext);
struct Ciphertext *receive_densedata(i64 clientSockfd, u64 size, i64 *channels, i64 *min_vector);
i64 send_denseresults(i64 sockfd, struct Ciphertext* ct_outputs, u64 size, i64 channels);

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));

  // set up the connection for handling connections
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int clientSockfd;

	// allow others to reuse the address
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) {
		perror("setsockopt");
		return 1;
	}

  i64 port_num = 12345;
  if (argc > 1) {
    sscanf(argv[1], "%ld", &port_num);
  }

	// bind address to socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 1;
	}
	
	// set socket to listen
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 1;
	}

	int client_count = 1;
	while (1) {
		// accept a new connection
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		// handle incoming connection from client
		handle_connection(clientSockfd, client_count);

		client_count++;
    close(clientSockfd);
    close(sockfd);
    break;
	}

  return 0;
}

void handle_connection(i64 clientSockfd, i64 client_count) {
	fprintf(stdout, "Connection (%d) \t Sockfd = %d\n", client_count, clientSockfd);

	int buflen = 2048, rc;
	char buf[2048];
	memset(buf, 0, buflen);
	int line = 0;

  struct Metadata data = receive_Metadata(clientSockfd);
  i64 size = data.size;

  i64 i, j, k;
  i64 channels, width, height;
  i64 ff, fw, fh, filters_per_ciphertext;
  i64 min_vector;

  struct Ciphertext *ct_images;
  struct Ciphertext *ct_outputs;

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  fprintf(stdout, "server received convdata\n");
  fprintf(stdout, "server (channels, width, height): (%ld, %ld, %ld)\n", channels, width, height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  fprintf(stdout, "server (channels, width, height): (%ld, %ld, %ld)\n", channels, width, height);
  fprintf(stdout, "server (ff, fw, fh, filters_per_ciphertext): (%ld, %ld, %ld, %ld)\n", ff, fw, fh, filters_per_ciphertext);
  fprintf(stdout, "conv server done\n");
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_1.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_2.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_3.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_4.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_5.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_convdata(clientSockfd, size, &channels, &width, &height);
  ct_outputs = conv_server("./miniONN_cifar_model/conv2d_6.kernel.txt", ct_images, &channels, &width, &height, data, 
                            &ff, &fw, &fh, &filters_per_ciphertext);
  send_convresults(clientSockfd, ct_outputs, size, ff, fw, fh, filters_per_ciphertext);

  ct_images = receive_densedata(clientSockfd, size, &channels, &min_vector);
  ct_outputs = dense_server("./miniONN_cifar_model/dense.kernel.txt", ct_images, &channels, min_vector, data);
  send_denseresults(clientSockfd, ct_outputs, size, channels);

	fprintf(stdout, "End of connection\n");
}

struct Metadata receive_Metadata(i64 clientSockfd) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) {
    perror("select");
    exit(1);
  } else if (retval == 0) {
    fprintf(stderr, "ERROR: Timeout after 10s\n");
    exit(1);
  }
  struct Metadata data;
  if ( (rc = read(clientSockfd, &data, sizeof(struct Metadata))) <= 0)
    return data;

  fprintf(stdout, "size:  %ld\n", data.size);
  fprintf(stdout, "mod:   %ld\n", data.mod);
  fprintf(stdout, "nsize: %ld\n", data.nsize);
  i64 size = data.size;
  i64 mod = data.mod;
  i64 nsize = data.nsize;

  // alloc size for public key, zetas, and metas
  data.public_key.poly0 = (i64*) malloc( sizeof(i64) * size );
  data.public_key.poly1 = (i64*) malloc( sizeof(i64) * size );
  data.zetas = (i64*) malloc( sizeof(i64) * (size/nsize) );
  data.metas = (i64*) malloc( sizeof(i64) * (size/nsize) );

  // get data for the public key, zetas, and metas

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, data.public_key.poly0, sizeof(i64) * size)) <= 0)
    return data;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, data.public_key.poly1, sizeof(i64) * size)) <= 0)
    return data;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, data.zetas, sizeof(i64) * size/nsize)) <= 0)
    return data;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, data.metas, sizeof(i64) * size/nsize)) <= 0)
    return data;

  return data;
}

struct Ciphertext receive_ciphertext(i64 clientSockfd, u64 size) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

  struct Ciphertext ciphertext;
  ciphertext.poly0 = (i64*) malloc( sizeof(i64) * size );
  ciphertext.poly1 = (i64*) malloc( sizeof(i64) * size );

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, ciphertext.poly0, sizeof(i64) * size)) <= 0) return ciphertext;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, ciphertext.poly1, sizeof(i64) * size)) <= 0) return ciphertext;

  return ciphertext;
}

struct Ciphertext *receive_convdata(i64 clientSockfd, u64 size, i64 *channels, i64 *width, i64 *height) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

  struct Ciphertext *ct_images;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, channels, sizeof(i64))) <= 0) return NULL;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, width, sizeof(i64))) <= 0) return NULL;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, height, sizeof(i64))) <= 0) return NULL;

  ct_images = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * (*channels));
  i64 i;
  for (i = 0; i < (*channels); i++) {
    ct_images[i] = receive_ciphertext(clientSockfd, size);
  }

  return ct_images;
}

i64 send_ciphertext(i64 sockfd, struct Ciphertext ciphertext, u64 size) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

  // send server metadata
  FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
  tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);

  if ( send(sockfd, ciphertext.poly0, sizeof(i64) * size, 0) == -1) {
    perror("send");
    close(sockfd);
    return 1;
  }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, ciphertext.poly1, sizeof(i64) * size, 0) == -1) {
    perror("send");
    close(sockfd);
    return 1;
  }
  return 0;
}

i64 send_convresults(i64 sockfd, struct Ciphertext* ct_outputs, u64 size, i64 ff, i64 fw, i64 fh, i64 filters_per_ciphertext) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &ff, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &fw, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &fh, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &filters_per_ciphertext, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

  i64 i;
  for (i = 0; i < ff; i++) {
    if (send_ciphertext(sockfd, ct_outputs[i], size))
      return 1;
  }
  fprintf(stdout, "finish send_convresults\n");
  return 0;
}

struct Ciphertext *receive_densedata(i64 clientSockfd, u64 size, i64 *channels, i64 *min_vector) {
  i64 rc;
	// Set timeout
	struct timeval tv;
	fd_set rfds;
	int retval;

  struct Ciphertext *ct_images;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, channels, sizeof(i64))) <= 0) return NULL;

	FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; FD_ZERO(&rfds); FD_SET(clientSockfd, &rfds); retval = select( clientSockfd+1, &rfds, NULL, NULL, &tv);
  if (retval == -1) { perror("select"); exit(1); } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); exit(1); }
  if ( (rc = read(clientSockfd, min_vector, sizeof(i64))) <= 0) return NULL;

  ct_images = (struct Ciphertext*) malloc( sizeof(struct Ciphertext) * (*channels));
  i64 i;
  for (i = 0; i < (*channels); i++) {
    ct_images[i] = receive_ciphertext(clientSockfd, size);
  }

  return ct_images;
}

i64 send_denseresults(i64 sockfd, struct Ciphertext* ct_outputs, u64 size, i64 channels) {
	// Set timeout
	struct timeval tv;
	fd_set wfds;
	int retval;

	FD_ZERO(&wfds); FD_SET(sockfd, &wfds);
	tv.tv_sec  = 60; tv.tv_usec = 0; retval = select( sockfd+1, NULL, &wfds, NULL, &tv);
	if (retval == -1) { perror("select"); close(sockfd); return 1; } else if (retval == 0) { fprintf(stderr, "ERROR: Timeout after 10s\n"); close(sockfd); return 1; }
  if ( send(sockfd, &channels, sizeof(i64), 0) == -1) { perror("send"); close(sockfd); return 1; }

  i64 i;
  for (i = 0; i < channels; i++) {
    if (send_ciphertext(sockfd, ct_outputs[i], size))
      return 1;
  }
  fprintf(stdout, "finish send_convresults\n");
  return 0;
}