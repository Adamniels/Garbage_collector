#include "heap.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

page_t *p_init(void *page_start, size_t page_index) {
  if (!page_start) {
    assert(!"invalid address");
  }

  page_t *page = (page_t *)page_start;

  // move page start to after the page struct
  page->page_start = (uint8_t *)page_start + sizeof(page_t);

  // set all the metadata
  page->next_empty_space = page->page_start;
  page->is_active = false;
  page->is_safe = true;
  page->remaining_size = PAGE_SIZE;
  page->index = page_index;

  return page;
}

heap_t *h_init(size_t bytes, bool unsafe_stack, float gc_threshold) {
  // Check if we have enough space for at least one page:
  // (bytes - (sizeof(heap_t) + sizeof(page_t))) needs to be >= PAGE_SIZE
  // If not, we assert (triggering a program abort).
  if (bytes < ((sizeof(heap_t) + sizeof(page_t)) + PAGE_SIZE)) {
    assert(!"Too small of a heap");
  }
  size_t bytes_to_allocate = bytes;
  size_t page_amount = bytes / PAGE_SIZE;
  // space for all structs
  bytes_to_allocate += page_amount * sizeof(page_t) + sizeof(heap_t);
  // space for allocation map???
  bytes_to_allocate += PAGE_SIZE * page_amount / MIN_OBJECT_SIZE;

  // TODO: om det här är arrayen så känns det som det borde vara * page_amount
  bytes_to_allocate += page_amount * sizeof(page_t *);

  void *heap_mem = NULL;

  int result = posix_memalign(&heap_mem, ALIGNMENT, bytes_to_allocate);
  if (result != 0) {
    fprintf(stderr, "posix_memalign misslyckades med kod: %d\n", result);
    assert(!"Allocation of heap failed, posix_error");
  }

  if (!heap_mem) {
    assert(!"Allocation of heap failed");
  }

  // The heap structure is placed at the start of heap_mem.
  heap_t *heap = (heap_t *)heap_mem;

  // The heap_start points just after the heap_t metadata.
  heap->heap_start = (uint8_t *)heap_mem + sizeof(heap_t);
  heap->page_amount = page_amount;

  // two uint64_t can represent a page
  size_t alloc_map_entries = page_amount * 2;

  heap->alloc_map = (uint64_t *)heap->heap_start;
  // initialise the alloc map to 0
  for (size_t i = 0; i < alloc_map_entries; i++) {
    heap->alloc_map[i] = 0;
  }
  // Move heap_start beyond alloc_map
  heap->heap_start =
      (uint8_t *)heap->heap_start + (alloc_map_entries * sizeof(uint64_t));

  heap->heap_size = bytes;
  heap->GC_threshold = gc_threshold;
  heap->safe = !unsafe_stack;

  // Positions the page_array after all the pages:
  heap->page_array =
      (page_t **)((uint8_t *)heap->heap_start +
                  (heap->page_amount * (PAGE_SIZE + sizeof(page_t))));

  // Initialize each page:
  for (size_t i = 0; i < heap->page_amount; i++) {
    heap->page_array[i] = p_init(
        (uint8_t *)heap->heap_start + i * (PAGE_SIZE + sizeof(page_t)), i);
  }

  return heap;
}

void h_delete(heap_t *heap) {
  if (!heap) {
    assert(!"invalid heap");
  }
  free(heap);
}

void h_delete_dbg(heap_t *heap, unsigned char dbg_value) {
  if (!heap) {
    assert(!"invalid heap");
  }

  if (heap->heap_start) {
    memset(heap, dbg_value, heap->heap_size);
  }

  free(heap);
}
