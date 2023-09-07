OPFLAG := -O3

all: test cub ntt_cub zntt_cub client server

cub: cub.o bfv.o poly.o ntt.o
	gcc cub.o bfv.o poly.o ntt.o -o cub -lm -Wall -Werror $(OPFLAG)

ntt_cub: ntt_cub.o bfv.o poly.o ntt.o
	gcc ntt_cub.o bfv.o poly.o ntt.o -o ntt_cub -lm -Wall -Werror $(OPFLAG)

zntt_cub: zntt_cub.o bfv.o poly.o ntt.o
	gcc zntt_cub.o bfv.o poly.o ntt.o -o zntt_cub -lm -Wall -Werror $(OPFLAG)

client: client.o bfv.o poly.o ntt.o client_server_func.o
	gcc client.o bfv.o poly.o ntt.o client_server_func.o -o client -lm -lpthread -Wall -Werror $(OPFLAG)

server: server.o bfv.o poly.o ntt.o client_server_func.o
	gcc server.o bfv.o poly.o ntt.o client_server_func.o -o server -lm -lpthread -Wall -Werror $(OPFLAG)

test: test_bfv.o bfv.o poly.o ntt.o
	gcc test_bfv.o bfv.o poly.o ntt.o -o test -lm -Wall -Werror $(OPFLAG)

%.o: %.c
	gcc -c $< -o $@ $(OPFLAG)

clean:
	rm -rf *.o test cub ntt_cub zntt_cub client server
