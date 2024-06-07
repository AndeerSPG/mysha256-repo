CC=gcc
CFLAGS=-I. -lm
DEPS = s_my_sha256header.h
OBJ =  sha256-ehu-lib-sc.o mysha256-repo.o inserta_fichero.o extrae_fichero.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mysha256-repo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lcrypto 

clean:
	rm *.o mysha256-repo