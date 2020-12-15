CFLAGS =  -Wall -g  -w -std=c++17

all: concurrent-bst

concurrent-bst : 
	g++ -o concurrent-bst concurrent-bst-main.cpp sense.h sense.cpp concurrent-bst.h concurrent-bst.cpp concurrent-bst-rw.h concurrent-bst-rw.cpp -pthread ${CFLAGS}

make clean :
	rm concurrent-bst