# compare_Bplus_Bstar_VanEmdeBoas
A group project (**CSPE43** - Advanced DSA) wherein a simple **Performance Analysis and Comparison of 3 popular multi-way search trees** was done. The trees chosen were: **B+ Tree, B-Star Tree and Van Emde Boas Tree**

The aim of our project was to compare the performances of the trees with respect to **5 specific cases**: -
1.	Insertion
2.	Deletion
3.	Search
4.	Find min
5.	Extract min

**Do note:-**

* **ADSA_GrpNo_18_FinalReport.pdf** is the cumulative report , consisting of detailed information regarding each tree and its performance. A final inference and comparison (layman interpretation) was done with respect to each operation. 
* **compare_trees.cpp** is the program used to generate the csv file for all the graphs. It runs some number of iterations, as per user input. Each iteration, it increments the number of input elements by 25. This is added as a proof of genuine plotting of the graph.
* For **B+ Tree**, the default order has been set to 4. However, the tester can respecify the order to his need through the console. In addition, provided the code with a menu option to aid in testing.
* For **veb tree**, the default size of the universal set is 1024, this value can be changed, but it must be a power of 2. Running the code will output the needed instructions to test the 5 basic operations.
* **seminarDeck** was the introductory slideDeck we used for the live demo.


