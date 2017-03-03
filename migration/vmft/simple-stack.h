struct stack {
	unsigned long *stk; 
	unsigned long top;
	unsigned long size; 
}


typedef struct stack stack; 

void init_stack(stack *s, unsigned long len){
	s->stk = (unsigned long *) malloc(len * sizeof(unsigned long));
	s->size = len; 
	s-> top = -1;
}

void push()
