#include "migration/vmft/hash.h"


#define nthread 20

static hash_t hashofpage(uint8_t *data, int len){
	int	nleft = len;
    hash_sum_t sum = 0;
    hash_t *w = (uint64_t *) data;
    hash_t answer = 0;

    while (nleft > 1)  {
        sum += *w++;
        nleft -= HASH_SHIFT;
    }

    sum = (sum >> HASH_SIZE) + (sum & HASH_MASK);	/* add hi 32 to low 32 */
    sum += (sum >> HASH_SIZE);			/* add carry */
    answer = ~sum;				/* truncate to 16 bits */
    return(answer);
} 


static hash_t hashofhash(hash_t *a, hash_t *b){
	hash_t ret;
	hash_sum_t sum = 0;
	hash_t answer = 0;
	sum = (*a + *b);
	sum = (hash >> HASH_SIZE) + (sum & HASH_MASK)
	answer = ~sum ;
	return answer;

}




struct merkle_tree_t {
	hash_t *tree;
	uint64_t tree_size; 
};

typedef struct merkle_tree_t merkle_tree_t; 



//Currently using multiple pthread_variable to do 

static pthread_t *ts; 
static int indices[nthread];
static pthread_mutex_t locks; 
static pthread_cond_t conds;

//static unsigned long *bitmap;



// For count all the "both dirtied" pages
static unsigned long *dirty_indices;
static unsigned long dirty_count; 

static merkle_tree_t *mtree; 




void *compute_thread_func(void *arg){
	int t = *(int *)arg; //The thread index of the thread
	
	while(1){
		pthread_mutex_lock(&locks[t]);
		pthread_cond_wait(&conds[t], &locks[t]);
		pthread_mutex_unlock(&locks[t]);
	}
	



}



void init(unsigned long len){

	int i; 

	ts = (pthread_t *) malloc (nthread * sizeof(pthread_t));
	locks = (pthread_mutex_t *) malloc(nthread * sizeof(pthread_mutex_t));
	conds = (pthread_cond_t *)malloc (nthread * sizeof(pthread_cond_t));

	for (i = 0; i < nthread; i++){
		pthread_mutex_init(&locks[i], NULL);
		pthread_cond_t(&conds[i], NULL);
		indices[i] = i;
		pthread_create(&ts[i], NULL, compute_thread_func, (void *)&indices[i]);
	}
	dirty_indices = (unsigned long *) malloc(len * sizeof(unsigned long));

}

static update_dirty_indices(unsigned long *bitmap, unsigned long len){
	unsigned long mask; 
	int i, offset; 
	for (i =0; i * 64 < len; ++i){
		mask = 0x8000000000000000;
		for (offset = 0; offset <64; offset++){
			if (mask & bitmap){
				dirty_indices[dirty_count]=i*64 + offset; 
				dirty_count++;
			}
			mask >>= 1; 
		}
	}
}
//Construct a complete binary tree as http://mathworld.wolfram.com/images/eps-gif/CompleteBinaryTree_1000.gif

merkle_tree_t build_merkle_tree (unsigned long *xor_bitmap, unsigned long len){
	dirty_count = 0;
	update_dirty_indices(xor_bitmap, len);

	mtree = (merkle_tree_t *) malloc (sizeof(merkle_tree_t));

		
	//get log base 2
	unsigned long c = dirty_count; 
	int tree_height = 1; 
	while ( c >>= 1) { ++tree_height }
	// the tree height 


	mtree-> tree_size = 1 << (tree_height) - 1 + 2 * (len - 1 << (tree_height-1));
	mtree-> tree = (hash_t *) malloc (mtree->tree_size * sizeof(hash_t));

	int i ;
	for (i = 0; i<nthread; i++){
		pthread_mutex_lock(&locks[i]);
		pthread_cond_broadcast(&conds[i]);
		pthread_mutex_unlock(&locks[i]);
	}
}


