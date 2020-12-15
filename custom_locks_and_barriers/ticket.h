#ifndef _TICKET_H_
#define _TICKET_H_
#include "abstractLock.h"
class ticket : public abstractLock{
public:
	void lock();
	void unlock();
private:
	std::atomic<int> next_num{0};
    std::atomic<int> now_serving{0};
};
#endif 
