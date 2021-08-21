.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g

BIN=echosrv echocli nofile_limit connect_test pollsrv

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)

