#include<iostream>
#include "concurrent-bst-rw.h"
using namespace std;

/****************************************************************
FUNCTION:   concurrent_bst::bst_insert
DESCRIPTION: Inserts into the bst a key-value pair. If key is already
             present then updates the value.
*****************************************************************/
void concurrent_bst_rw::bst_insert(treenode_rw* root, treenode_rw* parent, int key, int val, int tid){
    if(parent == NULL){
       
        tree_lock.lock();
       
        if(actual_root == NULL){
            actual_root = new treenode_rw(parent, key , val);
            successful_inserts++;
            tree_lock.unlock();
            return;
        }
        actual_root->lock(tid, rw_lock_type::WRITER);
        
        root = actual_root;
        
        tree_lock.unlock();
    }
    if(key < root->key){
        if(root->left == NULL){
            root->left = new treenode_rw(root, key , val);
            successful_inserts++;
            root->unlock(tid, rw_lock_type::WRITER);
        } else {
            
            (root->left)->lock(tid, rw_lock_type::WRITER);
            
            root->unlock(tid, rw_lock_type::WRITER);
            bst_insert(root->left, root, key, val, tid);
        }
    } else if (key > root->key){
        if(root->right == NULL){
            root->right = new treenode_rw(root, key, val);
            successful_inserts++;
            root->unlock(tid, rw_lock_type::WRITER);
        } else {
            
            (root->right)->lock(tid, rw_lock_type::WRITER);
            
            root->unlock(tid, rw_lock_type::WRITER);
            bst_insert(root->right, root, key, val, tid);
        }
    } else {
        root->val = val;
         
        root->unlock(tid, rw_lock_type::WRITER);
    }
}

/****************************************************************
FUNCTION:   concurrent_bst::bst_search
DESCRIPTION: Seaches for a key-value in the best. If the key is present
             then value in tree is matched with supplied value to determine
             if search succeeded.
*****************************************************************/
void concurrent_bst_rw::bst_search(treenode_rw* root, treenode_rw* parent, int key, int tid, int actual_value){
    if(parent == NULL){
       
        tree_lock.lock();
        
        if(actual_root == NULL){
           
            tree_lock.unlock();
            
            return;
        }
       
        actual_root->lock(tid, rw_lock_type::READER);
        
        root = actual_root;
        
        tree_lock.unlock();
    }

    if(key < root->key){
        if(root->left == NULL){
            
            root->unlock(tid, rw_lock_type::READER);
            return;
        }
      
        (root->left)->lock(tid, rw_lock_type::READER);
      
        root->unlock(tid, rw_lock_type::READER);
        bst_search(root->left, root, key, tid, actual_value);
    } else if(key > root->key){
        if(root->right == NULL){
            
            root->unlock(tid, rw_lock_type::READER);
            return;
        }
    
        (root->right)->lock(tid, rw_lock_type::READER);
        
        root->unlock(tid, rw_lock_type::READER);
        bst_search(root->right, root, key, tid, actual_value);
    } else {
        
        #ifdef DEBUG
            cout<<"Key found and data : "<<root->val<<" tid :"<<tid<<endl;
        #endif
        
        if(root->val == actual_value){
            successful_searches++;
        } 
        
        root->unlock(tid, rw_lock_type::READER);
    }

}


/****************************************************************
FUNCTION:   concurrent_bst::bst_range_queries
DESCRIPTION:Traverses the tree in level order fashion and if key
            in the node falls withing query range then prints the
            value contained.
*****************************************************************/
int concurrent_bst_rw::bst_range_queries(treenode_rw* root, treenode_rw* parent, int low, int high, int tid){
    static int counter = 0;
    queue<treenode_rw*> q;
    if(parent == NULL){
      
        tree_lock.lock();
     
        if(actual_root == NULL){
            #ifdef DEBUG
                cout <<"tree has not been constructed yet :"<<endl;
            #endif
            tree_lock.unlock();
            return counter;
        }
        actual_root->lock(tid, rw_lock_type::READER);
        
        q.push(actual_root);
        root = actual_root;
        
        tree_lock.unlock();
    }

    vector<pair<int, int>> v;
    while(!q.empty()){
        treenode_rw *temp = q.front();
        q.pop();
        if(temp and temp->key >= low and temp->key <= high){
            v.push_back({temp->key, temp->val});
        }

        if(low < root->key){
            if(temp and temp->left != NULL){
                temp->left->lock(tid, rw_lock_type::READER);
                q.push(temp->left);
            }
        }

        if(high > root->key){
            if(temp and temp->right != NULL){
                temp->right->lock(tid, rw_lock_type::READER);
                q.push(temp->right);
            }
        }
        temp->unlock(tid, rw_lock_type::READER);
    }
    sort(v.begin(), v.end());
    
    #ifdef DEBUG
        for(auto key_value : v){
            cout<<"key: "<<key_value.first<<" value: "<<key_value.second<<" thread id : "<<tid<<endl;
        }
    #endif
}


/****************************************************************
FUNCTION:   concurrent_bst::bst_inorder_successor
DESCRIPTION:Finds the inorder successor of the node to be deleted.
*****************************************************************/
treenode_rw* concurrent_bst_rw::bst_inorder_successor(treenode_rw *node, int tid){
    treenode_rw* parent = node->right;
    treenode_rw* succesor = parent->left;

    succesor->lock(tid, rw_lock_type::WRITER);
    while(succesor->left != NULL){
        succesor = succesor->left;
        parent->unlock(tid, rw_lock_type::WRITER);
        succesor->lock(tid, rw_lock_type::WRITER);
        parent = succesor->parent;
    }
    return succesor;
}


/****************************************************************
FUNCTION:   concurrent_bst::bst_inorder_predecessor
DESCRIPTION:Finds the inorder predecessor of the node to be deleted.
*****************************************************************/
treenode_rw* concurrent_bst_rw::bst_inorder_predecessor(treenode_rw *node, int tid){
    treenode_rw* parent = node->left;
    treenode_rw* predecessor = parent->right;
    predecessor->lock(tid, rw_lock_type::WRITER);
    while(predecessor->right != NULL){
        predecessor = predecessor->right;
        parent->unlock(tid, rw_lock_type::WRITER);
        predecessor->lock(tid, rw_lock_type::WRITER);
        parent = predecessor->parent;
    }
    return predecessor;
}

/****************************************************************
FUNCTION:   concurrent_bst::bst_delete_search
DESCRIPTION: Returns the node to be delted if the key exists in tree
*****************************************************************/
treenode_rw* concurrent_bst_rw::bst_delete_search(int key, treenode_rw* root, int tid){
    if(root->key == key){
        return root;
    } else if (key < root->key){
        if(root->left == NULL){
            root->unlock(tid, rw_lock_type::WRITER);
            return NULL;
        }else {
            (root->left)->lock(tid, rw_lock_type::WRITER);
            if((root->left)->key == key){
                return root->left;
            } else {
                root->unlock(tid, rw_lock_type::WRITER);
                return bst_delete_search(key, root->left, tid);
            }
        }
    } else {
        if(root->right == NULL){
            root->unlock(tid, rw_lock_type::WRITER);
            return NULL;
        } else {
            (root->right)->lock(tid, rw_lock_type::WRITER);
            if((root->right)->key == key){
                return root->right;
            } else {
                root->unlock(tid, rw_lock_type::WRITER);
                return bst_delete_search(key, root->right, tid);
            }
        }
    }
    return NULL;
}

/****************************************************************
FUNCTION:   concurrent_bst::bst_delete
DESCRIPTION: Deletes the key from tree and returns value in it.
*****************************************************************/
int concurrent_bst_rw::bst_delete(int key, treenode_rw *root, int tid){
    treenode_rw *node_tobe_deleted, *parent;
    treenode_rw *successor, *successor_parent;
    treenode_rw *predecessor, *predecessor_parent;

    tree_lock.lock();
    if(actual_root == NULL){
        tree_lock.unlock();
        return -1;
    }
    root = actual_root;
    root->lock(tid, rw_lock_type::WRITER);
    

    if(key == root->key and !root->left and !root->right){
        int val = root->val;
        delete root;
        actual_root = NULL;
        successful_deletes++;
        tree_lock.unlock();
        return val;
    }

    node_tobe_deleted = bst_delete_search(key, root, tid);

    if(node_tobe_deleted == NULL){
        
        #ifdef DEBUG
            cout<<"key "<<key<<" not found in the tree "<<tid<<endl;
        #endif
        
        tree_lock.unlock();
        return -1;
    }

    parent = node_tobe_deleted->parent;

    if(node_tobe_deleted->left == NULL and node_tobe_deleted->right == NULL and parent == NULL){
        int val = root->val;
        delete root;
        actual_root = NULL;
        successful_deletes++;
        tree_lock.unlock();
        return val;
    }

    tree_lock.unlock();

    if(node_tobe_deleted->left == NULL and node_tobe_deleted->right == NULL) {

        if(node_tobe_deleted->key < parent->key){
            int val = node_tobe_deleted->val;
            delete node_tobe_deleted;
            parent->left = NULL;
            successful_deletes++;
            parent->unlock(tid, rw_lock_type::WRITER);
            return val;

        } else {

            int val = node_tobe_deleted->val;
            delete node_tobe_deleted;
            parent->right = NULL;
            successful_deletes++;
            parent->unlock(tid, rw_lock_type::WRITER);
            return val;

        }
    }

    if(parent != NULL){

        parent->unlock(tid, rw_lock_type::WRITER);

    }

    if(node_tobe_deleted->right != NULL){

        (node_tobe_deleted->right)->lock(tid, rw_lock_type::WRITER);

        if(node_tobe_deleted->right->left == NULL){

            successor = node_tobe_deleted->right;
            int val = node_tobe_deleted->val;

            if(successor->right != NULL){

                (successor->right)->lock(tid, rw_lock_type::WRITER);
                node_tobe_deleted->val = successor->val;
                node_tobe_deleted->key = successor->key;
                node_tobe_deleted->right = successor->right;
                successor->right->parent = node_tobe_deleted;
                (successor->right)->unlock(tid, rw_lock_type::WRITER);

            } else {

                node_tobe_deleted->val = successor->val;
                node_tobe_deleted->key = successor->key;
                node_tobe_deleted->right = NULL;

            }
            delete successor;
            successful_deletes++;
            node_tobe_deleted->unlock(tid, rw_lock_type::WRITER);
            return val;
        }

        successor = bst_inorder_successor(node_tobe_deleted, tid);
        successor_parent = successor->parent;
        int val = node_tobe_deleted->val;

        if(successor->right != NULL){

            (successor->right)->lock(tid, rw_lock_type::WRITER);
            successor_parent->left = successor->right;
			successor->right->parent = successor_parent;
            node_tobe_deleted->val = successor->val;
            node_tobe_deleted->key = successor->key;
            (successor->right)->unlock(tid, rw_lock_type::WRITER);

        } else {

            node_tobe_deleted->val = successor->val;
            node_tobe_deleted->key = successor->key;
            successor_parent->left = NULL;

        }
        
        delete successor;
        successful_deletes++;
        successor_parent->unlock(tid, rw_lock_type::WRITER);
        node_tobe_deleted->unlock(tid, rw_lock_type::WRITER);
        return val;
    }

    if(node_tobe_deleted->left != NULL){

        (node_tobe_deleted->left)->lock(tid, rw_lock_type::WRITER);

        if(node_tobe_deleted->left->right == NULL){

            predecessor = node_tobe_deleted->left;
            int val = node_tobe_deleted->val;
            if(predecessor->left != NULL){

                (predecessor->left)->lock(tid, rw_lock_type::WRITER);
                node_tobe_deleted->val = predecessor->val;
                node_tobe_deleted->key = predecessor->key;
                node_tobe_deleted->left = predecessor->left;
                predecessor->left->parent = node_tobe_deleted;
                (predecessor->left)->unlock(tid, rw_lock_type::WRITER);

            } else {

                node_tobe_deleted->val = predecessor->val;
                node_tobe_deleted->key = predecessor->key;
                node_tobe_deleted->left = NULL;

            }
            
            delete predecessor;
            successful_deletes++;
            node_tobe_deleted->unlock(tid, rw_lock_type::WRITER);
            return val;
        }

        predecessor = bst_inorder_predecessor(node_tobe_deleted, tid);
        predecessor_parent = predecessor->parent;

        int val = node_tobe_deleted->val;
        
        if(predecessor->left != NULL){

            (predecessor->left)->lock(tid, rw_lock_type::WRITER);
            predecessor_parent->right = predecessor->left;
			predecessor->left->parent = predecessor_parent;
            node_tobe_deleted->val = predecessor->val;
            node_tobe_deleted->key = predecessor->key;
            (predecessor->left)->unlock(tid, rw_lock_type::WRITER);

        } else {

            node_tobe_deleted->val = predecessor->val;
            node_tobe_deleted->key = predecessor->key;
            predecessor_parent->right = NULL;

        }
        
        delete predecessor;
        successful_deletes++;
        predecessor_parent->unlock(tid, rw_lock_type::WRITER);
        node_tobe_deleted->unlock(tid, rw_lock_type::WRITER);
        return val;
    }
    return INT_MIN;
}


/****************************************************************
FUNCTION:   concurrent_bst::bst_correctness
DESCRIPTION:Checks if BST properties still hold after concurrent
            operations.
*****************************************************************/
bool concurrent_bst_rw::bst_correctness(treenode_rw* root,int lower, int upper){
    if(root==NULL){
            return true;
    }
    int val = root->key;
    total_nodes++;
    if(lower!=INT_MIN and val<=lower){
        return false;
    }
    if(upper!=INT_MIN and val>=upper){
        return false;
    }
    if(bst_correctness(root->left, lower, val) and bst_correctness(root->right, val, upper)){
        
        return true;
    }
    return false;
}

