#ifndef _CONCURRENT_BST_H_
#define _CONCURRENT_BST_H_
#include "commons.h"
using namespace std;
class treenode{
    public:
    int key;
    int val;
    treenode* parent;
    treenode* left;
    treenode* right;
    mutex treenode_lock;
    treenode(treenode* parent, int key, int val):key(key),val(val),parent(parent){
        left = right = NULL;
    }
/****************************************************************
FUNCTION:   lock()
DESCRIPTION: Tries to acquire lock on the tree node
*****************************************************************/
    void lock(int tid){
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" waiting lock for :"<<key<<endl;
        #endif
        
        treenode_lock.lock();
        
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" acquired lock for :"<<key<<endl;
        #endif
    }

/****************************************************************
FUNCTION:   unlock
DESCRIPTION: Releases the lock on tree node 
*****************************************************************/
    void unlock(int tid){
        #ifdef DEBUG
            cout<<"thread :"<<tid<<" unlocked for :"<<key<<endl;
        #endif
        treenode_lock.unlock();
    }
};

class concurrent_bst{
    public:
    std::atomic<int> successful_searches{0}; 
    std::atomic<int> successful_deletes{0};
    std::atomic<int> successful_inserts{0};
    
    mutex tree_lock;
    treenode* actual_root = NULL;
    
    int total_nodes = 0;
    
    void bst_insert(treenode* root, treenode* parent, int key, int val, int tid);
    void bst_search(treenode* root, treenode* parent, int key, int tid, int actual_value);
    int bst_range_queries(treenode* root, treenode* parent, int low, int high, int tid);
    int bst_delete(int key, treenode *root, int tid);

    /*helper functions for delete*/
    treenode* bst_delete_search(int key, treenode* root, int tid);
    treenode* bst_inorder_successor(treenode *node, int tid);
    treenode* bst_inorder_predecessor(treenode *node, int tid);

    /*not thread safe*/
    bool bst_correctness(treenode* root, int lower , int upper);
};

#endif