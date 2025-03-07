// this is a demo that i constructed after the void
// test_GC_same_case_as_test_traverse_and_forward(void) that is becuase in the
// test i find to many roots and i want to see it that has with cunit or not
#include "../src/gc.h"
#include "../src/heap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ptr_ptr_int {
  void *ptr1;
  void *ptr2;
  int int1;
};

int demo_from_test(void) {
  printf("Demo to see if it is just the tests that find to many roots\n");
  heap_t *heap = h_init(10400, false, 0.5);
  printf("Created heap at: %p\n", heap);

  struct ptr_ptr_int *obj1 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj2 = h_alloc_struct(heap, "**i");
  struct ptr_ptr_int *obj3 = h_alloc_struct(heap, "**i");
  obj1->ptr1 = obj3;

  obj3 = NULL;

  size_t reclaimed = h_gc_dbg(heap, false);
  printf("Reclaimed memory: %lu\n", reclaimed);
  assert(obj3 == NULL);

  obj1 = NULL;

  reclaimed = h_gc_dbg(heap, false);
  printf("Reclaimed memory: %lu, each object is of size: %lu\n", reclaimed,
         sizeof(struct ptr_ptr_int));
  assert(obj1 == NULL);

  h_delete(heap);
  return EXIT_SUCCESS;
}

int main(void) { return demo_from_test(); }
