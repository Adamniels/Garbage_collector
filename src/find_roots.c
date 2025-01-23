#include "find_roots.h"
#include "lib/linked_list.h"
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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
  if ((uint8_t *)ptr < (uint8_t *)heap->heap_start ||
      (uint8_t *)ptr >= (uint8_t *)heap->heap_start + heap->heap_size) {
    return false;
  }
  return true;
}

// NOTE: stack seems to grow downwards
ioopm_list_t *find_gc_roots(heap_t *heap) {
  void **stack_top = get_stack_top();
  void **stack_bottom = get_stack_bottom();

  ioopm_list_t *roots = ioopm_linked_list_create(ioopm_ptr_cmp_func);
  if (roots == NULL) {
    assert(!"Could not linked list to hold roots");
  }

  for (void **current = stack_top; current <= stack_bottom;
       current = current + 1) {
    if (is_allocated_on_heap(heap, *current)) {
      ioopm_linked_list_append(roots, (elem_t){.ptr = current});
    }
  }

  return roots;
}
