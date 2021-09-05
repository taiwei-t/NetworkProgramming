.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g

BIN=echosrv echocli 01thread

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

01thread:01thread.o
	$(CC) $(CFLAGS) -lpthread $< -o $@

echosrv:echosrv.o
	$(CC) $(CFLAGS) -lpthread $< -o $@
clean:
	rm -f *.o $(BIN)

