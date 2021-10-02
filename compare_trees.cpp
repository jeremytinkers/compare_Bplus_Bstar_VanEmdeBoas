#include <iostream>
#include <chrono>
#include <string.h>
#include <sstream>
#include <fstream>
#include <time.h> 
#include <bits/stdc++.h>  
#define NEXT_CELL 0
#define NEXT_ROW 1
#define MAX 100 // Threshold on maximum order
#define NIL -1
#define N 4  //order of B* tree

using namespace std;
const char outputPath[] = "output.csv";
ofstream outputFile;

/********** headers of csv file ************/
char csvcolumns[] =
"graph_size,b*_insert_time,b*_insert_node_count,b+_insert_time,b+_insert_node_count,veb_insert_time,veb_insert_node_count,b*_del_time,b*_del_node_count,b+_del_time,b+_del_node_count,veb_del_time,veb_del_node_count,b*_search_time,b*_search_node_count,b+_search_time,b+_search_node_count,veb_search_time,veb_search_node_count,b*_min_time,b*_min_node_count,b+_min_time,b+_min_node_count,veb_min_time,veb_min_node_count,b*_exmin_time,b*_exmin_node_count,b+_exmin_time,b+_exmin_node_count,veb_exmin_time,veb_exmin_node_count";

/**** debug variables **/
int loopControl=0;

/*********************classes and structures used***********************/

/************************** B-STAR TREE ********************************/
class b_star_node{

    friend class b_star_tree;
    int keys[2*((2*N-2)/3)+1];
    b_star_node *parent;
    b_star_node* child[2*((2*N-2)/3)+2];
    bool isLeaf;
    int num_keys_filled;

    public:
    b_star_node(){
        isLeaf = true;
        num_keys_filled = 0;
        parent = NULL;
        int n = 2*((2*N-2)/3)+1;
        for(int i=0; i<n; i++){
            keys[i] = 0;
            child[i] = NULL;
        }
        child[n] = NULL;
    }
};

class b_star_tree{

    b_star_node *root;
    const int max_keys;
    const int min_keys;
    const int max_keys_root;
    const int min_keys_root;

    b_star_node* find_node_insert(b_star_node*, int, int &);
    bool isOverloaded(b_star_node *);
    void split_root(int &);
    bool spill_to_siblings(b_star_node*, int, int &);
    void spill_to_left(b_star_node*, int, int &);
    void spill_to_right(b_star_node*, int, int &);
    void split_2_3(b_star_node*, int &);
    int get_child_index(b_star_node*);
    pair<b_star_node*, int> find_node_key(int, int &);
    bool borrow_from_siblings(b_star_node*, int &);
    void merge_2_3(b_star_node*, int &);
    void merge_root(int &);
    b_star_node* get_greater_of_minor(b_star_node*, int &);
    void handle_underload(b_star_node*, int &);
    bool isUnderloaded(b_star_node*);

    public:
    b_star_tree();
    void insert(int, int &);
    bool search(int, int &);
    void remove(int, int &);
    b_star_node* find_min(int &);
    void extract_min(int &);
    void construct_tree(int*, int);
    void print_tree();
    void fn();
    void empty(b_star_node*);
    ~b_star_tree();
};

void b_star_tree::print_tree(){
  if(!root)
      return;
    queue<b_star_node*> q;
    q.push(root);
    while(!q.empty()){
        cout<<"***";
        b_star_node *t = q.front();
        q.pop();
        if(t){
            for(int i=0; i<t->num_keys_filled; i++){
                cout<<t->keys[i]<<",";
                if(t->child[i])
                    q.push(t->child[i]);
            }
            if(t->child[t->num_keys_filled])
                q.push(t->child[t->num_keys_filled]);
        }
        cout<<"   ";
    }
    cout<<"\n";
}

b_star_tree::b_star_tree(): max_keys(N-1), min_keys(ceil((2*N-1)/3.0)-1), 
                             max_keys_root(2*floor((2*N-2)/3.0)), min_keys_root(1){

    root = new b_star_node;        
}

//Find the node at which data is to be inserted
b_star_node* b_star_tree::find_node_insert(b_star_node *root, int val, int &nodes){

    
    nodes++;
    
    if(root->isLeaf)  //leaf node - can't search any further
        return root;
    
    else{
        int i=0;
        while(i < root->num_keys_filled && root->keys[i] < val)
            i++;
        return find_node_insert(root->child[i], val, nodes);
    }
}

void b_star_tree::insert(int val, int &nodes){

    b_star_node *insert_node = find_node_insert(root, val, nodes);

    //Find position to insert val in this node by shifting keys greater than val
    int i;
    for(i=insert_node->num_keys_filled-1; i>=0; i--){
        if(insert_node->keys[i] > val)
            insert_node->keys[i+1] = insert_node->keys[i];
        else
            break;
    }

    insert_node->keys[i+1] = val;
    insert_node->num_keys_filled++;
    nodes++;
    
    b_star_node* cur_node = insert_node;

    while(cur_node && isOverloaded(cur_node)){
        
        //check for key overflow
        if(cur_node == root && cur_node->num_keys_filled > max_keys_root)        
            split_root(nodes);
        else if(cur_node != root && cur_node->num_keys_filled > max_keys){

          //check siblings - spill operation
          bool space_in_siblings = spill_to_siblings(cur_node, val, nodes);

          //2-3 split
          if(!space_in_siblings)
              split_2_3(cur_node, nodes);        
        }

        cur_node = cur_node->parent;
    }
}

bool b_star_tree::isOverloaded(b_star_node *node){

    if(node == NULL)
        return false;
    return (node == root && node->num_keys_filled > max_keys_root) || 
             (node != root && node->num_keys_filled > max_keys);
}

void b_star_tree::split_root(int &nodes){
    
    root->isLeaf = false;

    b_star_node *child1 = new b_star_node;
    b_star_node *child2 = new b_star_node;
    int i=0, j;

    //Left child
    for(j=0; j<max_keys_root/2; i++, j++){
        child1->keys[j] = root->keys[i];  //move key
        child1->child[j] = root->child[i];  //move subtree
        if(child1->child[j])
            child1->child[j]->parent = child1;
        root->keys[i] = 0;
        root->child[i] = NULL;
        root->num_keys_filled--;
        child1->num_keys_filled++;
    }
    child1->child[j] = root->child[i];
    if(child1->child[j])
        child1->child[j]->parent = child1;
    child1->parent = root;

    int root_key = root->keys[i++];
    root->num_keys_filled--;

    //Right child
    for(j=0; root->num_keys_filled > 0; i++, j++){
        child2->keys[j] = root->keys[i];  //move key
        child2->child[j] = root->child[i];  //move subtree
        if(child2->child[j])
            child2->child[j]->parent = child2;
        root->keys[i] = 0;
        root->child[i] = NULL;
        root->num_keys_filled--;
        child2->num_keys_filled++;
    }
    child2->child[j] = root->child[i];
    if(child2->child[j])
        child2->child[j]->parent = child2;
    child2->parent = root;

    //new root
    root->keys[0] = root_key;
    root->num_keys_filled = 1;
    root->child[0] = child1;
    root->child[1] = child2;
    if(child1->child[0])
       child1->isLeaf = false;
    if(child2->child[0])
       child2->isLeaf = false;
    nodes++; //Visited only the root
    
}

bool b_star_tree::spill_to_siblings(b_star_node *node, int inserted_val, int &nodes){

    int child_idx = get_child_index(node);

    if(child_idx != 0 && node->parent->child[child_idx-1]->num_keys_filled < max_keys){
        spill_to_left(node, child_idx, nodes); //right to left
        return true;
    }

    else if(child_idx != node->parent->num_keys_filled && 
            node->parent->child[child_idx+1]->num_keys_filled < max_keys){
        spill_to_right(node, child_idx, nodes);
        return true;
    }

    return false;
}

void b_star_tree::spill_to_left(b_star_node* node, int idx, int &nodes){

    b_star_node *left_sibling = node->parent->child[idx-1];

    left_sibling->keys[left_sibling->num_keys_filled] = node->parent->keys[idx-1];
    left_sibling->num_keys_filled++; 
    left_sibling->child[left_sibling->num_keys_filled] = node->child[0];
    if(node->child[0])
        node->child[0]->parent = left_sibling;
    node->child[0] = NULL;
    
    node->parent->keys[idx-1] = node->keys[0];

    for(int i=0; i<node->num_keys_filled-1; i++){
        node->keys[i] = node->keys[i+1];
        node->child[i] = node->child[i+1];
    }
    node->child[node->num_keys_filled-1] = node->child[node->num_keys_filled];

    node->num_keys_filled--;
    nodes += 3;  //visited the node, its left sibling and parent

}

void b_star_tree::spill_to_right(b_star_node* node, int idx, int &nodes){

    b_star_node *right_sibling = node->parent->child[idx+1];

    right_sibling->child[right_sibling->num_keys_filled+1] = right_sibling->child[right_sibling->num_keys_filled];
    for(int i=right_sibling->num_keys_filled-1; i>=0; i--){
        right_sibling->child[i+1] = right_sibling->child[i];
        right_sibling->keys[i+1] = right_sibling->keys[i];
    }
    right_sibling->keys[0] = node->parent->keys[idx];
    right_sibling->child[0] = node->child[node->num_keys_filled];
    if(right_sibling->child[0])
        right_sibling->child[0]->parent = right_sibling;
    node->child[node->num_keys_filled] = NULL;
    right_sibling->num_keys_filled++;
    
    node->parent->keys[idx] = node->keys[node->num_keys_filled-1];
    
    node->num_keys_filled--;
    nodes += 3;  //visited the node, its right sibling and parent
}

int b_star_tree::get_child_index(b_star_node *node){
    
    if(node->parent == NULL)
        return -1;
    
    int child_idx=0;

    while(node->parent->child[child_idx] != node)
           child_idx++;

    return child_idx;
}

void b_star_tree::split_2_3(b_star_node* node, int &nodes){
    
    /* split node and 1 of its sibling (2 nodes) along with parent key into 
       3 nodes with (2*N-2)/3, (2*N-1)/3, (2*N)/3 nodes respectively and 2 parent keys*/
       
    int node1_keys = (2*N-2)/3;
    int node2_keys = (2*N-1)/3;
    int node3_keys = (2*N)/3;
    int keys[3*N];
    b_star_node *children[3*N];
    int i=0, j=0;
    b_star_node *node1, *node2 = new b_star_node, *node3;

    int child_idx = get_child_index(node);
    if(child_idx == node->parent->num_keys_filled){
        //no right sibling so consider left sibling
        node1 = node->parent->child[child_idx-1];
        node3 = node->parent->child[child_idx];
        child_idx--; //Left child of the 2 children that we split into 3
    }
    else{
        node1 = node->parent->child[child_idx];
        node3 = node->parent->child[child_idx+1];
    }
   
    i=0;
    j=0;
    while(j<node1->num_keys_filled){
        keys[i] = node1->keys[j];
        children[i] = node1->child[j];
        i++;
        j++;
    }
    keys[i] = node1->parent->keys[child_idx];
    children[i] = node1->child[j];
    i++;

    j=0;
    while(j<node3->num_keys_filled){
        keys[i] = node3->keys[j];
        children[i] = node3->child[j];
        i++;
        j++;
    }
    children[i] = node3->child[j];

    //need not copy elements onto node1 as they are already present. Just delete the extra elements
    j=node1_keys;
    node1->num_keys_filled = node1_keys;
    node1->parent->keys[child_idx] = keys[j];
    j++;

    i=0;
    while(i<node2_keys){
        node2->keys[i] = keys[j];
        node2->child[i] = children[j];
        if(node2->child[i])
            node2->child[i]->parent = node2;
        i++;
        j++;
    }

    //Insert keys[j] in parent
    int k;
    for(k=node1->parent->num_keys_filled-1; k>=0 && node1->parent->keys[k] > keys[j]; k--){
        node1->parent->keys[k+1] = node1->parent->keys[k];
    }
    
    node1->parent->keys[k+1] = keys[j];
    node1->parent->num_keys_filled++;
    
    node2->child[i] = children[j];
    if(node2->child[i])
        node2->child[i]->parent = node2;
    node2->num_keys_filled = node2_keys;
    j++;
    for(i=node1->parent->num_keys_filled-1; i>child_idx; i--)
        node1->parent->child[i+1] = node1->parent->child[i];
    node->parent->child[i+1] = node2;
    node2->parent = node1->parent;
    if(node2->child[0])
        node2->isLeaf = false;
    
    i=0;
    while(i<node3_keys){
        node3->keys[i] = keys[j];
        node3->child[i] = children[j];
        if(node3->child[i])
            node3->child[i]->parent = node3;
        i++;
        j++;
    }
    node3->child[i] = children[j];
    if(node3->child[i])
        node3->child[i]->parent = node3;
    node3->num_keys_filled = node3_keys;

    nodes += 3; //visited the node, it's sibling and parent
    
}


b_star_tree::~b_star_tree(){
    empty(root);
}

//delete the memory allocated for the tree
void b_star_tree::empty(b_star_node *node){

    if(node != NULL){

        int i=0;

        while(i<=node->num_keys_filled){
            empty(node->child[i]);
            i++;
        }

        delete node;
        node = NULL;
    }
}

pair<b_star_node*,int> b_star_tree::find_node_key(int val, int &nodes){

    int i=0;
    b_star_node *cur_node = root;

    while(cur_node && cur_node->keys[i] != val){
        if(i == cur_node->num_keys_filled || cur_node->keys[i] > val){
            cur_node = cur_node->child[i];
            i=-1;
            nodes++;
        }
        i++;
    }

    return make_pair(cur_node, i);
}

bool b_star_tree::search(int val, int &nodes){

    int i = 0;
    b_star_node *cur_node = root;

    while(cur_node && cur_node->keys[i] != val){
        
        if(i == cur_node->num_keys_filled || cur_node->keys[i] > val){
            cur_node = cur_node->child[i];
            i=-1;
            nodes++;
        }

        i++;
    }
    
    if(cur_node &&cur_node->keys[i] == val){
        nodes++;
        return true;
    }
    return false;
}

bool b_star_tree::isUnderloaded(b_star_node *node){

    if(node == NULL)
        return false;
    return (node != root && node->num_keys_filled < min_keys) || (node == root && node->num_keys_filled < min_keys_root);
}

bool b_star_tree::borrow_from_siblings(b_star_node *node, int &nodes){
    //if the current node has to borrow a key from its right sibling then it can be considered as 
    // the right sibling wants to give an excess key that it has to it's left sibling (the current node)
    //So current node borrowing a key from right sibling is the same as 
    //right sibling spilling a key to its left sibling

    int child_idx = get_child_index(node);

    if(child_idx != node->parent->num_keys_filled && 
            node->parent->child[child_idx+1]->num_keys_filled > min_keys){
        
        //borrow from right sibling or right sibling gives an excess key to current node (its left sibling)
        spill_to_left(node->parent->child[child_idx+1], child_idx+1, nodes);
        return true;
    }

    else if(child_idx != 0 && node->parent->child[child_idx-1]->num_keys_filled > min_keys){
    
        //borrow from left or left sibling gives an excess key to current node (its right sibling)
        spill_to_right(node->parent->child[child_idx-1], child_idx-1, nodes);
        return true;
    }

    return false;
}

//called only when sibings have minimum keys = 2*((2N-1)/3-1)+1 = 2*(2N-1)/3-1 <= max_keys_root
// and there are only two children whose parent is the root node 
// So just merge the children
void b_star_tree::merge_root(int &nodes){
    
    int keys[max_keys_root];
    int children[max_keys_root+1];
    b_star_node *left_child = root->child[0], *right_child = root->child[1];
    int i=0, j=0;
    int root_key = root->keys[0];

    while(j<left_child->num_keys_filled){
        root->keys[i] = left_child->keys[j];
        root->child[i] = left_child->child[j];
        if(root->child[i])
            root->child[i]->parent = root;
        i++;
        j++;
    }
    root->keys[i] = root_key;
    root->child[i] = left_child->child[j];
    if(root->child[i])
        root->child[i]->parent = root;
    i++;

    j=0;
    while(j<right_child->num_keys_filled){
        root->keys[i] = right_child->keys[j];
        root->child[i] = right_child->child[j];
        if(root->child[i])
            root->child[i]->parent = root;
        i++;
        j++;
    }
    root->child[i] = right_child->child[j];
    if(root->child[i])
        root->child[i]->parent = root;
    root->num_keys_filled = i;

    if(!root->child[0])
        root->isLeaf = true;
    
    delete left_child;
    delete right_child;
    nodes += 3;  //visited root and its 2 children

}

void b_star_tree::merge_2_3(b_star_node *node, int &nodes){
    
    int child_idx = get_child_index(node);
    b_star_node *node1, *node2, *node3;
    int keys[3*N];
    b_star_node* children[3*N];

    if(child_idx == 0){
        node1 = node->parent->child[child_idx];
        node2 = node->parent->child[child_idx+1];
        node3 = node->parent->child[child_idx+2];
    }

    else if(child_idx == node->parent->num_keys_filled){
        node1 = node->parent->child[child_idx-2];
        node2 = node->parent->child[child_idx-1];
        node3 = node->parent->child[child_idx];
        child_idx = child_idx-2;
    }

    else{
        node1 = node->parent->child[child_idx-1];
        node2 = node->parent->child[child_idx];
        node3 = node->parent->child[child_idx+1];
        child_idx = child_idx-1;
    }
    
    int i=0;
    int j=0;
    while(j<node1->num_keys_filled){
        keys[i] = node1->keys[j];
        children[i] = node1->child[j];
        i++;
        j++;
    }
    keys[i] = node1->parent->keys[child_idx];
    children[i] = node1->child[j];
    i++;

    j=0;
    while(j<node2->num_keys_filled){
        keys[i] = node2->keys[j];
        children[i] = node2->child[j];
        i++;
        j++;
    }
    keys[i] = node2->parent->keys[child_idx+1];
    children[i] = node2->child[j];
    i++;

    j=0;
    while(j<node3->num_keys_filled){
        keys[i] = node3->keys[j];
        children[i] = node3->child[j];
        i++;
        j++;
    }
    children[i] = node3->child[j];

    int node1_size = i/2;
    int node2_size = (i-i/2)-1; //1 for parent

    i=node1->num_keys_filled;
    j=node1->num_keys_filled;;
    while(i<node1_size){
        node1->keys[i] = keys[j];
        node1->child[i] = children[j];
        if(node1->child[i])
            node1->child[i]->parent = node1;
        i++;
        j++;
    }
    node1->child[i] = children[j];
    if(node1->child[i])
        node1->child[i]->parent = node1;
    node1->parent->keys[child_idx] = keys[j];
    node1->num_keys_filled = node1_size;
    j++;

    i=0;
    while(i<node2_size){
        node2->keys[i] = keys[j];
        node2->child[i] = children[j];
        if(node2->child[i])
            node2->child[i]->parent = node2;
        i++;
        j++;
    }
    node2->child[i] = children[j];
    if(node2->child[i])
        node2->child[i]->parent = node2;
    node2->num_keys_filled = node2_size;

    //Delete the key and child
    i=child_idx+1;
    while(i<node1->parent->num_keys_filled-1){
        node1->parent->keys[i] = node1->parent->keys[i+1];
        node1->parent->child[i+1] = node1->parent->child[i+2];
        i++;
    }
    node1->parent->num_keys_filled--;

    delete node3;
    nodes += 4;  //visited 3 nodes and their parent to merge them

}

b_star_node* b_star_tree::get_greater_of_minor(b_star_node* node, int &nodes){

    nodes++;
    
    if(node->isLeaf)
        return node;
    else
        return get_greater_of_minor(node->child[node->num_keys_filled], nodes);
}

void b_star_tree::handle_underload(b_star_node *node, int &nodes){
  
  //no more keys/nodes 
  if(node == root && node->num_keys_filled == 0){

    delete root;
    root = NULL;
    return;
  }
    
  //try spill
  bool borrow = borrow_from_siblings(node, nodes);

  //2-3 merge
  if(!borrow){

    if(node->parent != root || (node->parent == root && node->parent->num_keys_filled > 1))
        merge_2_3(node, nodes);
    else 
        merge_root(nodes); //root has only 1 key and both children have minimum number of keys
  }
}

void b_star_tree::remove(int val, int &nodes){

    if(root == NULL)
        return;

    pair<b_star_node*, int> p = find_node_key(val, nodes);
    b_star_node* delete_node = p.first;
    int key_idx = p.second;

    if(delete_node == NULL)
        return;
    
    b_star_node* cur_node = delete_node;
    
    if(!cur_node->isLeaf){
        b_star_node *leaf_node  = get_greater_of_minor(cur_node->child[key_idx], nodes);
        cur_node->keys[key_idx] = leaf_node->keys[leaf_node->num_keys_filled-1];
        cur_node = leaf_node;
    }
    else{
        for(int i=key_idx; i<cur_node->num_keys_filled-1; i++)
            cur_node->keys[i] = cur_node->keys[i+1];
        nodes++;
    }

    cur_node->num_keys_filled--;

    while(cur_node && isUnderloaded(cur_node)){

        b_star_node *parent = cur_node->parent;
        handle_underload(cur_node, nodes);
        cur_node = parent;
    }

}

b_star_node* b_star_tree::find_min(int &nodes){
   
    b_star_node *cur_node = root;

    while(cur_node && !cur_node->isLeaf){
        cur_node = cur_node->child[0];
        nodes++;
    }
    if(cur_node != NULL)
        nodes++;
    return cur_node;
}

void b_star_tree::extract_min(int &nodes){

    b_star_node *delete_node = find_min(nodes);
    remove(delete_node->keys[0], nodes);
}


void b_star_tree::construct_tree(int *arr, int n){

    int count = 0;
    for(int i=0; i<n; i++, count=0){   
        insert(arr[i], count);
    }
}




/********************** B-PLUS TREE ************************/

struct Block {

    int node_count;
    Block * parentBlock;
    int value[MAX];
    Block * childBlock[MAX];
    Block() {
        node_count = 0;
        parentBlock = NULL;
        for (int i = 0; i < MAX; i++) {
            value[i] = INT_MAX;
            childBlock[i] = NULL;
        }
    }
};

class b_plus_tree {

    void splitLeaf(Block * , int & );
    void splitNonLeaf(Block * curBlock, int &);
    void redistributeBlock(Block * , Block * , bool, int, int,int &);
    void mergeBlock(Block * , Block * , bool, int, int &);

    public:
    Block * rootBlock = new Block();
    int bplus_order = 4; //basically the order of the B+ Tree, 4 if not specified by user otherwise
    bool search(Block * , int, int & );
    void insertNode(Block * curBlock, int val, int & nodes);
    int findmin(Block * curBlock, int & nodes);
    void deleteNode(Block * curBlock, int val, int curBlockPosition, int & nodes);
    void extractMin(Block * curBlock, int & nodes);
    void print(vector < Block * > Blocks);
    void construct_bplustree(int * arr, int n);
    void fn();
};

void b_plus_tree::splitLeaf(Block * curBlock, int &nodes) {
    int x, i, j;

    x = bplus_order / 2;
    Block * rightBlock = new Block();

    //leftBlock has x number of nodes
    curBlock -> node_count = x;
    // rightBlock has bplus_order-x
    rightBlock -> node_count = bplus_order - x;
    //so both of them have their common parent
    rightBlock -> parentBlock = curBlock -> parentBlock;
    for (i = x, j = 0; i < bplus_order; i++, j++) {
        rightBlock -> value[j] = curBlock -> value[i];
        curBlock -> value[i] = INT_MAX;
    }
    //for splitting the leaf blocks we
    // copy the first item from the rightBlock to their parentBlock nand val contains that value
    int val = rightBlock -> value[0];

    //if the leaf itself is a parent then
    if (curBlock -> parentBlock == NULL) {
        Block * parentBlock = new Block();
        nodes++;
        parentBlock -> parentBlock = NULL;
        parentBlock -> node_count = 1;
        parentBlock -> value[0] = val;
        parentBlock -> childBlock[0] = curBlock;
        parentBlock -> childBlock[1] = rightBlock;
        curBlock -> parentBlock = rightBlock -> parentBlock = parentBlock;
        rootBlock = parentBlock;
        return;
    } else { //if the splitted leaf block is not rootBlock then

        curBlock = curBlock -> parentBlock;
        nodes++;
        Block * newChildBlock = new Block();
        newChildBlock = rightBlock;

        for (i = 0; i <= curBlock -> node_count; i++) {
            if (val < curBlock -> value[i]) {
                swap(curBlock -> value[i], val);
            }
        }
        curBlock -> node_count++;
        for (i = 0; i < curBlock -> node_count; i++) {
            if (newChildBlock -> value[0] < curBlock -> childBlock[i] -> value[0]) {
                swap(curBlock -> childBlock[i], newChildBlock);
            }
        }
        curBlock -> childBlock[i] = newChildBlock;
        //just for safety
        for (i = 0; curBlock -> childBlock[i] != NULL; i++) {
            curBlock -> childBlock[i] -> parentBlock = curBlock;
        }
    }

}

//function to split the non leaf nodes
void b_plus_tree::splitNonLeaf(Block * curBlock, int &nodes) {
    int x, i, j;

    x = bplus_order / 2;

    Block * rightBlock = new Block();
    curBlock -> node_count = x;
    rightBlock -> node_count = bplus_order - x - 1;
    rightBlock -> parentBlock = curBlock -> parentBlock;

    for (i = x, j = 0; i <= bplus_order; i++, j++) {
        rightBlock -> value[j] = curBlock -> value[i];
        rightBlock -> childBlock[j] = curBlock -> childBlock[i];
        curBlock -> value[i] = INT_MAX;
        if (i != x) curBlock -> childBlock[i] = NULL;
    }

    //we will take a copy of the first item of the rightBlock and
    //delete that item later from the list
    int val = rightBlock -> value[0];
        
    memcpy( & rightBlock -> value, & rightBlock -> value[1], sizeof(int) * (rightBlock -> node_count + 1));
    memcpy( & rightBlock -> childBlock, & rightBlock -> childBlock[1], sizeof(rootBlock) * (rightBlock -> node_count + 1));
    for (i = 0; curBlock -> childBlock[i] != NULL; i++) {
        curBlock -> childBlock[i] -> parentBlock = curBlock;
    }
    for (i = 0; rightBlock -> childBlock[i] != NULL; i++) {
        rightBlock -> childBlock[i] -> parentBlock = rightBlock;
    }

    //if the splitted block itself a parent
    if (curBlock -> parentBlock == NULL) {
        Block * parentBlock = new Block();
        nodes++;
        parentBlock -> parentBlock = NULL;
        parentBlock -> node_count = 1;
        parentBlock -> value[0] = val;
        parentBlock -> childBlock[0] = curBlock;
        parentBlock -> childBlock[1] = rightBlock;
        curBlock -> parentBlock = rightBlock -> parentBlock = parentBlock;
        rootBlock = parentBlock;
        return;
    } else {

        curBlock = curBlock -> parentBlock;
        nodes++;
        Block * newChildBlock = new Block();
        newChildBlock = rightBlock;

        //put val at the exact position of values[] in the parentBlock 
        for (i = 0; i <= curBlock -> node_count; i++) {
            if (val < curBlock -> value[i]) {
                swap(curBlock -> value[i], val);
            }
        }

        curBlock -> node_count++;

        //rightBlock -> at the exact position of childBlock[] in the parentBlock [here curBlock]

        for (i = 0; i < curBlock -> node_count; i++) {
            if (newChildBlock -> value[0] < curBlock -> childBlock[i] -> value[0]) {
                swap(curBlock -> childBlock[i], newChildBlock);
            }
        }
        curBlock -> childBlock[i] = newChildBlock;

        //safety
        for (i = 0; curBlock -> childBlock[i] != NULL; i++) {
            curBlock -> childBlock[i] -> parentBlock = curBlock;
        }
    }

}

void b_plus_tree::insertNode(Block * curBlock, int val, int & nodes) {

    for (int i = 0; i <= curBlock -> node_count; i++) {

        if (val < curBlock -> value[i] && curBlock -> childBlock[i] != NULL) {
            //        	cout<<"Insert 2\n";
            //        	cout<<"This happens when curBlock value[i] is "<<curBlock->value[i]<<endl;
            nodes++;
            insertNode(curBlock -> childBlock[i], val, nodes);

            if (curBlock -> node_count == bplus_order) {
                splitNonLeaf(curBlock, nodes);
                nodes+=3;
                //                	cout<<"Split Leaf\n";
            }
            return;
        } else if (val < curBlock -> value[i] && curBlock -> childBlock[i] == NULL) {
            //        	cout<<"Insert 1\n";
            //        	cout<<"Before swapping curBlock value -> "<<curBlock->value[i]<<endl;
            swap(curBlock -> value[i], val);
            //swap(curBlock->childBlock[i], newChildBlock);
            //          	if(curBlock->childBlock[i]!=NULL)
            //          	{
            ////          		cout<<"The current Block has a child at i:"<<i<<endl;
            //			  }
            //            cout<<"After swapping curBlock value -> "<<curBlock->value[i]<<" and i :"<<i<<endl;
            if (i == curBlock -> node_count) {
                curBlock -> node_count++;
                break;
            }
        }
    }

    if (curBlock -> node_count == bplus_order) {

        splitLeaf(curBlock, nodes);
        nodes+=3;
    }
}

void b_plus_tree::redistributeBlock(Block * leftBlock, Block * rightBlock, bool isLeaf, int posOfLeftBlock, int whichOneisCurBlock, int &nodes) {

    //remember it for later replacement of the copy of this value somewhere in ancestor Block
    int PrevRightFirstVal = rightBlock -> value[0];
    if (whichOneisCurBlock == 0) {

        //leftBlock is curBlock
        //if the blocks are not leaf node
        if (!isLeaf) {
            //bring down the value from which it is left child in parentBlock
            leftBlock -> value[leftBlock -> node_count] = leftBlock -> parentBlock -> value[posOfLeftBlock];
            leftBlock -> childBlock[leftBlock -> node_count + 1] = rightBlock -> childBlock[0];
            leftBlock -> node_count++;
            leftBlock -> parentBlock -> value[posOfLeftBlock] = rightBlock -> value[0];
            memcpy( & rightBlock -> value[0], & rightBlock -> value[1], sizeof(int) * (rightBlock -> node_count + 1));
            memcpy( & rightBlock -> childBlock[0], & rightBlock -> childBlock[1], sizeof(rootBlock) * (rightBlock -> node_count + 1));
            rightBlock -> node_count--;
       

        } else {
            //borrow the first value of rightBlock to the last position of leftBlock
            leftBlock -> value[leftBlock -> node_count] = rightBlock -> value[0];
            leftBlock -> node_count++;
            memcpy( & rightBlock -> value[0], & rightBlock -> value[1], sizeof(int) * (rightBlock -> node_count + 1));
            rightBlock -> node_count--;
            leftBlock -> parentBlock -> value[posOfLeftBlock] = rightBlock -> value[0];
          
        }

    } else {
        //rightBlock is curBlock

        if (!isLeaf) {
            //shift right by one in rightBlock so that first position becomes free
            memcpy( & rightBlock -> value[1], & rightBlock -> value[0], sizeof(int) * (rightBlock -> node_count + 1));
            memcpy( & rightBlock -> childBlock[1], & rightBlock -> childBlock[0], sizeof(rootBlock) * (rightBlock -> node_count + 1));
            //bring down the value from which it is left child in parentBlock to first pos of rightBlock
            rightBlock -> value[0] = leftBlock -> parentBlock -> value[posOfLeftBlock];
            //and the left child of the newly first value of right child will be the last child of leftBlock
            rightBlock -> childBlock[0] = leftBlock -> childBlock[leftBlock -> node_count];

            rightBlock -> node_count++;

            //send up a the last value of the leftBlock to the parentBlock
            leftBlock -> parentBlock -> value[posOfLeftBlock] = leftBlock -> value[leftBlock -> node_count - 1];
            //erase the last element and pointer of leftBlock
            leftBlock -> value[leftBlock -> node_count - 1] = INT_MAX;
            leftBlock -> childBlock[leftBlock -> node_count] = NULL;
            leftBlock -> node_count--;
	
		
        } else {

            //shift by one node to right of the rightBlock so that we can free the first position
            memcpy( & rightBlock -> value[1], & rightBlock -> value[0], sizeof(int) * (rightBlock -> node_count + 1));
            //borrow the last value of leftBlock to the first position of rightBlock
            rightBlock -> value[0] = leftBlock -> value[leftBlock -> node_count - 1];
            rightBlock -> node_count++;

            leftBlock -> value[leftBlock -> node_count - 1] = INT_MAX;
            leftBlock -> node_count--;

            leftBlock -> parentBlock -> value[posOfLeftBlock] = rightBlock -> value[0];
    
        }
    }
    
}

void b_plus_tree::mergeBlock(Block * leftBlock, Block * rightBlock, bool isLeaf, int posOfRightBlock, int &nodes) {

    if (!isLeaf) {

        leftBlock -> value[leftBlock -> node_count] = leftBlock -> parentBlock -> value[posOfRightBlock - 1];
        leftBlock -> node_count++;
      
    }

    memcpy( & leftBlock -> value[leftBlock -> node_count], & rightBlock -> value[0], sizeof(int) * (rightBlock -> node_count + 1));
    memcpy( & leftBlock -> childBlock[leftBlock -> node_count], & rightBlock -> childBlock[0], sizeof(rootBlock) * (rightBlock -> node_count + 1));

    leftBlock -> node_count += rightBlock -> node_count;
    memcpy( & leftBlock -> parentBlock -> value[posOfRightBlock - 1], & leftBlock -> parentBlock -> value[posOfRightBlock], sizeof(int) * (leftBlock -> parentBlock -> node_count + 1));
    memcpy( & leftBlock -> parentBlock -> childBlock[posOfRightBlock], & leftBlock -> parentBlock -> childBlock[posOfRightBlock + 1], sizeof(rootBlock) * (leftBlock -> parentBlock -> node_count + 1));
    leftBlock -> parentBlock -> node_count--;

    // safety
    for (int i = 0; leftBlock -> childBlock[i] != NULL; i++) {
        leftBlock -> childBlock[i] -> parentBlock = leftBlock;
    }
    

}
bool dataFound;

//O(logn) - depends on height, complexity does not depend on order of B+ Tree as it is a constant
int b_plus_tree::findmin(Block * curBlock, int & nodes) {
    if (curBlock == NULL) {
        //cout << " B+ Tree is empty\n";
        return 0;
    } else {
        Block * cursor = curBlock; //or rootBlock would work just fine
        while (cursor -> childBlock[0] != NULL) //it hits null when it reaches the leaf
        {
            cursor = cursor -> childBlock[0];
            nodes++;
        }
        int min = cursor -> value[0];
        return min;

    }
}

//search in O(m.logn) time
bool b_plus_tree::search(Block * curBlock, int x, int & nodes) { //nodes passed initially as zero
    if (curBlock != NULL) {
        Block * cursor = curBlock; //or rootBlock would work just fine
        while (cursor -> childBlock[0] != NULL) {
            for (int i = 0; i < cursor -> node_count; i++) {
                if (x < cursor -> value[i]) {
                    cursor = cursor -> childBlock[i];
                    nodes++;
                    break;
                }
                if (i == cursor -> node_count - 1) {
                    cursor = cursor -> childBlock[i + 1];
                    nodes++;
                    break;
                }
            }
        }
        for (int i = 0; i < cursor -> node_count; i++) {
            if (cursor -> value[i] == x) {
                return true;
            }
        }
    }
    return false;
}

void b_plus_tree::deleteNode(Block * curBlock, int val, int curBlockPosition, int & nodes) {
	

    //if the current block is a leaf or not
    bool isLeaf;
    if (curBlock -> childBlock[0] == NULL)
        isLeaf = true;
    else isLeaf = false;

    //left most value could be changed due to merge or re-distribution later,so keep it to replace it's copy from it's ancestor
    int prevLeftMostVal = curBlock -> value[0];

    for (int i = 0; dataFound == false && i <= curBlock -> node_count; i++) {
        if (val < curBlock -> value[i] && curBlock -> childBlock[i] != NULL) {
            nodes++;
            deleteNode(curBlock -> childBlock[i], val, i, nodes);

        }
        //if we  find the target value at any leafBlock then
        else if (val == curBlock -> value[i] && curBlock -> childBlock[i] == NULL) {

            memcpy( & curBlock -> value[i], & curBlock -> value[i + 1], sizeof(int) * (curBlock -> node_count + 1));
            curBlock -> node_count--;
            dataFound = true;
            break;
        }
    }

    //if the root is the only leaf
    if (curBlock -> parentBlock == NULL && curBlock -> childBlock[0] == NULL) {
        return;
    }

    //if the curBlock is rootBlock and it has one pointers only
    if (curBlock -> parentBlock == NULL && curBlock -> childBlock[0] != NULL && curBlock -> node_count == 0) {
        rootBlock = curBlock -> childBlock[0];
        rootBlock -> parentBlock = NULL;
        return;
    }

    if (isLeaf && curBlock -> parentBlock != NULL) {

        if (curBlockPosition == 0) {
            Block * rightBlock = new Block();
            rightBlock = curBlock -> parentBlock -> childBlock[1];

            //if we the right one has more than half nodes of maximum capacity than re-distribute
            if (rightBlock != NULL && rightBlock -> node_count > (bplus_order + 1) / 2) {

                redistributeBlock(curBlock, rightBlock, isLeaf, 0, 0, nodes);
            }
            //else there is nothing to re-distribute, so we can merge them
            else if (rightBlock != NULL && curBlock -> node_count + rightBlock -> node_count < bplus_order) {

                mergeBlock(curBlock, rightBlock, isLeaf, 1, nodes);
            }
        } else {

            Block * leftBlock = new Block();
            Block * rightBlock = new Block();

            leftBlock = curBlock -> parentBlock -> childBlock[curBlockPosition - 1];

            rightBlock = curBlock -> parentBlock -> childBlock[curBlockPosition + 1];

            //if we see that left one has more than half nodes of maximum capacity then try to re-distribute
            if (leftBlock != NULL && leftBlock -> node_count > (bplus_order + 1) / 2) {
                redistributeBlock(leftBlock, curBlock, isLeaf, curBlockPosition - 1, 1,nodes);
            } else if (rightBlock != NULL && rightBlock -> node_count > (bplus_order + 1) / 2) {
                redistributeBlock(curBlock, rightBlock, isLeaf, curBlockPosition, 0,nodes);
            } else if (leftBlock != NULL && curBlock -> node_count + leftBlock -> node_count < bplus_order) {
                mergeBlock(leftBlock, curBlock, isLeaf, curBlockPosition,nodes);
            } else if (rightBlock != NULL && curBlock -> node_count + rightBlock -> node_count < bplus_order) {
                mergeBlock(curBlock, rightBlock, isLeaf, curBlockPosition + 1,nodes);
            }
        }
        
        nodes+=3;
    } else if (!isLeaf && curBlock -> parentBlock != NULL) {

        if (curBlockPosition == 0) {
            Block * rightBlock = new Block();
            rightBlock = curBlock -> parentBlock -> childBlock[1];

            //if we see the right one has more than half nodes of maximum capacity than re-distribute
            if (rightBlock != NULL && rightBlock -> node_count - 1 >= ceil((bplus_order - 1) / 2)) {
                redistributeBlock(curBlock, rightBlock, isLeaf, 0, 0, nodes);
            }
            //else there is nothing to re-distribute, so we can merge them
            else if (rightBlock != NULL && curBlock -> node_count + rightBlock -> node_count < bplus_order - 1) {
                mergeBlock(curBlock, rightBlock, isLeaf, 1,nodes);
            }
        }
        //for any other case we can safely take the left one to try for re-distribution
        else {

            Block * leftBlock = new Block();
            Block * rightBlock = new Block();

            leftBlock = curBlock -> parentBlock -> childBlock[curBlockPosition - 1];

            rightBlock = curBlock -> parentBlock -> childBlock[curBlockPosition + 1];

            //if we see that left one has more than half nodes of maximum capacity then try to re-distribute
            if (leftBlock != NULL && leftBlock -> node_count - 1 >= ceil((bplus_order - 1) / 2)) {
                redistributeBlock(leftBlock, curBlock, isLeaf, curBlockPosition - 1, 1, nodes);
            } else if (rightBlock != NULL && rightBlock -> node_count - 1 >= ceil((bplus_order - 1) / 2)) {
                redistributeBlock(curBlock, rightBlock, isLeaf, curBlockPosition, 0, nodes);
            }
            //else there is nothing to re-distribute, so we merge them
            else if (leftBlock != NULL && curBlock -> node_count + leftBlock -> node_count < bplus_order - 1) {
                mergeBlock(leftBlock, curBlock, isLeaf, curBlockPosition, nodes);
            } else if (rightBlock != NULL && curBlock -> node_count + rightBlock -> node_count < bplus_order - 1) {
                mergeBlock(curBlock, rightBlock, isLeaf, curBlockPosition + 1, nodes);
            }
        }
			
			nodes+=3;
				
    }
    //delete the duplicate if any in the ancestor Block
    Block * tempBlock = new Block();
    tempBlock = curBlock -> parentBlock;
    while (tempBlock != NULL) {
        for (int i = 0; i < tempBlock -> node_count; i++) {
            if (tempBlock -> value[i] == prevLeftMostVal) {
                tempBlock -> value[i] = curBlock -> value[0];
                nodes++;
                break;
            }
        }
        tempBlock = tempBlock -> parentBlock;
      
    }

}
//heavier function here is delete -> O(mlogn)    
void b_plus_tree::extractMin(Block * curBlock, int & nodes) {

    int min = findmin(curBlock, nodes);
    nodes = 0;
    deleteNode(rootBlock, min, 0, nodes);

}

void b_plus_tree::print(vector < Block * > Blocks) {
    vector < Block * > newBlocks;
    for (int i = 0; i < Blocks.size(); i++) { //for every block
        Block * curBlock = Blocks[i];

        int j;
        for (j = 0; j < curBlock -> node_count; j++) { //traverse the childBlocks, print values and save all the childBlocks
            
            if (curBlock -> childBlock[j] != NULL)
                newBlocks.push_back(curBlock -> childBlock[j]);
        }
        if (curBlock -> value[j] == INT_MAX && curBlock -> childBlock[j] != NULL)
            newBlocks.push_back(curBlock -> childBlock[j]);
        
    }

    if (newBlocks.size() == 0) { //if there is no childBlock block left to send out then just the end of the recursion

        puts("");
        puts("");
        Blocks.clear();
        //exit(0);
    } else { //else send the childBlocks to the recursion to continue to the more depth
        puts("");
        puts("");
        Blocks.clear();
        print(newBlocks);
    }
}

//constructing a b+ tree from given n elements
void b_plus_tree::construct_bplustree(int * arr, int n) { // n-> size of input array 
    int nodes = 0;
    for (int i = 0; i < n ; i++) {
        insertNode(rootBlock, arr[i], nodes);
    }

}

/******************************** VEB - TREE ************************************/

int nodeCountVeb=0;

class VEB_tree{
	
	int u; // universal set size 
	int min; // minimum in cluster
	int max; // maximum in cluster
	VEB_tree* summary; // summary of cluster
	vector<VEB_tree*> cluster;
	int high(int x);
	int low(int x);
	int index(int x, int y);
	int lowerRoot(int n);
	int upperRoot(int n);	

	public:
		// recursively creating the tree
		VEB_tree(int size)
		{
			u = size;
			min = NIL;
			max = NIL;

			// Base case
			if (size <= 2) 
			{
				summary = nullptr;
				cluster = vector<VEB_tree*>(0, nullptr);
			}
			else 
			{
				int upperSize= upperRoot(u);
				summary = new VEB_tree(upperSize);
				cluster = vector<VEB_tree*>(upperSize, nullptr);
 				for (int i = 0; i < upperSize; i++) {
					cluster[i] = new VEB_tree(lowerRoot(u));
				}
			}
		}

		int getMin();
		int getMax();
		int extractMin();
		void insert(int k);
		void init_tree(int *arr, int size);
		bool find(int n);
		int successor(int n);
		void del_element(int key);

		void printTree();
};

// used to find upper root
int VEB_tree::upperRoot(int x){
	return (int) pow( 2,ceil(log2(x)/2) );
}

//used to find lower root
int VEB_tree::lowerRoot(int x){
	return (int) pow( 2,floor(log2(x)/2) );
}

// used to find the high bits of x
int VEB_tree:: high(int x){
	return x / lowerRoot(u);
}

// used to find the low bits of x
int VEB_tree:: low(int x)
{
	return x % lowerRoot(u);
}

// used to create an index A such that
// high(A) =x, low(A)=y
int VEB_tree:: index(int x, int y)
{
	return x * lowerRoot(u) + y;
}

// gets the minimum element in the node
int VEB_tree:: getMin()
{
	nodeCountVeb++;
	return min != NIL ? min : NIL;
}

// gets the maximum element in the node
int VEB_tree:: getMax()
{
	nodeCountVeb++;
    return (max == NIL ? NIL : max);
}

// inserts elements into the tree
void VEB_tree:: insert(int key)
{
	nodeCountVeb++;
	// check if node is empty
	if (min == NIL) {
		min = key;
		max = key;
	}
	else {
		// inserts element if base case, else just update min
		if (key < min) {
			swap(min, key);
		}

		// for non base case
		if (u > 2) 
		{
			// If no key is present in the cluster then insert key into
			// both cluster and summary

			if (cluster[high(key)]->getMin() == NIL) {
				summary->insert(high(key));

				// Sets the min and max of cluster to the key
				// as no other keys are present we will stop at this level
				// we are not going deeper into the structure like
				// Lazy Propagation
				nodeCountVeb++;
				cluster[ high(key)]->min =  low(key);
				cluster[ high(key)]->max =  low(key);
			}
			else {
				// If there are other elements in the tree then recursively
				// go deeper into the structure to set attributes accordingly
				cluster[ high(key)]->insert(low(key));
			}
		}

		// if key is new max
		if (key >  max) {
			 max = key;
		}
	}
}

//initialises the tree with values given in array
void VEB_tree:: init_tree( int *arr, int size )
{
	for(int i=0; i<size; i++)
	{
		insert(arr[i]);
	}
	nodeCountVeb=0;
}

//searches for an element, returns true if found
bool VEB_tree:: find(int key)
{
	// for elements exceeding u or not existing 
	nodeCountVeb++;
	if (u < key) 
		return false;

	if ( min == key ||  max == key) 
		return true;
	else 
	{
		if (u == 2) 
			return false;
		else 
			return cluster[high(key)]->find(low(key));
	}
}

int VEB_tree:: successor(int key)
{
	// Base case: If key is 0 and its successor
	// is present then return 1 else return null
	nodeCountVeb++;
	if (u == 2) 
	{
		if (key == 0 && max == 1) 
			return 1;
		else 
			return NIL;
	}

	// If key is less then min then return min
	// because it will be successor of the key
	else if (min != NIL && key < min) 
	{
		return min;
	}
	else 
	{
		// Find successor inside the cluster of the key
		// First find the max in the cluster
		int maxClusterK = cluster[high(key)]->getMax();
		int offset, sucessorClusterK;

		// If there is any key( max!=NIL ) present in the cluster then find
		// the successor inside of the cluster
		if (maxClusterK != NIL && low(key) < maxClusterK) {
			offset = cluster[high(key)]->successor(low(key));
			return index(high(key), offset);
		}
		// Otherwise look for the next cluster with at least one key present
		else 
		{
			sucessorClusterK = summary->successor(high(key));
			// If there is no cluster with any key present
			// in summary then return null
			if (sucessorClusterK == NIL) {
				return NIL;
			}
			// Find min in successor cluster which will
			// be the successor of the key
			else {
				offset = cluster[sucessorClusterK]->getMin();
				return index(sucessorClusterK, offset);
			}
		}
	}
}

// deletes elements
void VEB_tree:: del_element(int key)
{
	nodeCountVeb++;
	// only one element in node
	if (max == min) 
	{
		min = NIL;
		max = NIL;
	}

	// base case if min!=max
	else if (u == 2) 
	{
		if (key == 0) 
			min = 1;
		else 
			min = 0;
		max = min;
	}
	else {

		// As we are doing something similar to lazy propagation
		// we will basically find next bigger key
		// and assign it as min
		if (key == min) 
		{
			int first_cluster = summary->getMin();
			key	= index(first_cluster,	cluster[first_cluster]->getMin() );
			min = key;
		}

		// Now we delete the key
		cluster[high(key)]->del_element(low(key));

		// After deleting the key, rest of the improvements

		// If the min in the cluster of the key is NIL
		// then we have to delete it from the summary to
		// eliminate the key completely
		if (cluster[high(key)]->getMin() == NIL)
		{
			summary->del_element(high(key));
			if (key == max) {
				int max_insummary = summary->getMax();

				// If the max value of the summary is null
				// then only one key is present so
				// assign min. to max.
				if (max_insummary == NIL) 
					max = min;
				else 
				{
					// Assign global max of the tree, after deleting
					// our query-key
					max	= index(max_insummary, cluster[max_insummary]->getMax());
				}
			}
		}

		// Simply find the new max key and
		// set the max of the tree
		// to the new max
		else if (key == max) {
			max= index(high(key), cluster[high(key)]->getMax());
		}
	}
}

//extracts min
int VEB_tree:: extractMin(){
	int x = getMin();
	del_element(x);
	return x;
}

/************funtion prototypes ***************/
void writeToFile(char *,int);
void writeNodecount(int count, int isnextcell);
void singleTestRun(int);

//other  Function prototypes
void compute_time(int n);

// utility structure for timing and outputing to csv file
struct Timer{
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    char *buffr;
    std::string s;
    int cellPosition;

    Timer(int n = NEXT_CELL){
        cellPosition = n;
        start = std::chrono::steady_clock::now();
    }

    ~Timer(){
        end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();
        //std::cout << duration <<std:: endl;
        s = std::to_string(duration);
        buffr = &s[0];
        writeToFile(buffr,cellPosition);
    }
};

/******** driver function ************/
int main (){
    outputFile.open(outputPath);    
    srand(0);   
    // srand(time(NULL));
    char *tempstr;
    int loopNO = -1; 
    int n=100;     // Initial size of the array
    int no_of_iterations; // no of iterations
    int arrSizeIncrement = 50; // size increment per iteration 
    writeToFile(csvcolumns,NEXT_ROW);
    cout<< "Enter the number of data points for the plot: ";
    cin >> no_of_iterations;
    cout<< endl;
    while(++loopNO<no_of_iterations){
        tempstr = &(to_string(n)[0]);
        writeToFile(tempstr,NEXT_CELL);   

        // runs the test
        singleTestRun(n);

        n+=arrSizeIncrement;
        //if(n>SIZE){break; cout<<"Exceeded allowed array size.\n";}        // OPTIONAL
    }
    cout<< "Completed "<< loopNO << " iterations.";
    cout<<" Output file created and values have been dumped to the file";
    outputFile.close();
    return 0;
}

/******** function definitions *******/

void writeToFile(char* entry, int navigateTo){
    outputFile << entry;
    if(navigateTo == NEXT_CELL)
    outputFile << ",";
    else 
    outputFile << endl;
}


void writeNodecount(int count, int cellpos=NEXT_CELL){
    std::string s;
    char *buffr;
    s = std::to_string(count);
    buffr = &s[0];
    writeToFile(buffr,cellpos);
}

//runs the test cases and returns output to csv file
void singleTestRun(int n){
    compute_time(n);
}

void compute_time(int n){
    //init array
    int *arr = new int[n];
    set<int> s;
    int i=0;
    while(i<n){
        int arrElement= rand()%n;
        if(s.count(arrElement)==0){
            arr[i] = arrElement;
            s.insert(arrElement);
            i++;
        }
    }

    // construction of trees
    b_star_tree b_star;
    b_star.construct_tree(arr, n);


    b_plus_tree b_plus;
    b_plus.construct_bplustree(arr, n);
    
    VEB_tree V(131072);
    V.init_tree(arr,n);

    
    int nodes = 0;
    //Insertion comparison
    {
    {
        Timer t;
        b_star.insert(n+1, nodes);
    }
    writeNodecount(nodes);

    nodes = 0;
    {
        Timer t;
        b_plus.insertNode(b_plus.rootBlock, n+1, nodes);
    }
    writeNodecount(nodes);
    
    nodeCountVeb=0;
    //veb insert
    {
        Timer t;
        V.insert(n+1);
    }
    writeNodecount(nodeCountVeb);

    }

    //restoring size n after insertion
    b_star.remove(n+1, nodes);  
    dataFound = false;
    b_plus.deleteNode(b_plus.rootBlock, n+1,0, nodes);  
    V.del_element(n+1);  

    //Deletion comparison

    {
    int key = rand()%n;
    nodes = 0;
    {         
        Timer t;
        b_star.remove(arr[key], nodes);
    }
    writeNodecount(nodes);


    nodes = 0;
    {
        dataFound = false;
        Timer t;
        b_plus.deleteNode(b_plus.rootBlock, arr[key],0, nodes);
    }
    writeNodecount(nodes);
    
    nodeCountVeb = 0;
    {
        Timer t;
        V.del_element(arr[key]);
    }
    writeNodecount(nodeCountVeb);
    }

    //Search comparison
    {
    int key = rand()%n+1;
    nodes = 0;
    {
        Timer t;
        b_star.search(key, nodes);
    }
    writeNodecount(nodes);

    nodes = 0;
    {
        Timer t;
        b_plus.search(b_plus.rootBlock, key, nodes);
    }
    writeNodecount(nodes);

    nodeCountVeb=0;
    {
        Timer t;
        V.find(key);
    }
    writeNodecount(nodeCountVeb);
    }

    //Find min comparison
    {
    nodes = 0;
    {
        Timer t;
        b_star.find_min(nodes);
    }
    writeNodecount(nodes);

    nodes = 0;
    {
        Timer t;
        b_plus.findmin(b_plus.rootBlock, nodes);
    }
    writeNodecount(nodes);

    nodeCountVeb=0;
    {
        Timer t;
        V.getMin();
    }
    writeNodecount(nodeCountVeb);
    }

    //Extract min comparison
    {
    nodes = 0;
    {
        Timer t;
        b_star.extract_min(nodes);
    }
    writeNodecount(nodes);

    nodes = 0;
    {
        dataFound = false;
        Timer t;
        b_plus.extractMin(b_plus.rootBlock, nodes);
    }
    writeNodecount(nodes);

    nodeCountVeb=0;
    {
        Timer t;
        V.extractMin();
    }
    writeNodecount(nodeCountVeb,NEXT_ROW);
    }

    free(arr);
}



