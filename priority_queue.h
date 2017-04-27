#ifndef _priorityq_h
#define _priorityq_h


#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int priority;
    void *data;
} node_t;

typedef struct {
    node_t *nodes;
    int len;
    int size;
} heap_t;

void* getAt(heap_t *h, int pos);


void push (heap_t *h, int priority, void *data);

void *pop (heap_t *h);

#endif
