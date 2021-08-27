.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g

BIN= msg_stat msg_send msg_recv

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)

