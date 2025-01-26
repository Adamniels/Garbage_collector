#include "find_roots.h"
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

  // there is always a header so we should point 8 bytes in
  if (heap_ptr_offset % 8 != 0) {
    return false;
  }
  uint64_t pages_before_ptr = heap_ptr_offset / PAGE_SIZE;
  uint64_t page_offset = sizeof(page_t) * pages_before_ptr;

  uint64_t alloc_map_index = heap_ptr_offset / ((2048 + sizeof(page_t)));
  heap_ptr_offset -= page_offset;
  // heap_ptr_offset -= alloc_map_index;
  uint64_t alloc_map_bit = (127 - heap_ptr_offset / 16) % 128;
  alloc_map_index += alloc_map_bit < 64;
  alloc_map_bit %= 64;
  printf("\n-----------\n"
         "Pointer: %p\n"
         "With offset: %llu\n"
         "Alloc map index: %llu\n"
         "Alloc map bit: %llu\n"
         "-------------\n",
         ptr, heap_ptr_offset, alloc_map_index, alloc_map_bit);
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

  printf("Created list at: %p\n", roots);
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
  printf("Roots before returning:\n");
  print_linked_list(roots);

  return res;
}
