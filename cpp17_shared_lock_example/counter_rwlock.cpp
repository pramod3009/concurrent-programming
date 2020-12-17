
#include<iostream>
#include<getopt.h>
#include<string>
#include<vector>
#include<thread>
#include<shared_mutex>
#include<stdlib.h>
using namespace std;
/******************Global Variables************************/
int num_threads = 0;
int num_iterations = 0;
std::shared_mutex rw_lock;
int counter = 0;
/*****************Global Variables End*********************/

/****************************************************************
FUNCTION:   process_command_line_arguments
DESCRIPTION: processes the command line arguments and populates
             global variables.
*****************************************************************/
void process_command_line_arguments(int argc, char** argv) {
    const char* const short_opts = "t:i:";
    
    while(1){
    	const auto opt = getopt_long(argc, argv, short_opts, nullptr, nullptr);
        if (-1 == opt)
            break;
        switch(opt){
        	case 't':
        		num_threads = stoi(optarg);
        		break;
        	case 'i':
        		num_iterations = stoi(optarg);
        		break;
        	case '?':
        		cout<<"unknown argument. Please try again!";
        		exit(0);
        }
    }

    if(num_threads == 0){
        cout<<"num threads should be greater than 0."<<endl;
        exit(1);
    } else if(num_iterations == 0){
        cout<<"num iterations should be greater than 0."<<endl;
        exit(1);
    }

}


/****************************************************************
FUNCTION:    reader
DESCRIPTION: Tries to acquire reader lock and reads the shared
             counter. Reader lock can be held by multiple threads
             given there is no writer thread holding an exclusive
             lock.
*****************************************************************/
void reader(int tid){
    for(int iter = 0; iter < num_iterations; iter++){
        rw_lock.lock_shared();
            cout<<"Thread id: "<<tid<<" read counter value : "<<counter<<endl;
        rw_lock.unlock_shared();
    }
}



/****************************************************************
FUNCTION:    reader
DESCRIPTION: Tries to acquire writer lock and increments the shared
             counter. No other thread can enter the critical 
             section if a writer holds exclusive lock.
*****************************************************************/
void writer(int tid){
    for(int iter = 0; iter < num_iterations; iter++){
        rw_lock.lock();
            counter++;
            cout<<"Thread id: "<<tid<<" incremented counter value to : "<<counter<<endl;
        rw_lock.unlock();
    }
}


/****************************************************************
FUNCTION:   create_threads_and_call_workers
DESCRIPTION:Creates num_threads. Half of them write to the counter
            while the rest read it.
*****************************************************************/
void create_threads_and_call_workers(){    
    vector<thread> workers;

    for(int tid = 1; tid <= num_threads; tid++){
        if(tid % 2 == 0){
            workers.emplace_back(reader, tid);
        } else {
            workers.emplace_back(writer, tid);
        }
    }

    for(auto &worker : workers){
           worker.join(); 
    }
}



int main(int argc, char** argv){
    process_command_line_arguments(argc, argv);
    create_threads_and_call_workers();
    return 0;
}