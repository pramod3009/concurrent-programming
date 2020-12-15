#ifndef _MCS_H_
#define _MCS_H_
#include "commons.h"

class Node{
    public:
    std::atomic<Node*> next;
    std::atomic<bool> wait;
};

class mcs{
public:
    std::atomic<Node*> tail = {NULL};
    void acquire(Node* mynode);
    void release(Node* mynode);
	
};
#endif