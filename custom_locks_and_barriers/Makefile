CFLAGS =  -Wall -g -w -std=c++11

all: counter


counter : 
	g++ counter.cpp mcs.cpp mcs.h sense.cpp sense.h ticket.cpp ticket.h tas.cpp tas.h ttas.cpp ttas.h abstractLock.h commons.h -o counter -pthread ${CFLAGS}


clean :
	rm counter
