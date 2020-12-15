#ifndef _ABSTRACTLOCK_H_
#define _ABSTRACTLOCK_H_
#include "commons.h"
class abstractLock{
public:
	virtual void lock()=0;
	virtual void unlock()=0;
};
#endif 
