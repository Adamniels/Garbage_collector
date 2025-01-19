#include "../src/gc.h"
#include "../src/heap.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void create_heap_test(void) {
  heap_t *heap = h_init((size_t)5200, false, 0.4);
  CU_ASSERT_PTR_NOT_NULL_FATAL(heap);
  CU_ASSERT_EQUAL(heap->page_amount, 2);

  // Test to see if anything overlaps
  // TODO: actually write test for this
  // TODO: något är fel, det är förskjutet med 320 bytes
  printf("address heap: %lu\n", (size_t)heap);
  printf("address page start: %lu\n", (size_t)&heap->heap_start);
  printf("address heap size: %lu\n", (size_t)&heap->heap_size);
  printf("address page array: %lu\n", (size_t)&heap->page_array);
  printf("address page amount: %lu\n", (size_t)&heap->page_amount);
  printf("address safe: %lu\n", (size_t)&heap->safe);
  printf("address gc threshold: %lu\n", (size_t)&heap->GC_threshold);
  printf("address alloc map: %lu\n", (size_t)&heap->alloc_map);
  printf("address alloc map last: %lu\n", (size_t)&heap->alloc_map[3]);
  printf("address of heap start: %lu\n", (size_t)heap->heap_start);
  printf("first page start: %lu\n", (size_t)heap->page_array[0]->page_start);
  printf("second page start: %lu\n", (size_t)heap->page_array[1]->page_start);
  printf("size of page struct: %lu\n", sizeof(page_t));
  printf("addres of first in page array: %lu\n", (size_t)&heap->page_array[0]);
}

// TODO: vad vill jag testa:
//  -

int heap_tests() {
  CU_pSuite pSuite = CU_add_suite("allocation_tests", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL ==
       CU_add_test(pSuite, "create a heap, simple test", create_heap_test))) {

    CU_cleanup_registry();
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
