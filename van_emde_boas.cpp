#include <bits/stdc++.h>
#define NIL -1
using namespace std;


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
		VEB_tree(int size){
			u = size;
			min = NIL;
			max = NIL;

			// Base case
			if (size <= 2) {
				summary = nullptr;
				cluster = vector<VEB_tree*>(0, nullptr);
			}
			else {
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
int VEB_tree:: low(int x){
	return x % lowerRoot(u);
}

// used to create an index A such that
// high(A) =x, low(A)=y
int VEB_tree:: index(int x, int y){
	return x * lowerRoot(u) + y;
}

// gets the minimum element in the node
int VEB_tree:: getMin(){
	return min != NIL ? min : NIL;
}

// gets the maximum element in the node
int VEB_tree:: getMax(){
    return (max == NIL ? NIL : max);
}

// inserts elements into the tree
void VEB_tree:: insert(int key){
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
			// If no key is present in the cluster 
			if (cluster[high(key)]->getMin() == NIL) {
				summary->insert(high(key));

				// Sets the min and max of current empty cluster
				cluster[ high(key)]->min =  low(key);
				cluster[ high(key)]->max =  low(key);
			}
			else {
				// since other elements present in cluster
				// move deeper and repeat
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
void VEB_tree:: init_tree( int *arr, int size ){
	for(int i=0; i<size; i++)	{
		insert(arr[i]);
	}
}

//searches for an element, returns true if found
bool VEB_tree:: find(int key){
	// for elements exceeding u or not existing 
	if (u < key) 
		return false;

	if ( min == key ||  max == key) 
		return true;
	else {
		if (u == 2) 
			return false;
		else 
			return cluster[high(key)]->find(low(key));
	}
}

// finds the next element in sequence, 
// used for printing the keys in the tree
int VEB_tree:: successor(int key){
	// base case: if key is stored in 0 index
	if (u == 2) {
		if (key == 0 && max == 1) 
			return 1;
		else 
			return NIL;
	}

	// when key less than current min
	else if (min != NIL && key < min){
		return min;
	}
	else 
	{
		// finding successor in cluster
		int maxClusterK = cluster[high(key)]->getMax();
		int offset, sucessorClusterK;

		// when maxclusterk is greater than key, return 
		if (maxClusterK != NIL && low(key) < maxClusterK) {
			offset = cluster[high(key)]->successor(low(key));
			return index(high(key), offset);
		}
		else // searching for cluster with any key
		{
			sucessorClusterK = summary->successor(high(key));
			// when no element is in cluster 
			// according to to summary
			if (sucessorClusterK == NIL) {
				return NIL;
			}
			// pick min element in successor cluster
			else {
				offset = cluster[sucessorClusterK]->getMin();
				return index(sucessorClusterK, offset);
			}
		}
	}
}

// deletes elements
void VEB_tree:: del_element(int key){
	// only one element in node
	if (max == min) {
		min = NIL;
		max = NIL;
	}

	// base case if min!=max
	else if (u == 2) {
		if (key == 0) 
			min = 1;
		else 
			min = 0;
		max = min;
	}
	else {

		// find next big element and assign
		// it as min
		if (key == min) 
		{
			int first_cluster = summary->getMin();
			key	= index(first_cluster,	cluster[first_cluster]->getMin() );
			min = key;
		}

		// Now delete the key
		cluster[high(key)]->del_element(low(key));

		// if cluster is empty then we update the summary
		if (cluster[high(key)]->getMin() == NIL){
			summary->del_element(high(key));
			if (key == max) {
				int max_insummary = summary->getMax();

				// max isn't assigned a value yet, only one 
				// key is present, which is min. update max to min
				if (max_insummary == NIL) 
					max = min;
				else {
					// else set max to the max of the child clusters
					max	= index(max_insummary, cluster[max_insummary]->getMax());
				}
			}
		}

		//find the next max key in the same cluster as key
		// set max to new value of max
		else if (key == max) {
			max= index(high(key), cluster[high(key)]->getMax());
		}
	}
}

// finds the element and then deletes it
int VEB_tree:: extractMin(){
	int x = getMin();
	del_element(x);
	return x;
}

//prints the elements in the tree
void VEB_tree:: printTree(){
	cout << "\nThe elements in the tree are:";
	if(find(0)){
		cout << "0, ";
	}
	int x=0;
	while(true){
		x=successor(x);
		if(x!=NIL)
			cout << x << ", ";
		else
			break;
	}
	cout << "\b\b ";
}

int main(){
	VEB_tree V(1024); // u = 2^10
	srand(0);
	int init_size;

	cout << "\nEnter the number of elements to insert:";
	cin >> init_size;	
	
	// Inserting Keys
	int *arr= new int[init_size];
	cout << "\nEnter the keys to be inserted: (range is 0-1023)";
	for(int i=0; i<init_size; i++){
		int x;
		cin >> x;
		if(!V.find(x)){
			V.insert(x);
		}
		else{
			cout << x << " was already inserted.\n";
		}
	}

	// prints the elements in the tree
	V.printTree();
	
	// deletion
	int temp;
	cout << "\nEnter the number of elements to delete:";
	cin >>temp;

	cout <<"\nEnter the elements to be deleted:";
	for(int i=0; i<temp; i++){
		int x;
		cin >> x;
		if(V.find(x)){
			V.del_element(x);
			cout << x << " is deleted.\n";
		}
	}
	
	V.printTree();
	
	cout << "\nEnter element to be searched for:";
	cin>> temp;

	if(V.find(temp)){
		cout << "\nElement "<< temp<< " found!";
	}
	else
		cout << "Element " << temp <<" not found!";
	
	cout << "\n\nThe minimum element in tree is " << V.getMin();
	
	cout << "\nMinimum element shall be extracted.";
	V.extractMin();

	V.printTree();

	return 0;
}




