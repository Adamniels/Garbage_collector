#include "find_roots.h"
#include "debug.h"
#include "lib/linked_list.h"
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define Dump_registers()                                                       \
  jmp_buf env;                                                                 \
  if (setjmp(env))                                                             \
    abort();

extern char **environ;

void *get_stack_top() { return __builtin_frame_address(0); }

void *get_stack_bottom() { return (void *)environ; }

// helper function check if allocated on our heap
bool is_allocated_on_heap(heap_t *heap, void *ptr) {
  if (!heap) {
    assert(!"invalid heap");
  }
  if ((uint8_t *)ptr <= (uint8_t *)heap->heap_start ||
      (uint8_t *)ptr >= (uint8_t *)heap->heap_start + heap->heap_size) {
    return false;
  }
  uint64_t *allocation_map = heap->alloc_map;
  uint64_t heap_ptr_offset =
      (uint8_t *)ptr - ((uint8_t *)heap->heap_start + sizeof(page_t));

  // för debuggin //
  uint64_t second_page_start_address =
      (uint64_t)heap->page_array[1]->page_start -
      ((uint64_t)heap->heap_start + sizeof(page_t));
  DEBUG_PRINT("second_page_start_address: %llu\n", second_page_start_address);
  //////////////////

  // there is always a header so we should point 8 bytes in
  if (heap_ptr_offset % 8 != 0) {
    return false;
  }
  uint64_t pages_before_ptr = heap_ptr_offset / PAGE_SIZE;

  DEBUG_PRINT("\n--------------\n");
  DEBUG_PRINT("heap_ptr_offset before: %llu\n", heap_ptr_offset);
  DEBUG_PRINT("pages_before_ptr: %llu\n", pages_before_ptr);

  uint64_t page_offset = sizeof(page_t) * pages_before_ptr;

  uint64_t alloc_map_index = heap_ptr_offset / ((2048 + sizeof(page_t)));
  alloc_map_index *= 2;
  heap_ptr_offset -= page_offset;
  // heap_ptr_offset -= alloc_map_index;
  uint64_t alloc_map_bit = 127 - ((heap_ptr_offset / 16) % 128);
  alloc_map_index += alloc_map_bit < 64;
  alloc_map_bit %= 64;

  DEBUG_PRINT("Pointer: %p\n", ptr);
  DEBUG_PRINT("With offset: %llu\n", heap_ptr_offset);
  DEBUG_PRINT("Alloc map index: %llu\n", alloc_map_index);
  DEBUG_PRINT("Alloc map bit: %llu\n", alloc_map_bit);
  DEBUG_PRINT("-------------\n");

  if ((allocation_map[alloc_map_index] & (1ull << alloc_map_bit)) != 0) {
    return true;
  }

  return false;
}

// NOTE: stack seems to grow downwards
result *find_gc_roots(heap_t *heap) {
  void **stack_top = get_stack_top();
  void **stack_bottom = get_stack_bottom();

  result *res = calloc(1, sizeof(result));
  ioopm_list_t *roots = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_list_t *expected_roots1 = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  ioopm_list_t *expected_roots2 = ioopm_linked_list_create(ioopm_ptr_cmp_func);

  res->roots = roots;
  res->expected_roots1 = expected_roots1;
  res->expected_roots2 = expected_roots2;

  DEBUG_PRINT("created list at: %p\n", roots);
  if (roots == NULL) {
    assert(!"Could not linked list to hold roots");
  }

  for (void **current = stack_top; current <= stack_bottom;
       current = current + 1) {

    if (is_allocated_on_heap(heap, *current)) {
      ioopm_linked_list_append(expected_roots1, (elem_t){.ptr = *current});
      ioopm_linked_list_append(expected_roots2, (elem_t){.ptr = *current});

      ioopm_linked_list_append(roots, (elem_t){.ptr = current});
    }
  }
  asm volatile("" ::: "memory");
  DEBUG_PRINT("Roots before returning:\n");
  print_linked_list(roots);

  return res;
}
