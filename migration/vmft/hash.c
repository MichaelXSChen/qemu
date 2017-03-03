#include "migration/vmft/hash.h"
#include "sys/queue.h"

#define nthread 20

#define ALGO_TEST




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
	uint8_t full;
	uint64_t full_part_last_index; 
	uint64_t first_leaf_index; 
	uint64_t last_level_leaf_count;
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

#ifdef ALGO_TEST
static uint8_t *fake_page; 
#endif

static int finished_thread; 
static pthread_spinlock_t finished_lock;


static inline unsigned long index_to_node (unsigned long i){
	if ( i >= mtree->last_level_leaf_count ){
		return (i - mtree->last_level_leaf_count) + mtree->first_leaf_index; 
	}
	else{
		return (mtree->last_level_leaf_count + i + 1);
	}
}


static inline uint8_t* get_page_addr(unsigned long i){
	return fake_page;
}

static inline void compute_hash(unsigned long i){
	mtree->tree[index_to_node(i)] = hashofpage(get_page_addr(i), 4096);
}



void *compute_thread_func(void *arg){
	int t = *(int *)arg; //The thread index of the thread
	
	while(1){
		pthread_mutex_lock(&locks[t]);
		pthread_cond_wait(&conds[t], &locks[t]);
		pthread_mutex_unlock(&locks[t]);
	}

	unsigned long workload = dirty_count / nthread + 1
	unsigned long job_start = t * workload; 
	unsigned long job_end;
	if ( t == (nthread -1)) {
		job_end = dirty_count -1; 
	}
	else {
		job_end  = (t+1) * workload -1;  
	}
	unsigned long i; 
	for (i = job_start; i <= job_end; i++){
		compute_hash(i);
	}
	pthread_spin_lock(&finished_lock);
	finished_thread++;
	pthread_spin_unlock(&finished_lock);

}



void init(unsigned long len){

	int i; 

	ts = (pthread_t *) malloc (nthread * sizeof(pthread_t));
	locks = (pthread_mutex_t *) malloc(nthread * sizeof(pthread_mutex_t));
	conds = (pthread_cond_t *)malloc (nthread * sizeof(pthread_cond_t));

	pthread_spin_init (&finished_lock, 0);


	for (i = 0; i < nthread; i++){
		pthread_mutex_init(&locks[i], NULL);
		pthread_cond_t(&conds[i], NULL);
		indices[i] = i;
		pthread_create(&ts[i], NULL, compute_thread_func, (void *)&indices[i]);
	}
	dirty_indices = (unsigned long *) malloc(len * sizeof(unsigned long));

	#ifdef ALGO_TEST
	fake_page = (uint8_t *) malloc (4096);
	#endif
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
	if (len - 1 << (tree_height-1) == 0) {
		mtree -> full = 1; 
	}
	else {
		mtree -> full = 0;
	}
	merkle_tree -> first_leaf_index = (tree_size - 1) / 2 ;
	merkle_tree -> full_part_last_index = 1 << (tree_height) - 2; 
	merkle_tree -> last_level_leaf_count =  2 * (len - 1 << (tree_height-1));


	int i ;
	for (i = 0; i<nthread; i++){
		pthread_mutex_lock(&locks[i]);
		pthread_cond_broadcast(&conds[i]);
		pthread_mutex_unlock(&locks[i]);
	}
	while(finished_thread < nthread){
	}
	unsigned long j;
	for ( j= mtree->first_leaf_index-1 ; j >= 0; j--){
		mtree->tree[j]=hashofhash(mtree->tree[2*j +1], mtree->tree[2*j + 2]);
	}
}





unsigned long compare_merkle_tree(merkle_tree_t *tree_a, merkle_tree_t *tree_b){
	unsigned long count = 0;

	unsigned long *stk = (unsigned long*) malloc(tree_a->tree_size * sizeof(unsigned long));
	unsigned long top = -1;



	stk[top+1] = 0;
	top++; 
	while (top )
}


