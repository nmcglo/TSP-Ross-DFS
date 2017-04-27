//Pulled and modified from https://rosettacode.org/wiki/Priority_queue#C on April 26, 2017


#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"

void* getAt(heap_t *h, int pos)
{
     return h->nodes[pos+1].data;
}

void push (heap_t *h, int priority, void *data) {
    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (node_t *)realloc(h->nodes, h->size * sizeof (node_t));
    }
    int i = h->len + 1;
    int j = i / 2;
    while (i > 1 && h->nodes[j].priority > priority) {
        h->nodes[i] = h->nodes[j];
        i = j;
        j = j / 2;
    }
    h->nodes[i].priority = priority;
    h->nodes[i].data = data;
    h->len++;
}

void *pop (heap_t *h) {
    int i, j, k;
    if (!h->len) {
        return NULL;
    }
    int *data = h->nodes[1].data;
    h->nodes[1] = h->nodes[h->len];
    h->len--;
    i = 1;
    while (1) {
        k = i;
        j = 2 * i;
        if (j <= h->len && h->nodes[j].priority < h->nodes[k].priority) {
            k = j;
        }
        if (j + 1 <= h->len && h->nodes[j + 1].priority < h->nodes[k].priority) {
            k = j + 1;
        }
        if (k == i) {
            break;
        }
        h->nodes[i] = h->nodes[k];
        i = k;
    }
    h->nodes[i] = h->nodes[h->len + 1];
    return data;
}
//
// int main () {
//     heap_t *h = (heap_t *)calloc(1, sizeof (heap_t));
//
//     city_weight_pair pairs[5];
//
//     for(int i = 0; i < 5; i++)
//     {
//          pairs[i].cityID = i;
//          pairs[i].weight = 5-i;
//
//          push(h, pairs[i].weight, &pairs[i]);
//     }
//
//
//     int i;
//     for (i = 0; i < 5; i++) {
//          city_weight_pair* pair = pop(h);
//         printf("%i %i\n", pair->cityID,pair->weight);
//     }
//     return 0;
// }
