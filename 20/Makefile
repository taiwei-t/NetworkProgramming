.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g
CXX=g++
CXXFLAGS=-Wall -g

BIN=udp_chat_srv udp_chat_cli

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)

