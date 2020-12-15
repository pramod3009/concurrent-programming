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
#include<omp.h>
/*************includes_end************/


/************global_initializations_start****/
struct partition_indices{
	int start;
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
/************global_initializations_end****/


/************multithreaded_quicksort_functions_start*******/


/*****************************************************************
FUNCTION:    merge_helper
DESCRIPTION: Takes start and end indices of two sorted chunks.
			 Merges two chunks and stores into original vector.
******************************************************************/
void merge_helper(int start, int mid, int end){
	int size  = nums.size();
    if(start >= size){
        return;
    }
    if(mid >= size){
        mid = size-1;
    }
    if(end >= size){
        end = size-1;
    }
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
}


/*****************************************************************
FUNCTION:    merge
DESCRIPTION: Merging sub-arrays in a level concurrently 
******************************************************************/
void merge(){
	int block_size = ceil(float(nums.size())/num_threads);
	int levels = 0;
	int size = nums.size();
	while(levels++ <= log(num_threads) + 1){
		#pragma omp for
		for(int start = 0; start< size; start = start + (2 * block_size)){
			merge_helper(start, start + block_size - 1, start + (2 * block_size) -1);
		}
		block_size *= 2;
	}
    
}

/*****************************************************************
FUNCTION:    Partition
DESCRIPTION: Takes start and end indices of a block and partitions
			 such that numbers less than or equal to pivot are to 
			 the left pivot and others to right. 
******************************************************************/
int partition(vector<int>&list, int start, int end){
	int pivot = list[end];
	int i = start - 1;
	for(int j = start ; j < end; j++){
		if(list[j] < pivot){
			i++;
			swap(list[i], list[j]);
		}
	}
	swap(list[i+1],list[end]);
	return i + 1;
}

/*****************************************************************
FUNCTION:    quicksort
DESCRIPTION: Sorts the block into ascending order
******************************************************************/
void quicksort(vector<int>&list, int start, int end){
	if(end >= nums.size()){
		end = nums.size() - 1;
	}
	if(start < end){
		int pivot = partition(list, start, end);
		quicksort(list, start, pivot - 1);
		quicksort(list, pivot + 1, end);
	}
}

/************process_command_line_inputs_start**********************/
void process_command_line_arguments(int argc, char** argv){
	const char* const short_opts = "o:t:";
    const option long_opts[] = {
            {"name", no_argument, nullptr, 'n'},
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
	} 

	
	parse_input_file(nums);
    num_threads = omp_get_max_threads();
	int block_size = ceil(float(nums.size())/num_threads);
	clock_gettime(CLOCK_MONOTONIC,&time_start);
	#pragma omp parallel default(none) shared(nums, block_size)
    {
            #pragma omp for
                for(int start = 0; start < nums.size(); start += block_size){ 
                    quicksort(nums, start, start + block_size - 1);
				}
			if(omp_get_max_threads() > 1){
				merge();
			}
    }
	for(auto num : nums){
		cout<<num<<endl;
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