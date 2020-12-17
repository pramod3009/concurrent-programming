CFLAGS =  -Wall -g  -w -std=c++17

all: counter

counter : 
		g++ -o counter counter_rwlock.cpp -pthread ${CFLAGS}

make clean :
	rm counter 