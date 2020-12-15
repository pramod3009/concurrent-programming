#include "ttas.h"

void ttas::lock(){
	while(flag.load(std::memory_order_relaxed)==true || flag.exchange(true, std::memory_order_acquire));
}

void ttas::unlock(){
	flag.store(false, std::memory_order_release);
}