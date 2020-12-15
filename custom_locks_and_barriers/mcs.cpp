#include "mcs.h"


void mcs::acquire(Node* mynode){
    Node* oldtail = tail.load(std::memory_order_seq_cst);
    mynode->next.store(NULL,std::memory_order_relaxed);
    while(!tail.compare_exchange_strong(oldtail,mynode,std::memory_order_seq_cst)){
        //oldtail = tail.load(std::memory_order_seq_cst);
    }
    if(oldtail!=NULL){
        mynode->wait.store(true,std::memory_order_relaxed);
        oldtail->next.store(mynode,std::memory_order_seq_cst);
        while(mynode->wait.load(std::memory_order_seq_cst)){}
    }
}

void mcs::release(Node* mynode){
    Node* temp(mynode);
    if(tail.compare_exchange_strong(temp, NULL, std::memory_order_seq_cst)){
        
    } else {
        while(mynode->next.load(std::memory_order_seq_cst)==NULL){

        }
        mynode->next.load(std::memory_order_seq_cst)->wait.store(false,std::memory_order_seq_cst);
    }
}