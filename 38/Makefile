.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g

BIN=tsd

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

tsd:tsd.o
	$(CC) $(CFLAGS) $< -o $@ -lpthread

clean:
	rm -f *.o $(BIN)

