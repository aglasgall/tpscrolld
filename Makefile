LDFLAGS=-lX11 -lXi -lXtst
CC=cc

all: tpscrolld

tpscrolld: tpscrolld.o
	$(CC) tpscrolld.o -o tpscrolld $(LDFLAGS)

clean:
	rm -f tpscrolld.o tpscrolld
