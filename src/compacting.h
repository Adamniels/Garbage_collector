#pragma once

#include "allocation.h"
#include "find_roots.h"
#include "gc.h"
#include "heap.h"
#include "lib/linked_list.h"

// Takes a pointer to a heap object and interprets its header,
// returning an array containing all pointers contained within the object
// Example: Header is [...] 0000 1101 0000 b2 b1 b0 -> returns array containing
// 3 pointers the size of the array is written into num_pointers the size (in
// bytes) of the object (excluding header) is written into obj_size (Header: "0"
// = 4-byte data, "1" = 8-byte pointer) PRE: p is a pointer into the heap, or
// NULL
void ***interpret_header(void *p, size_t *num_pointers, size_t *obj_size);

size_t h_gc(heap_t *h);

size_t h_gc_dbg(heap_t *h, bool unsafe_stack);

uint64_t extract_adress(uint64_t header);

size_t count_allocated_bytes_on_heap(heap_t *h);

void traverse_and_forward(heap_t *h, ioopm_list_t *root_list,
                          ioopm_list_t *expected_list);

void traverse_and_move(heap_t *h, ioopm_list_t *root_list,
                       ioopm_list_t *expected_list);
