#include "stack.h"

bool stack_is_empty(stack *s) { return *s == NULL; }

void stack_init(struct _stack **s) {
    *s = NULL;
}

int stack_getsize(stack *s) {
    if (stack_is_empty(s)) {
        return 0;
    } else {
        return (*s)->size;
    }
}

void stack_push(struct _stack **s, void *data) {
    struct _stack *new_node = malloc(sizeof(struct _stack));
    if(!new_node) {
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    if(stack_is_empty(s)) {
        new_node->size = 0;
    } else {
        new_node->size = (*s)->size;
    }
    new_node->next = *s;
    *s = new_node;
    (*s)->size++;
}

void* stack_pop(struct _stack **s) {
    if (stack_is_empty(s)) {
        return NULL;
    }
    void *ret_val;
    struct _stack *trash = *s;
    ret_val = (*s)->data;
    *s = (*s)->next;
    free(trash);
    trash = NULL;
    return ret_val;
}

void* stack_peek(struct _stack **s) {
    return (*s)->data;
}

void stack_destroy(struct _stack **s) {
    while (!stack_is_empty(s)) {
        //printf("motherfucker is %i\n",(*s)->size);
        stack_pop(s);
    }
}
