//Stack of strings data structure
#include <stdlib.h>
#include <assert.h>


struct stack {
	char* string;
	struct stack* next;
};

struct stack* init() {
	return (struct stack*)0 ;
}

void push(struct stack** stack, char* s) {
	struct stack* newTop = (struct stack*)malloc(sizeof(struct stack));
	newTop->string = s;
	newTop->next = *stack;
	*stack = newTop;
}

char* pop(struct stack** stack) {
	char* top = (*stack)->string;
	*stack = (*stack)->next;
	return top;
}

char* top(struct stack* stack) {
	return stack->string;
}

/*
void main(void) {
	struct stack* st = init();
	push(&st, &"hello");
	push(&st, &"world");
	assert(!strcmp(top(st), "world"));
	assert(!strcmp(top(st), "world"));
	pop(&st);
	push(&st, &"monde");
	assert(!strcmp(top(st), "monde"));
	pop(&st);
	assert(!strcmp(top(st), "hello"));
	pop(&st);
	assert(st = init());
}
*/
