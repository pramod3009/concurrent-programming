#include "concurrent-bst.h"
#include "sense.h"
#include "concurrent-bst-rw.h"
using namespace std;


/************************Global Variables****************/
int num_threads = 0;
int num_iterations = 0;
string lock_type = "";
string contention = "";
concurrent_bst bst;
concurrent_bst_rw bst_rw;
struct timespec start, time_end;
vector<thread> threads;
barrier_sense *bar;
int key_limit = 100000;
std::atomic<int> num_searches{0}; 
std::atomic<int> num_deletes{0};
std::atomic<int> num_inserts{0};
/*********************************************************/


/****************************************************************
FUNCTION:   process_command_line_arguments
DESCRIPTION: processes the command line arguments and populates
             global variables.
*****************************************************************/
void process_command_line_arguments(int argc, char** argv){
	const char* const short_opts = "t:i:";
    const option long_opts[] = {
            {"name", no_argument, nullptr, 'n'},
            {"contention", required_argument, nullptr, 'c'},
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
        	case 't':
        		num_threads = stoi(optarg);
                bar = new barrier_sense(num_threads);
        		break;
        	case 'i':
        		num_iterations = stoi(optarg);
        		break;
        	case 'l':
        		lock_type +=  std::string(optarg);
        		break;
        	case 'c':
        	    contention += std::string(optarg);
        		break;
        	case '?':
        		cout<<"unknown argument. Plis try again!";
        		exit(0);
        }
    }

    if(num_threads == 0){
        cout<<"num threads should be greater than 0."<<endl;
        exit(1);
    } else if(num_iterations == 0){
        cout<<"num iterations should be greater than 0."<<endl;
        exit(1);
    } else if(lock_type == ""){
        cout<<"please mention a lock type"<<endl;
        exit(1);
    } else if(contention == ""){
        cout<<"please mention contention type : high or low"<<endl; 
        exit(1);
    }

    cout<<"num threads : "<<num_threads<<" num_iterations: "<<num_iterations<<" lock_type "<<lock_type<<" contention type :"<<contention<<endl;
}


/***********************************************************************
FUNCTION:   Worker
DESCRIPTION: All threads call this function. Each set of 
            1/4th num threads workers perform insert/search/range/delete
***********************************************************************/
void worker(int tid){
    if(tid == 1){
        if(contention == "high"){
            for(int i = (num_iterations * (num_threads/4)); i > 4; i -= 2){
                if(lock_type=="fg"){
                    num_inserts++;
                    bst.bst_insert(bst.actual_root, NULL, i, i + 1, tid);
                } else {
                    num_inserts++;
                    bst_rw.bst_insert(bst_rw.actual_root, NULL, i, i + 1, tid);
                }
            }
            for(int i = (num_iterations * (num_threads/4)) - 1; i > 4; i -= 2){
                if(lock_type=="fg"){
                    num_inserts++;
                    bst.bst_insert(bst.actual_root, NULL, i, i + 1, tid);
                } else {
                    num_inserts++;
                    bst_rw.bst_insert(bst_rw.actual_root, NULL, i, i + 1, tid);
                }
            }
            
        }

        clock_gettime(CLOCK_MONOTONIC,&start);

    }
    bar->wait();

    if(tid <= num_threads/4){
        //cout<<"thread: "<<tid<<" performing inserts"<<endl;
        for(int i = 0; i < num_iterations; i++){
            srandom(tid | i * 4);
            int key = random() % key_limit;
            if(lock_type=="fg"){
                num_inserts++;
                bst.bst_insert(bst.actual_root, NULL, key, key + 1, tid);
            } else {
                num_inserts++;
                bst_rw.bst_insert(bst_rw.actual_root, NULL, key, key + 1, tid);
            }
        }
    } else if(tid <= num_threads/2 and tid > num_threads/4){
        //cout<<"thread: "<<tid<<" performing searches"<<endl;
        for(int j = 0; j < num_iterations; j++){
            srandom((tid - num_threads/4) | j * 4);
            int key = random() % key_limit;
            if(lock_type=="fg"){
                num_searches++;
                bst.bst_search(bst.actual_root, NULL, key, tid, key + 1);
            } else {
                num_searches++;
                bst_rw.bst_search(bst_rw.actual_root, NULL, key, tid, key + 1);
            }
        }
    }

    else if(tid <= (3*num_threads/4) and tid > num_threads/2){
        //cout<<"thread: "<<tid<<" performing delete"<<endl;
        for(int k = 0; k < num_iterations; k++){
            srandom((tid - num_threads/2) | k * 4);
            int key = random() % key_limit;
            if(lock_type=="fg") {
                num_deletes++;
                bst.bst_delete(key, bst.actual_root, tid);
            } else {
                num_deletes++;
                bst_rw.bst_delete(key, bst_rw.actual_root, tid);
            }
        }
    }
    else if (tid > (3/4 * num_threads)){
        //cout<<"thread: "<<tid<<" performing range queries"<<endl;
        for(int s = 0 ; s < num_iterations; s++){
            
            int low = 0;
            int high = 0;
            while(low >= high){
                    low = random() % key_limit;
                    high = random() % key_limit;
            }
            if(lock_type=="fg"){
                bst.bst_range_queries(bst.actual_root, NULL, low, high, tid);
            } else {
                bst_rw.bst_range_queries(bst_rw.actual_root, NULL, low, high, tid);
            }
            
        }
    }

    
}

/****************************************************************
FUNCTION:   create_threads_and_call_worker_functions
DESCRIPTION:creather num threads and calls worker fucntion.
*****************************************************************/
void create_threads_and_call_worker_functions(){
    for(int i = 2; i <= num_threads; i++){
        threads.emplace_back(worker, i);
    }
    worker(1);
    for( auto &t : threads){
        t.join();
    }
    clock_gettime(CLOCK_MONOTONIC,&time_end);
    
}

/************************************************************************
FUNCTION:   display_run_stats
DESCRIPTION: Checks if the BST properties hold after parallel
             execution of operations. Peforms validity and correctness
             using atomic variables populated during execution by threads.
***************************************************************************/
void display_run_stats(){
    if(lock_type == "fg"){
        if(bst.bst_correctness(bst.actual_root, INT_MIN, INT_MAX)){// and bst.total_nodes == bst.successful_inserts.load() - bst.successful_deletes.load()){
            cout<<"fine-grained inorder traversal correctness check succeeded"<<endl;
        }
        cout<<"Insert attempts : "<<num_inserts<<" successful inserts: "<<bst.successful_inserts.load()<<endl;
        cout<<"Delete attempts : "<<num_deletes<<" successful deletes: "<<bst.successful_deletes.load()<<endl;
        cout<<"Search attempts : "<<num_searches<<" successful searches: "<<bst.successful_searches.load()<<endl;
        cout<<"Total number of nodes after inorder correctness check : "<<bst.total_nodes<<endl;
        cout<<"Expected number of nodes after run : "<<bst.successful_inserts.load() - bst.successful_deletes.load()<<endl;
    } else {
        if(bst_rw.bst_correctness(bst_rw.actual_root, INT_MIN, INT_MAX)){// and bst.total_nodes == bst.successful_inserts.load() - bst.successful_deletes.load()){
            cout<<"reader-writer lock based inorder traversal correctness check succeeded"<<endl;
        }
        cout<<"Insert attempts : "<<num_inserts<<" successful inserts: "<<bst_rw.successful_inserts.load()<<endl;
        cout<<"Delete attempts : "<<num_deletes<<" successful deletes: "<<bst_rw.successful_deletes.load()<<endl;
        cout<<"Search attempts : "<<num_searches<<" successful searches: "<<bst_rw.successful_searches.load()<<endl;
        cout<<"Total number of nodes after inorder correctness check : "<<bst_rw.total_nodes<<endl;
        cout<<"Expected number of nodes after run : "<<bst_rw.successful_inserts.load() - bst_rw.successful_deletes.load()<<endl;
    }


    unsigned long long elapsed_ns;
	elapsed_ns = (time_end.tv_sec-start.tv_sec)*1000000000 + (time_end.tv_nsec-start.tv_nsec);
	printf("\nElapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);
}


int main(int argc, char** argv){
    process_command_line_arguments(argc, argv);
    create_threads_and_call_worker_functions();
    display_run_stats();
    return 0;
}