#include "tas.h"

void tas::lock(){
	while(std::atomic_flag_test_and_set(&flag));
}

void tas::unlock(){
	std::atomic_flag_clear(&flag);
}