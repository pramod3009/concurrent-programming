#include "sense.h"
void barrier_sense::wait(){
        thread_local bool my_sense = 0;
        if(my_sense==0){
            my_sense = 1;
        } else {
            my_sense = 0;
        }
        int cnt_cpy = cnt++;
        if(cnt_cpy == num_threads - 1){
            cnt.store(0, std::memory_order_relaxed);
            sense.store(my_sense, std::memory_order_seq_cst);
        } else {
            while(sense.load(std::memory_order_seq_cst)!=my_sense);
        }
    }