#include "../src/debug.h"
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
  DEBUG_PRINT("address heap: %lu\n", (size_t)heap);
  DEBUG_PRINT("address heap start: %lu\n", (size_t)&heap->heap_start);
  DEBUG_PRINT("address heap size: %lu\n", (size_t)&heap->heap_size);
  DEBUG_PRINT("address page array: %lu\n", (size_t)&heap->page_array);
  DEBUG_PRINT("address page amount: %lu\n", (size_t)&heap->page_amount);
  DEBUG_PRINT("address safe: %lu\n", (size_t)&heap->safe);
  DEBUG_PRINT("address gc threshold: %lu\n", (size_t)&heap->GC_threshold);
  DEBUG_PRINT("address alloc map: %lu\n", (size_t)&heap->alloc_map);
  DEBUG_PRINT("address alloc map last: %lu\n", (size_t)&heap->alloc_map[3]);
  DEBUG_PRINT("address of heap start: %lu\n", (size_t)heap->heap_start);
  DEBUG_PRINT("first page start: %lu\n",
              (size_t)heap->page_array[0]->page_start);
  DEBUG_PRINT("second page start: %lu\n",
              (size_t)heap->page_array[1]->page_start);
  DEBUG_PRINT("size of page struct: %lu\n", sizeof(page_t));
  DEBUG_PRINT("addres of first in page array: %lu\n",
              (size_t)&heap->page_array[0]);

  size_t heap_start = (size_t)heap->heap_start;
  size_t address_end_of_alloc_map =
      (size_t)&heap->alloc_map[heap->page_amount * 2 - 1] + sizeof(uint64_t);

  // make sure allocation map don't overlap with first page, alloc end should be
  // same as heap start
  CU_ASSERT_EQUAL(heap_start, address_end_of_alloc_map);
  // TODO: fortsätt testa detta

  h_delete(heap);
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
