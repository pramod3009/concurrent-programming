#ifndef _SENSE_H_
#define _SENSE_H_
#include "commons.h"
class barrier_sense{
public:
    barrier_sense(int n) : num_threads(n){}
	void wait();
private:
	std::atomic<int> cnt{0};
    std::atomic<int> sense{0};
    int num_threads;
};
#endif 
