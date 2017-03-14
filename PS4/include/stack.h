#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _stack{
    int size;
    void* data;
    struct _stack *next;
} *stack;

bool stack_is_empty(stack *s);
int stack_getsize(stack *s);
void stack_init(struct _stack **s);
void stack_push(struct _stack **s, void *data);
void* stack_pop(struct _stack **s);
void* stack_peek(struct _stack **s);
void stack_destroy(struct _stack **s);
