#include<bits/stdc++.h>
using namespace std;

#define MAX 100 // Threshold on maximum order

struct Block { // Node Block that will hold a maximum of m-1 keys and pointers to a maximum of m children

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

class b_plus_tree { //A bplus tree to host the relevant member functions  and 

    void splitLeaf(Block * , int & );
    void splitNonLeaf(Block * curBlock, int &);
    void redistributeBlock(Block * , Block * , bool, int, int,int &);
    void mergeBlock(Block * , Block * , bool, int, int &);

    public:
    Block * rootBlock = new Block();
    int bplus_order = 4; //basically the order of the B+ Tree, 4 if not specified by user otherwise
    void search(Block * , int, int & );
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
        nodes++;
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
        nodes++;
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
    // For Debuggin
    //    cout<<"Before right shifting:-\n";
    //      for(i=0;curBlock->childBlock[i]!=NULL;i++){
    //      curBlock->childBlock[i]->parentBlock = curBlock;
    //        cout<<"Value i:"<<i<<" is "<<curBlock->value[i]<<endl;
    //    }
    //    for(i=0;rightBlock->childBlock[i]!=NULL;i++){
    //        rightBlock->childBlock[i]->parentBlock = rightBlock;
    //        cout<<"Value i:"<<i<<" is "<<rightBlock->value[i]<<endl;
    //    }
    //    
    //    
    memcpy( & rightBlock -> value, & rightBlock -> value[1], sizeof(int) * (rightBlock -> node_count + 1));
    memcpy( & rightBlock -> childBlock, & rightBlock -> childBlock[1], sizeof(rootBlock) * (rightBlock -> node_count + 1));
    for (i = 0; curBlock -> childBlock[i] != NULL; i++) {
        curBlock -> childBlock[i] -> parentBlock = curBlock;
        //        cout<<"Value i:"<<i<<" is "<<curBlock->value[i]<<endl;
    }
    for (i = 0; rightBlock -> childBlock[i] != NULL; i++) {
        rightBlock -> childBlock[i] -> parentBlock = rightBlock;
        //        cout<<"Value i:"<<i<<" is "<<rightBlock->value[i]<<endl;
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
        nodes++;
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
            nodes++;

        } else {
            //borrow the first value of rightBlock to the last position of leftBlock
            leftBlock -> value[leftBlock -> node_count] = rightBlock -> value[0];
            leftBlock -> node_count++;
            memcpy( & rightBlock -> value[0], & rightBlock -> value[1], sizeof(int) * (rightBlock -> node_count + 1));
            rightBlock -> node_count--;
            leftBlock -> parentBlock -> value[posOfLeftBlock] = rightBlock -> value[0];
            nodes++;
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
			nodes++;
		
        } else {

            //shift by one node to right of the rightBlock so that we can free the first position
            memcpy( & rightBlock -> value[1], & rightBlock -> value[0], sizeof(int) * (rightBlock -> node_count + 1));
            //borrow the last value of leftBlock to the first position of rightBlock
            rightBlock -> value[0] = leftBlock -> value[leftBlock -> node_count - 1];
            rightBlock -> node_count++;

            leftBlock -> value[leftBlock -> node_count - 1] = INT_MAX;
            leftBlock -> node_count--;

            leftBlock -> parentBlock -> value[posOfLeftBlock] = rightBlock -> value[0];
            nodes++;
        }
    }
}

void b_plus_tree::mergeBlock(Block * leftBlock, Block * rightBlock, bool isLeaf, int posOfRightBlock, int &nodes) {

    if (!isLeaf) {

        leftBlock -> value[leftBlock -> node_count] = leftBlock -> parentBlock -> value[posOfRightBlock - 1];
        leftBlock -> node_count++;
        nodes++;
    }

    memcpy( & leftBlock -> value[leftBlock -> node_count], & rightBlock -> value[0], sizeof(int) * (rightBlock -> node_count + 1));
    memcpy( & leftBlock -> childBlock[leftBlock -> node_count], & rightBlock -> childBlock[0], sizeof(rootBlock) * (rightBlock -> node_count + 1));

    leftBlock -> node_count += rightBlock -> node_count;
    memcpy( & leftBlock -> parentBlock -> value[posOfRightBlock - 1], & leftBlock -> parentBlock -> value[posOfRightBlock], sizeof(int) * (leftBlock -> parentBlock -> node_count + 1));
    memcpy( & leftBlock -> parentBlock -> childBlock[posOfRightBlock], & leftBlock -> parentBlock -> childBlock[posOfRightBlock + 1], sizeof(rootBlock) * (leftBlock -> parentBlock -> node_count + 1));
    leftBlock -> parentBlock -> node_count--;
	nodes++;
    // safety
    for (int i = 0; leftBlock -> childBlock[i] != NULL; i++) {
        leftBlock -> childBlock[i] -> parentBlock = leftBlock;
    }

}
bool dataFound;

//O(logn) - depends on height, does not depend on order of B+ Tree
int b_plus_tree::findmin(Block * curBlock, int & nodes) {
    if (curBlock == NULL) {
        cout << " B+ Tree is empty\n";
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
void b_plus_tree::search(Block * curBlock, int x, int & nodes) { //nodes passed initially as zero
    if (curBlock == NULL) {
        cout << " B+ Tree is empty\n";
    } else {
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
                cout << "The element : " << x << " has been found\n";
                return;
            }
        }
        cout << "Element does not exist\n";
    }
}

void b_plus_tree::deleteNode(Block * curBlock, int val, int curBlockPosition, int & nodes) {
	
	cout<<"In deletion\n";

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
            cout<<"Data Found!!! \n";
            break;
        }
    }

    //if the root is the only leaf
    if (curBlock -> parentBlock == NULL && curBlock -> childBlock[0] == NULL) {
    	cout<<"Root - only leaf \n";
        return;
    }

    //if the curBlock is rootBlock and it has one pointers only
    if (curBlock -> parentBlock == NULL && curBlock -> childBlock[0] != NULL && curBlock -> node_count == 0) {
        rootBlock = curBlock -> childBlock[0];
        rootBlock -> parentBlock = NULL;
        cout<<"curBlock is rootBlock and it has one pointers only \n";
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

    }
    //delete the duplicate if any in the ancestor Block
    Block * tempBlock = new Block();
    tempBlock = curBlock -> parentBlock;
    while (tempBlock != NULL) {
        for (int i = 0; i < tempBlock -> node_count; i++) {
            if (tempBlock -> value[i] == prevLeftMostVal) {
                tempBlock -> value[i] = curBlock -> value[0];
                break;
            }
        }
        tempBlock = tempBlock -> parentBlock;
        nodes++;
    }

}

//heavier function here is delete -> O(mlogn)    
void b_plus_tree::extractMin(Block * curBlock, int & nodes) {

    int min = findmin(curBlock, nodes);
    cout << "Extracting min.... : " << min << endl;
    //Reintializaing nodes==0 since deleteNode is the heavier function
    nodes = 1;
    deleteNode(rootBlock, min, 0, nodes);

}

void b_plus_tree::print(vector < Block * > Blocks) {
    vector < Block * > newBlocks;
    for (int i = 0; i < Blocks.size(); i++) { //for every block
        Block * curBlock = Blocks[i];

        cout << "[|";
        int j;
        for (j = 0; j < curBlock -> node_count; j++) { //traverse the childBlocks, print values and save all the childBlocks
            cout << curBlock -> value[j] << "|";
            if (curBlock -> childBlock[j] != NULL)
                newBlocks.push_back(curBlock -> childBlock[j]);
        }
        if (curBlock -> value[j] == INT_MAX && curBlock -> childBlock[j] != NULL)
            newBlocks.push_back(curBlock -> childBlock[j]);

        cout << "]  ";
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
        printf("**** Insert %d ****\n\n", arr[i]);
        insertNode(rootBlock, arr[i], nodes);
    }

}

void b_plus_tree::fn() {
//You can utlitize this to precreate a random graph for testing	
	
        int num[100]={0}; 
//       for(int i=0;i<70;i++)
//    		{
//    	num[i]=80-i;
//    		}
//    
//     	construct_bplustree(num,70);
     
    	 FILE *p;
        p = fopen("input.txt", "r");
        freopen("output.txt", "w", stdout);
    
        vector < Block* > Blocks;
        int ch;
        int i = 0;
   // if using the random array we created, size would be -> 70
   //    int totalValues = 70;
   		int totalValues =0;
    
         while(scanf("%d", &ch)!=EOF){
    
    
            if(ch==1){
                scanf("%d", &num[i]);
                printf("**** Insert %d ****\n\n", num[i]);
                int nodes=1;
                insertNode(rootBlock, num[i],nodes);
                cout<<"The no of nodes visited are : "<<nodes<<endl;
                i++;
                totalValues++;
    
            }else if(ch==2){
                Blocks.clear();
                Blocks.push_back(rootBlock);
                print(Blocks);
                puts("");
    
            }else if(ch==3) {
                int val;
                scanf("%d", &val);
                if(totalValues==0) {
                    printf("Sorry! There is no more data to be deleted!");
                    continue;
    
                }
                printf("---- Delete %d ----\n\n", val);
                dataFound = false;
                int nodes=1;
                deleteNode(rootBlock, val, 0,nodes);
                cout<<"The no of nodes visited are : "<<nodes<<endl;
                totalValues--;
            }
            else if(ch==4)
            {
            	int nodes=1;
            	cout<<"The minimum value is : "<<findmin(rootBlock,nodes)<<endl;
            	cout<<"The no of nodes visited are : "<<nodes<<endl;
    		}
    		else if(ch==5)
    		{
    			
                cout<<"Value to search ?\n";
                int avalue;
                cin>>avalue;
                int nodes=1;
                search(rootBlock, avalue, nodes);
                cout<<"The no of nodes visited are : "<<nodes<<endl;
    		}
    		else if(ch==6)
    		{
    			int nodes=1;
    			extractMin(rootBlock, nodes);
    			cout<<"The no of nodes visited are : "<<nodes<<endl;
    		}
        }

}

int main() {

     b_plus_tree t;

//Default order is 4, but can be changed through input from console
        printf("Enter the order : ");
        scanf("%d", &t.bplus_order);
        
	printf("1.Insert a value\n2.Print the tree\n3.Delete a value\n4.Find Min \n5.Search a key \n6.Extract Min \n\n ");
    t.fn(); //This is the member function that can be utlized for testing


    return 0;
}
