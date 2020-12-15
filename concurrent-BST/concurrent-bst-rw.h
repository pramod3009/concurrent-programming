#ifndef _CONCURRENT_BST_RW_H_
#define _CONCURRENT_BST_RW_H_
#include "commons.h"
using namespace std;

enum class rw_lock_type{READER, WRITER};

class treenode_rw{
    public:
    int key;
    int val;
    treenode_rw* parent;
    treenode_rw* left;
    treenode_rw* right;
     
    mutable std::shared_mutex treenode_lock;

    treenode_rw(treenode_rw* parent, int key, int val):key(key),val(val),parent(parent){
        left = right = NULL;
    }

/****************************************************************
FUNCTION:   lock
DESCRIPTION: Based on the enum type it tries to acquire either
             reader lock or writer lock.
*****************************************************************/
    void lock(int tid, rw_lock_type type){
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" waiting lock for :"<<key<<endl;
        #endif
        if(type == rw_lock_type::WRITER){
             treenode_lock.lock();
        } else {
             treenode_lock.lock_shared();
        }
        
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" acquired lock for :"<<key<<endl;
        #endif
    }
/****************************************************************
FUNCTION:   unlock
DESCRIPTION:Based on the enum type it releases either
             reader lock or writer lock.
*****************************************************************/
    void unlock(int tid, rw_lock_type type){
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" unlocked for :"<<key<<endl;
        #endif
        if(type == rw_lock_type::WRITER){
             treenode_lock.unlock();
        } else {
             treenode_lock.unlock_shared();
        }
    }
};

class concurrent_bst_rw{
    public:
    std::atomic<int> successful_searches{0}; 
    std::atomic<int> successful_deletes{0};
    std::atomic<int> successful_inserts{0};
    mutex tree_lock;
    treenode_rw* actual_root = NULL;
    int total_nodes = 0;
    
    void bst_insert(treenode_rw* root, treenode_rw* parent, int key, int val, int tid);
    void bst_search(treenode_rw* root, treenode_rw* parent, int key, int tid, int actual_value);
    int bst_range_queries(treenode_rw* root, treenode_rw* parent, int low, int high, int tid);
    int bst_delete(int key, treenode_rw *root, int tid);

    /*helper functions for delete*/
    treenode_rw* bst_delete_search(int key, treenode_rw* root, int tid);
    treenode_rw* bst_inorder_successor(treenode_rw *node, int tid);
    treenode_rw* bst_inorder_predecessor(treenode_rw *node, int tid);

    /*not thread safe*/
    bool bst_correctness(treenode_rw* root, int lower , int upper);
};

#endif