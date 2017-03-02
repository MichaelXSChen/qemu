Say if the tree has `x` leaf (which means `x` pages in our project), then

	
	l =  int (log (x));
	
	//Tree height
	th = l + 1; 

	//Total number of all nodes
	nnodes = (2 ^ th - 1) + 2 * (x - 2 * l); 

	indices_of_leaf = [nnodes/2, ..., nnodes-1]  //index from 0

	childer_of_node (y) = (2*y +1, 2 *y +2)

