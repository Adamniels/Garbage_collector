#include <stdbool.h>
#include <stddef.h>

#ifndef __gc__
#define __gc__

typedef struct heap heap_t;

heap_t *h_init(size_t bytes, bool unsafe_stack, float gc_threshold);
void h_delete(heap_t *heap);
void h_delete_dbg(heap_t *heap, unsigned char dbg_value);

void *h_alloc_struct(heap_t *h, char *layout);
void *h_alloc_raw(heap_t *h, size_t bytes);

size_t h_avail(heap_t *h);
size_t h_used(heap_t *h);
size_t h_gc(heap_t *h);
size_t h_gc_dbg(heap_t *h, bool unsafe_stack, void *top);

#endif
