#ifndef _TTAS_H_
#define _TTAS_H_
#include "abstractLock.h"
class ttas : public abstractLock{
public:
	void lock();
	void unlock();
private:
	std::atomic<bool> flag{false};
};
#endif 
