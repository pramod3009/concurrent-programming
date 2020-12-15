/*************includes_start************/
using namespace std;
#include<iostream>
#include <getopt.h>
#include <fstream>
#include <iterator>
#include <string>
#include<vector>
#include<pthread.h>
#include<mutex>
#include<queue>
#include<unistd.h>
#include<queue>
#include<stdlib.h>
#include<algorithm>
#include<climits>
#include<math.h>
/*************includes_end************/


/************global_initializations_start****/
struct partition_indices{
	int start;
	int mid;
	int end;
	int thread_id;
};

string output_file = "";
string input_file = "";
string algo = "";
int num_threads = 0;
int num_max = INT_MIN;
int num_buckets;
vector<mutex> locks;
vector<priority_queue<int,vector<int>,greater<int>>> q;
vector<int> nums;
struct partition_indices *data_d;
struct timespec time_start, time_end;
pthread_t *threads;
/************global_initializations_end****/


/************multithreaded_quicksort_functions_start*******/


/*****************************************************************
FUNCTION:    merge_helper
DESCRIPTION: Takes start and end indices of two sorted chunks.
			 Merges two chunks and stores into original vector.
******************************************************************/
void* merge_helper(void *input){
	struct partition_indices *data = (struct partition_indices*)input;
	int start = data->start;
    int mid = data->mid;
    int end = data->end;

	vector<int> list_1;
	vector<int> list_2;
	for(int i = start; i <=mid; i++){
		list_1.push_back(nums[i]);
	}
	for(int i = mid + 1; i<=end; i++){
		list_2.push_back(nums[i]);
	}
	int i = 0;
	int j = 0;
	int k = start;

	while(i < list_1.size() and j < list_2.size()){
		if(list_1[i]<=list_2[j]){
			nums[k] = list_1[i];
			i++;
		} else {
			nums[k] = list_2[j];
			j++;
		}
		k++;
	}

	while(i < list_1.size()) {
		nums[k] = list_1[i];
		i++;
		k++;
	}

	while(j < list_2.size()) {
		nums[k] = list_2[j];
		j++;
		k++;
	}

	return NULL;
}


/*****************************************************************
FUNCTION:    merge
DESCRIPTION: Iterates over block indices and calls merge on 
			 neighboring blocks.
******************************************************************/
// void merge(){
// 	int prev_end = data_d[0].end;
// 	for(int i = 1; i < num_threads; i++){
// 		merge_helper(0, prev_end, data_d[i].start, data_d[i].end);
// 		prev_end = data_d[i].end;
// 	}
// }

void merge(){
	int start = 0;
	int mid  = 0;
	int end = 0;
	int tid = 1;
	int prev_tid = 1;
	int times = 0;
	int block_size = ceil(float(nums.size())/num_threads);
	vector<partition_indices*> free_list;

	while(times++ <= log(num_threads) +1 ){
		start = 0;
        end = 0;
        mid = 0;
		while(end < nums.size()){
			mid  = start + block_size - 1;
			end =  mid + block_size;
			if(mid >= nums.size()){
				mid = nums.size() - 1;
			}
			if(end >= nums.size()){
				end = nums.size() - 1;
			}

			struct partition_indices *d = (struct partition_indices *) malloc(sizeof(struct partition_indices));
			free_list.push_back(d);
			d->start = start;
            d->end = end;
            d->mid = mid;

			int ret = pthread_create(&(threads[tid]), NULL, &merge_helper, (void *)(d));
            if(ret){
                printf("ERROR; pthread_create: %d\n", ret);
                exit(-1);
            }
            tid += 1;

            if(end >= nums.size() - 1){
                break;
            }
            start = end + 1;

		}
		for(int i = prev_tid; i < tid; i++){
            int ret = pthread_join(threads[i],NULL);
            if(ret){
                printf("ERROR; pthread_join: %d\n", ret);
                exit(-1);
            }
        }
        prev_tid = tid;
        block_size *= 2;
	}

	for(auto data_pointer : free_list){
		free(data_pointer);
	}
}
/*****************************************************************
FUNCTION:    Partition
DESCRIPTION: Takes start and end indices of a block and partitions
			 such that numbers less than or equal to pivot are to 
			 the left pivot and others to right. 
******************************************************************/
int partition(int start, int end){
	int pivot = nums[end];
	int i = start - 1;
	for(int j = start ; j < end; j++){
		if(nums[j] < pivot){
			i++;
			swap(nums[i], nums[j]);
		}
	}
	swap(nums[i+1],nums[end]);
	return i + 1;
}


/*****************************************************************
FUNCTION:    quicksort
DESCRIPTION: Sorts the block into ascending order
******************************************************************/
void *quicksort(void *index){
	struct partition_indices *d = (struct partition_indices*)index;
	int start = d->start;
	int end = d->end;
	if(start < end){
		int pivot = partition(start, end);
		struct partition_indices temp[2];
		temp[0].start = start;
		temp[0].end = pivot - 1;
		temp[1].start = pivot + 1;
		temp[1].end = end;
		quicksort((void*)(&temp[0]));
		quicksort((void*)(&temp[1]));
	}
}


/*****************************************************************
FUNCTION:    helper_threaded_quicksort
DESCRIPTION: Creates "num_threads" and assigns each thread a 
			 sub-array of size "block_size" and calls quicksort for
			 each thread.
******************************************************************/
void helper_threaded_quicksort(){
	threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
	for(size_t i=1; i<num_threads; i++){
		printf("creating thread %d\n",i);
		int ret = pthread_create(&threads[i], NULL, &quicksort, (void*)(&data_d[i]));
		if(ret){
			printf("ERROR; pthread_create: %d\n", ret);
			exit(-1);
		}
	}

	quicksort((void*)(&data_d[0])); // master also calls thread_main
	
	// join threads
	for(size_t i=1; i<num_threads; i++){
		int ret = pthread_join(threads[i],NULL);
		if(ret){
			printf("ERROR; pthread_join: %d\n", ret);
			exit(-1);
		}
		printf("joined thread %d\n",i);
	}
	merge();

}
/************multithreaded_quicksort_functions_end*******/


/************multithreaded_bucketsort_functions_start*******/


/*****************************************************************
FUNCTION:    bucketsort
DESCRIPTION: Gets bucket(prioroty_queue) number based on number. 
			 If lock for that bucket is available then populate 
			 it with the number.  
******************************************************************/
void* bucketsort(void *index){
	struct partition_indices *d = (struct partition_indices*)index;
	int start = d->start;
	int end = d->end; 
	for(int i = start; i<=end; i++ ){
		int bucket = floor(num_buckets*nums[i]/num_max);
		if(bucket>=num_buckets){
			bucket = num_buckets - 1;
		}
		locks[bucket].lock();
		q[bucket].push(nums[i]);
		locks[bucket].unlock();
	}
	return NULL;
}


/*****************************************************************
FUNCTION:    helper_threaded_bucketsort
DESCRIPTION: Initializes locks.Creates "num_threads" and assigns 
			 each thread a sub-array of size "block_size" and 
			 calls bucketsort for each thread. 
******************************************************************/
void helper_threaded_bucketsort(){
	threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
	
	num_buckets = ceil(float(log(nums.size())));
	vector<mutex> temp_locks(num_buckets);
	locks.swap(temp_locks);
	q.resize(num_buckets);

	for(size_t i=1; i<num_threads; i++){
		printf("creating thread %d\n",i);
		int ret = pthread_create(&threads[i], NULL, &bucketsort, (void*)(&data_d[i]));
		if(ret){
			printf("ERROR; pthread_create: %d\n", ret);
			exit(-1);
		}
	}

	bucketsort((void*)(&data_d[0])); // master also calls thread_main
	
	// join threads
	for(size_t i=1; i<num_threads; i++){
		int ret = pthread_join(threads[i],NULL);
		if(ret){
			printf("ERROR; pthread_join: %d\n", ret);
			exit(-1);
		}
		printf("joined thread %d\n",i);
	}
	nums.clear();
	for(int i = 0; i < num_buckets; i++){
		while(!q[i].empty()){
			nums.push_back(q[i].top());
			q[i].pop();
		}
	}
	
}
/************multithreaded_bucketsort_functions_end*******/


void cleanup(){
	free(threads);
	free(data_d); 
}
/************populate_block_indices***********************/
void create_nums_partitions_and_sort(int block_size){
	data_d = (struct partition_indices*)malloc(num_threads*sizeof(struct partition_indices));
	int current_block_size = block_size;
	data_d[0].start = 0;
	data_d[0].end = block_size - 1;
	data_d[0].thread_id = 0;
	for(int i = 1; i < num_threads; i++){
		int start = current_block_size;
		int end;
		if(current_block_size + block_size >= nums.size()){
			end = nums.size() - 1;
		} else {
			end = current_block_size + block_size - 1;
		}
		data_d[i].start = start;
		data_d[i].end = end;
		data_d[i].thread_id = i;
		current_block_size+=block_size;
	} 

}
/************populate_block_indices_end***********************/


/************process_command_line_inputs_start**********************/
void process_command_line_arguments(int argc, char** argv){
	const char* const short_opts = "o:t:";
    const option long_opts[] = {
            {"name", no_argument, nullptr, 'n'},
            {"alg", required_argument, nullptr, 'a'},
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
        	case 'a':
        		algo = std::string(optarg);
        		break;
        	case '?':
        		cout<<"unknown argument. Plis try again!";
        		exit(0);
        }
    }
    if(optind < argc){
    	input_file += std::string(argv[optind]); 
    }

}
/************process_command_line_inputs_end**********************/


/************file_handling_functions_start************************/


void parse_input_file(vector<int> &nums){
  ifstream input (input_file);
  string line;
  if (input.is_open())
  {
    while ( getline (input,line) )
    {
      nums.push_back(stoi(line));
      num_max = std::max(num_max, nums.back());
    }
    input.close();
  }
  else cout << "Unable to open file";
}


void write_output_file(vector<int> &nums){
  ofstream  output (output_file);
  if (output.is_open())
  {
    ostream_iterator<int> itr(output, "\n");
    copy(nums.begin(),nums.end(),itr);
  }
  else cout << "Unable to open file";
}
/************file_handling_functions_end************************/


/******************main_start**********************************/
int main(int argc, char** argv){
	process_command_line_arguments(argc, argv);
	if(input_file==""){
		cout<<"plis enter input file"<<endl;
		exit(1);
	} else if(output_file==""){
		cout<<"plisenter output file"<<endl;
	} else if(algo==""){
		cout<<"plis select algorithm"<<endl;
	}

	
	parse_input_file(nums);
	cout<<"end of parse input"<<endl;

	int block_size = ceil(float(nums.size())/num_threads);
	create_nums_partitions_and_sort(block_size);
	cleanup();
	clock_gettime(CLOCK_MONOTONIC,&time_start);
	if(algo=="fjmerge" or algo=="fjquick"){
		helper_threaded_quicksort();
	}else {
		helper_threaded_bucketsort();
	}
	clock_gettime(CLOCK_MONOTONIC,&time_end);
	
	unsigned long long elapsed_ns;
	elapsed_ns = (time_end.tv_sec-time_start.tv_sec)*1000000000 + (time_end.tv_nsec-time_start.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);
	write_output_file(nums);
}
/******************main_end**********************************/
