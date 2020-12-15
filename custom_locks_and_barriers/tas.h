#ifndef _TAS_H_
#define _TAS_H_
#include "abstractLock.h"
class tas : public abstractLock{
public:
	void lock();
	void unlock();
private:
	std::atomic_flag flag =  ATOMIC_FLAG_INIT;
};
#endif 
