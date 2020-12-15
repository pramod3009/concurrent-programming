#include "abstractLock.h"
#include "tas.h"
#include "ttas.h"
#include "ticket.h"
#include "sense.h"
#include "mcs.h"
using namespace std;
/*******************global parameter start*********************************/
int num_threads = 0;
int num_iterations = 0;
string output_file = "";
string lock_type = "";
string bar_type = "";
struct timespec time_start, time_end;
abstractLock *lk;
vector<std::thread> threads;
int global_count = 0;
barrier_sense *b;
mutex m;
pthread_barrier_t bar;
mcs *mcs_lock;
/*******************global parameter end  *********************************/


/********************counter for all barriers***************************************/

/*****************************************************************
FUNCTION: barrier_counter
DESCRIPTION : If modulus result is current thread ID then thread
			increments global counter and waits for other threads
			to finish their part.
******************************************************************/
void barrier_counter(int my_tid){
	for(int i = 0; i<num_iterations*num_threads; i++){
		if(i%num_threads==my_tid){
			global_count++;
		}
		if(bar_type=="sense"){
			b->wait();
		} else {
			pthread_barrier_wait (&bar);
		}
		
	}
}

/*****************************************************************
FUNCTION: barrier_main
DESCRIPTION : Creates threads and calls barrier_counter
******************************************************************/
void barrier_main(){
for(int i = 1; i < num_threads; i++){
		threads.emplace_back(barrier_counter, i);
	}
	barrier_counter(0);
	for(int i = 1; i < num_threads; i++){
		threads[i-1].join();
	}
	cout<<global_count;
}
/********************sense barrier functions end************************************/

/************Counter for all lock types**********************/
/*****************************************************************
FUNCTION: counter
DESCRIPTION : Coomon function for all locks. Based on value in
              lock_type threads try to acquire appropriate lock.
			  If lock is acquired counter is incremented else thread
			  waits to acquire it in next iteration. 
******************************************************************/
void counter(){
	for(int i = 0; i < num_iterations; i++){
		Node *temp = new Node();
        if(lock_type=="mcs"){
            mcs_lock->acquire(temp);
        } else if(lock_type=="tas" or lock_type=="ttas" or lock_type=="ticket"){
            lk->lock();
        } else{
           m.lock();
        }
		global_count++;
		if(lock_type=="mcs"){
            mcs_lock->release(temp);
        } else if(lock_type=="tas" or lock_type=="ttas" or lock_type=="ticket"){
            delete temp;
            lk->unlock();
        } else{
           m.unlock();
        }
	}
}

/*****************************************************************
FUNCTION: 		create_threads_and_call_counter
DESCRIPTION : 	Creates threads and calls counter function
******************************************************************/
void create_threads_and_call_counter(){
	for(int i = 0; i < num_threads - 1; i++){
		threads.emplace_back(counter);
	}
	counter();
	for(int i = 0; i < num_threads - 1; i++){
		threads[i].join();
	}
	cout<<global_count;
}
/******************************************************************/


/************process_command_line_inputs_start**********************/
void process_command_line_arguments(int argc, char** argv){
	const char* const short_opts = "o:t:i:";
    const option long_opts[] = {
            {"name", no_argument, nullptr, 'n'},
            {"bar", required_argument, nullptr, 'b'},
            {"lock", required_argument, nullptr, 'l'},
            {nullptr, no_argument, nullptr, 0}
    };
    while(1){
    	const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;
        switch(opt){
        	case 'n':
        		cout<<"pramod venkatesh kulkarni - prku8035";
        		exit(0);
        	case 'o':
        		output_file = std::string(optarg);
        		break;
        	case 't':
        		num_threads = stoi(optarg);
        		break;
        	case 'i':
        		num_iterations = stoi(optarg);
        		break;
        	case 'l':
        		lock_type += std::string(optarg);
        		break;
        	case 'b':
        		bar_type += std::string(optarg);
        		break;
        	case '?':
        		cout<<"unknown argument. Plis try again!";
        		exit(0);
        }
    }
    
}

/************process_command_line_inputs_end**********************/
/*******************************************************************/
void write_output_file(){
  
 	
  ofstream  output (output_file);
  if (output.is_open())
  {
    output<<global_count;
  }
  else cout << "Unable to open file";
}
/********************************************************************/
/*********************************************************************
FUNCTION: 		Main
DESCRIPTION : 	Based on the values command line arguments initializes
				appropriate lock/barrier and calls their respective
				counter functions.
**********************************************************************/
int main(int argc, char** argv){
	process_command_line_arguments(argc, argv);
	if(output_file==""){
		cout<<"plis enter output file"<<endl;
		exit(1);
	} else if(bar_type=="" and lock_type==""){
		cout<<"please choose lock or barrier"<<endl;
		exit(1);
	} else if(bar_type!="" and lock_type!=""){
		cout<<"invalid input cannot choose both barrier and lock"<<endl;
		exit(1);
	} else if(num_iterations==0){
		cout<<"enter number of iterations";
		exit(1);
	} else if(num_threads==0){
		cout<<"enter atleast one thread";
	}

	
	
	
	clock_gettime(CLOCK_MONOTONIC,&time_start);
	if(lock_type=="tas"){
			lk = new tas();
			create_threads_and_call_counter();
	} else if (lock_type=="ttas"){
			lk = new ttas();
			create_threads_and_call_counter();
	}else if (lock_type=="ticket"){
		lk = new ticket();
		create_threads_and_call_counter();
	}else if (lock_type=="mcs"){
		mcs_lock = new mcs();
		create_threads_and_call_counter();
	}else if (bar_type=="sense"){
		b = new barrier_sense(num_threads - 1);
		barrier_main();
	} else if(bar_type=="pthread"){
		pthread_barrier_init(&bar, NULL, num_threads);
		barrier_main();
	} else { // default run pthread lock
		create_threads_and_call_counter();
	}
	clock_gettime(CLOCK_MONOTONIC,&time_end);
	
	unsigned long long elapsed_ns;
	elapsed_ns = (time_end.tv_sec-time_start.tv_sec)*1000000000 + (time_end.tv_nsec-time_start.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);
	write_output_file();
}
/******************main_end**********************************/