#include "ticket.h"

void ticket::lock(){
    int my_num = next_num++;
    while(now_serving.load(std::memory_order_seq_cst) != my_num);
}

void ticket::unlock(){
    now_serving++;
}