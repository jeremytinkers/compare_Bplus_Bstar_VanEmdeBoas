#include <bits/stdc++.h>

using namespace std;

class b_star_node{

    friend class b_star_tree;
    int *keys;
    b_star_node *parent;
    b_star_node** child;
    bool isLeaf;
    int num_keys_filled;

    public:
    b_star_node(int N){
        isLeaf = true;
        num_keys_filled = 0;
        parent = NULL;

        keys = new int[2*((2*N-2)/3)+1];
        child = new b_star_node*[2*((2*N-2)/3)+2];
        int n = 2*((2*N-2)/3)+1;
        for(int i=0; i<n; i++){
            keys[i] = 0;
            child[i] = NULL;
        }
        child[n] = NULL;
    }

    ~b_star_node(){
        delete keys;
        delete child;
    }
};

class b_star_tree{

    b_star_node *root;
    const int N;
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
    b_star_tree(int);
    void insert(int, int &);
    bool search(int, int &);
    void remove(int, int &);
    b_star_node* find_min(int &);
    void extract_min(int &);
    void construct_tree(int*, int);
    void print_tree();
    void fn();
    void empty(b_star_node*);
    void disp_insert(int);
    void disp_del(int);
    void disp_search(int);
    void disp_find_min();
    void disp_extract_min();
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

b_star_tree::b_star_tree(int N): N(N), max_keys(N-1), min_keys(ceil((2*N-1)/3.0)-1), 
                             max_keys_root(2*floor((2*N-2)/3.0)), min_keys_root(1){

    root = new b_star_node(N);        
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

    b_star_node *child1 = new b_star_node(N);
    b_star_node *child2 = new b_star_node(N);
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
    b_star_node *node1, *node2 = new b_star_node(N), *node3;

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

void b_star_tree::disp_insert(int n){ 
  int c=0;

	cout<<"\nInsert "<<n<<"\n";
	insert(n,c);
  cout<<"\nTree: \n";
  print_tree();
}

void b_star_tree::disp_del(int n){ 
  int c=0;

	cout<<"\nDelete "<<n<<"\n";
	remove(n,c);
  cout<<"\nTree: \n";
  print_tree();
}

void b_star_tree::disp_search(int n){ 
  int c=0;

	cout<<"\nSearch "<<n<<"\n";
	if(search(n,c) == true)
    cout<<"Found!";
  else
    cout<<"Not found\n";
}

void b_star_tree::disp_find_min(){ 
  int c=0;

	cout<<"\nFind Min\n";
	b_star_node *t = find_min(c);
  cout<<"Minimum key: "<<t->keys[0]<<"\n";
}

void b_star_tree::disp_extract_min(){ 
  int c=0;

	cout<<"\nExtract min \n";
	extract_min(c);
  cout<<"\nTree: \n";
  print_tree();
}


void b_star_tree::fn(){

  int n;
  vector<int> v;
  cout<<"\nEnter the number of elements: ";
  cin>>n;

  for(int i=1; i<=n; i++){
    v.push_back(i);
  }

  shuffle(v.begin(), v.end(), default_random_engine(time(0)));
  
  for(int i=0; i<n; i++)
      disp_insert(v[i]);

  cout<<"\n\n*****************************DONE INSERTION*************************************\n\n";
     
  int choice = 0;
  char ch= 'Y';
  do{
    cout<<"\n\n\t\t\tMAIN MENU\n";
    cout<<"\t\t\t*********\n";
    cout<<"1. Insert\n2. Delete\n3. Search\n4. Find min\n5. Extract min\n6. Print tree\n7. Clear screen\n";
    cout<<"Enter your choice: ";
    cin>>choice;
    int t;
    switch(choice){
      case 1: 
              cout<<"Enter the element to add: ";
              cin>>t;
              disp_insert(t);
              break;
      case 2: 
              cout<<"Enter the element to delete: ";
              cin>>t;
              disp_del(t);
              break;
      case 3: 
              cout<<"Enter the element to search: ";
              cin>>t;
              disp_search(t);
              break;
      case 4: 
              disp_find_min();
              break;
      case 5: 
              disp_extract_min();
              break;
      case 6: 
              cout<<"Tree (level order traversal):\n";
              print_tree();
              break;
      case 7:
              system("CLS");
      default: 
              cout<<"Invalid choice";
    }
    cout<<"Do you want to continue(Y/y - yes)? ";
    cin>>ch;
  }while(ch == 'y' || ch == 'Y');
}

int main (){  
    int N; 
    cout<<"Enter the order of the tree: ";
    cin>>N;
    b_star_tree t(N);
	t.fn();
}
