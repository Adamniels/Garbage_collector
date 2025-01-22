#pragma once
#include "lib/linked_list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 2048
#define MIN_OBJECT_SIZE 16
#define ALIGNMENT 0x1000
/**
 * A page represent a single memory page within the heap
 *
 * Each page tracks metadata:
 *  - ptr to next empty space
 *  - ptr to the page start
 *  - the remaining size
 *  - if the page is active or passive(used in compacting)
 *  - if it is safe or not(depends if we see the ptr from the stack as safe or
 * not)
 *  - the page index in the page array in the heap
 */
typedef struct page {
  void *next_empty_space;
  void *page_start;
  size_t remaining_size;
  bool is_active;
  bool is_safe;
  size_t index;
} page_t;

/**
 * The heap structure represent a memory region where the user can allocate
 * memory
 *
 * TODO: fortsätt med denna
 */
typedef struct heap {
  void *heap_start;
  size_t heap_size;
  page_t **page_array;
  size_t page_amount;
  bool safe;
  float GC_threshold;
  uint64_t *alloc_map;
} heap_t;

heap_t *h_init(size_t bytes, bool unsafe_stack, float gc_threshold);
void h_delete(heap_t *heap);
void h_delete_dbg(heap_t *heap, unsigned char dbg_value);
