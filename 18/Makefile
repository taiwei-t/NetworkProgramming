.PHONY:clean all

CC=gcc
CXX=g++
CFLAGS=-Wall -g
CXXFLAGS=-g \
		 -DCHECK_PTHREAD_RETURN_VALUE \
		 -D_FILE_OFFSET_BITS=64 \
		 -Wall \
		 -Wextra \
		 -Werror \
		 -Wno-unused-parameter \
		 -Woverloaded-virtual \
		 -Wpointer-arith \
		 -Wshadow \
		 -Wwrite-strings \
 		 -march=native \
 		 -std=c++11 \
		 -rdynamic

BIN=echosrv echocli connect_test pollsrv epollsrv

all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)

